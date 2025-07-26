#include "test.h"
#include "utils/fluid_sys.h"


// test threading, conditions, thread private data and atomics

#define THREAD_COUNT        10
#define INCREMENT_COUNT     10000

struct increment_data {
    fluid_cond_mutex_t *mutex;
    fluid_cond_t *condition;
    fluid_private_t local;
    int ready;
    int done;
    int regular;
    int atomic;
};

static fluid_thread_return_t increment(void *_data)
{
    int i;
    char *original = NULL;
    char *pointer = NULL;
    struct increment_data *data = (struct increment_data *)_data;

    TEST_ASSERT(fluid_private_get(data->local) == NULL);

    fluid_cond_mutex_lock(data->mutex);
    data->ready++;

    fluid_cond_broadcast(data->condition);
    while (data->ready != THREAD_COUNT)
    {
        fluid_cond_wait(data->condition, data->mutex);
    }

    fluid_cond_mutex_unlock(data->mutex);

    for (i = 0; i < INCREMENT_COUNT; i++)
    {
        data->regular++;
        fluid_atomic_int_inc(&data->atomic);

        pointer = fluid_private_get(data->local);
        fluid_private_set(data->local, pointer + 1);
    }

    fluid_cond_mutex_lock(data->mutex);
    data->done++;

    fluid_cond_broadcast(data->condition);
    while (data->done != THREAD_COUNT)
    {
        fluid_cond_wait(data->condition, data->mutex);
    }

    fluid_cond_mutex_unlock(data->mutex);

    pointer = fluid_private_get(data->local);
    TEST_ASSERT(pointer - original == INCREMENT_COUNT);
    return FLUID_THREAD_RETURN_VALUE;
}

void test_atomic_inc(void)
{
    int i;
    fluid_thread_t *threads[THREAD_COUNT];
    struct increment_data data;

    data.mutex = new_fluid_cond_mutex();
    TEST_ASSERT(data.mutex != NULL);

    data.condition = new_fluid_cond();
    TEST_ASSERT(data.condition != NULL);

    fluid_private_init(data.local);

    data.ready = 0;
    data.done = 0;
    data.regular = 0;
    data.atomic = 0;

    for (i = 0; i < THREAD_COUNT; i++)
    {
        threads[i] = new_fluid_thread("increment", increment, &data, 0, FALSE);
    }

    for (i = 0; i < THREAD_COUNT; i++)
    {
        fluid_thread_join(threads[i]);
        delete_fluid_thread(threads[i]);
    }

    FLUID_LOG(FLUID_INFO, "incremented %d regular vs. %d atomic", data.regular, data.atomic);
    TEST_ASSERT(data.atomic == THREAD_COUNT * INCREMENT_COUNT);

    fluid_private_free(data.local);
    delete_fluid_cond(data.condition);
    delete_fluid_cond_mutex(data.mutex);
}

int main(void)
{
    test_atomic_inc();
    return EXIT_SUCCESS;
}
