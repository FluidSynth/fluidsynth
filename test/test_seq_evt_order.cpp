// A C++11 test verifying that event_compare() is a strict weak ordering.
//
// Properties for a comparator comp(a,b):
//   1. Irreflexivity:        !comp(a,a)
//   2. Asymmetry:            comp(a,b) => !comp(b,a)
//   3. Transitivity:         comp(a,b) && comp(b,c) => comp(a,c)
//   4. Equivalence transitivity:
//        eq(a,b) := !comp(a,b) && !comp(b,a)
//        eq(a,b) && eq(b,c) => eq(a,c)
//
// NOTE: event_compare() (wrapped as event_compare_for_test) returns
//       !leftIsBeforeRight (i.e. inverted). We still test the strict weak
//       ordering properties against that returned predicate because that
//       predicate is exactly what std::make_heap / std::push_heap use.
//
#include <vector>
#include <memory>
#include <cstdlib>

#include "test.h"
#include "fluidsynth.h"
#include "fluid_seq_queue.h"

// Helper RAII deleter for fluid_event_t*
struct FluidEventDeleter
{
    void operator()(fluid_event_t *e) const
    {
        delete_fluid_event(e);
    }
};

using EventPtr = std::unique_ptr<fluid_event_t, FluidEventDeleter>;

static EventPtr make_event(unsigned int time)
{
    EventPtr p(new_fluid_event());
    fluid_event_set_time(p.get(), time);
    return p;
}

static void test_strict_weak_ordering_same_timestamp()
{
    // Create a representative set of events sharing the same timestamp
    // to exercise the type-based precedence logic.
    std::vector<EventPtr> ev;
    ev.push_back(make_event(100));
    fluid_event_system_reset(ev.back().get());
    ev.push_back(make_event(100));
    fluid_event_noteoff(ev.back().get(), 0, 62);
    ev.push_back(make_event(100));
    fluid_event_unregistering(ev.back().get());
    ev.push_back(make_event(100));
    fluid_event_bank_select(ev.back().get(), 0, 1);
    ev.push_back(make_event(100));
    fluid_event_program_change(ev.back().get(), 0, 2);
    ev.push_back(make_event(100));
    fluid_event_noteon(ev.back().get(), 0, 60, 90);
    ev.push_back(make_event(100));
    fluid_event_note(ev.back().get(), 0, 61, 80, 10);
    ev.push_back(make_event(100));
    fluid_event_pan(ev.back().get(), 0, 40);
    ev.push_back(make_event(100));
    fluid_event_modulation(ev.back().get(), 0, 50);
    ev.push_back(make_event(100));
    fluid_event_pitch_bend(ev.back().get(), 0, 200);

    auto COMP = &event_compare_for_test;
    auto EQ = [&](const fluid_event_t *a, const fluid_event_t *b) {
        return !COMP(a, b) && !COMP(b, a);
    };

    const size_t N = ev.size();

    // 1. Irreflexivity
    for (size_t i = 0; i < N; ++i)
    {
        TEST_ASSERT(!COMP(ev[i].get(), ev[i].get()));
    }

    // 2. Asymmetry
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            if (i == j)
            {
                // we're not interested in identical event types
                continue;
            }
            if (COMP(ev[i].get(), ev[j].get()))
            {
                // if event j sorts before i, i should not sort before j
                TEST_ASSERT(!COMP(ev[j].get(), ev[i].get()));
            }
        }
    }

    // 3. Transitivity
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            if (i == j)
            {
                // we're not interested in identical event types
                continue;
            }
            if (!COMP(ev[i].get(), ev[j].get()))
            {
                // ignore if event i sorts before j
                continue;
            }
            // at this point event j sorts before i
            for (size_t k = 0; k < N; ++k)
            {
                if (k == i || k == j)
                {
                    // we're not interested in identical event types
                    continue;
                }
                if (COMP(ev[j].get(), ev[k].get()))
                {
                    // if event k sorts before event j, and j sorts before i, k must sort before i
                    TEST_ASSERT(COMP(ev[i].get(), ev[k].get()));
                }
            }
        }
    }

    // 4. Equivalence transitivity
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            if (i == j)
            {
                // we're not interested in identical event types
                continue;
            }
            if (!EQ(ev[i].get(), ev[j].get()))
            {
                // we're only interested in different event types that evaluate to "equal"
                continue;
            }
            for (size_t k = 0; k < N; ++k)
            {
                if (k == i || k == j)
                {
                    // we're not interested in identical event types
                    continue;
                }
                if (EQ(ev[j].get(), ev[k].get()))
                {
                    // events k and j that evaluate "equal" should also evaluate "equal" with i
                    TEST_ASSERT(EQ(ev[i].get(), ev[k].get()));
                }
            }
        }
    }
}

static void test_time_precedence()
{
    // Earlier timestamp must "win" regardless of type precedence.
    EventPtr early = make_event(10);
    EventPtr late = make_event(20);
    fluid_event_program_change(early.get(), 0, 1);
    fluid_event_system_reset(late.get()); // System reset has highest type precedence at same tick.

    // event_compare_for_test returns !leftIsBeforeRight; since early < late by time,
    // leftIsBeforeRight is true -> comparator returns false.
    TEST_ASSERT(!event_compare_for_test(early.get(), late.get()));
    TEST_ASSERT(event_compare_for_test(late.get(), early.get()));
}

static void test_original_precedence_cases()
{
    EventPtr evt1 = make_event(1);
    EventPtr evt2 = make_event(1);

    TEST_ASSERT(!event_compare_for_test(evt1.get(), evt1.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt2.get()));

    fluid_event_bank_select(evt1.get(), 0, 0);
    fluid_event_program_change(evt2.get(), 0, 0);

    TEST_ASSERT(!event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(event_compare_for_test(evt2.get(), evt1.get()));

    fluid_event_note(evt1.get(), 0, 0, 0, 1);

    TEST_ASSERT(event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));

    fluid_event_noteon(evt1.get(), 0, 0, 60);
    fluid_event_noteoff(evt2.get(), 0, 0);

    TEST_ASSERT(event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));

    // velocity 0 noteon treated as noteoff
    fluid_event_noteon(evt1.get(), 0, 0, 60);
    fluid_event_noteon(evt2.get(), 0, 0, 0);

    TEST_ASSERT(event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));

    // two noteoffs -> equivalent
    fluid_event_noteon(evt1.get(), 0, 0, 0);
    fluid_event_noteoff(evt2.get(), 0, 0);

    TEST_ASSERT(!event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));

    fluid_event_unregistering(evt1.get());
    fluid_event_system_reset(evt2.get());

    TEST_ASSERT(event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));

    fluid_event_unregistering(evt1.get());
    fluid_event_pan(evt2.get(), 0, 0);

    TEST_ASSERT(!event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(event_compare_for_test(evt2.get(), evt1.get()));

    fluid_event_modulation(evt1.get(), 0, 0);
    fluid_event_pan(evt2.get(), 0, 0);

    TEST_ASSERT(!event_compare_for_test(evt1.get(), evt2.get()));
    TEST_ASSERT(!event_compare_for_test(evt2.get(), evt1.get()));
}

int main()
{
    test_original_precedence_cases();
    test_strict_weak_ordering_same_timestamp();
    test_time_precedence();
    return EXIT_SUCCESS;
}
