/*----------------------------------------------------------------------------
These tests check fluid_mod_check_linked_mod() function.
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_mod.h"
#include "utils/fluid_sys.h"

//----------------------------------------------------------------------------
static fluid_mod_t * fluid_build_list(fluid_mod_t mod_table[], int count_mod);
static int fluid_list_test_identity(fluid_mod_t *list_mod1, fluid_mod_t *list_mod2);

// tests the linked "nature" of modulators, i.e. fluid_mod_check_linked_mod()
int main(void)
{
    int linked_count;
	fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();
    fluid_mod_t *mod3 = new_fluid_mod();
    fluid_mod_t *mod4 = new_fluid_mod();
    fluid_mod_t *mod5 = new_fluid_mod();
    
    fluid_mod_t *list_of_mods = NULL;
    
    // set up a list of simple modulators
	printf("Test 1: set up a list of simple modulators\n");
    {
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_VOLENVATTACK);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod1, 11025);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod2, 34, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, 0);
        fluid_mod_set_amount (mod2, 960);
        fluid_mod_set_dest   (mod2, GEN_ATTENUATION);
        
        // Return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_simple_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);
        
        // order of modulators remains the same
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // no modulators are invalidated, amounts are not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 11025);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 960);
    }

    // Test 1.1: Same test as 1 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod doesn't contain any linked modulator.
    printf("\nTest 1.1: same test as 1 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - NULL must be returned in linked_mod
        //  - 0 must be returned in linked_count
        linked_mod = mod0; // initialize linked_mod to non NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_simple_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 0);
        TEST_ASSERT(linked_mod == NULL);
    }

    // set up a valid list of complex modulators
	printf("\nTest 2: set up a valid list of complex modulators\n");
	printf(  " List:m0,m1,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m1-->m0-->gen\n");
	printf(  "  CC-->m2-->m0\n");
    {
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        // Return must be 3
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 3);

        // order not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }
    
    // Test 2.1: Same test as 2 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 3 linked modulator.
    printf("\nTest 2.1: same test as 2 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }

    // same list that Test 2, but changed order
	printf("\nTest 3: same list that test 2, but changed order\n");
	printf(  " List:m0,m2,m1\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m2-->m0-->gen\n");
	printf(  "  CC-->m1-->m0\n");
    {
        mod1->next = NULL;
        mod2->next = mod1;
        mod0->next = mod2;
        list_of_mods = mod0;
    
        // Return must be 3
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 3);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod1);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }

    // Test 3.1: Same test as 3 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 3 linked modulator.
    printf("\nTest 3.1: same test as 3 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }
    
    // Test 3.2: Same as 3 but again change order.
    printf("\nTest 3.2: Same as 3 but again change order\n");
    printf(  " List:m0,m1,m2\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m1) connected to gen GEN_FILTERFC:\n");
    printf(  "  CC20-->m0-->m1-->GEN_FILTERFC\n");
    printf(  "  CC21-->m2-->m1\n");
    {
        list_of_mods = mod0;
        mod0->next = mod1;
        mod1->next = mod2;
        mod2->next = NULL;

        // CC20-->m0->m1
        fluid_mod_set_source1(mod0, 20, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 1);

        // link->m1->gen
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);

        // CC21-->m2->m1
        fluid_mod_set_source1(mod2, 21, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 1);

        // We don't want return linked modulator (i.e linked_mod is set to NULL).
        // Return count must be 3
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 3);

        // order not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);

        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }

    // Test 3.2.1: Same test as 3.2 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 3 linked modulator.
    printf("\nTest 3.2.1: Same test as 3.2 but with linked_mod not NULL\n");
    printf(  " List:m0,m1,m2\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m1) connected to gen GEN_FILTERFC:\n");
    printf(  "  CC20-->m0-->m1-->GEN_FILTERFC\n");
    printf(  "  CC21-->m2-->m1\n");
    printf(  " Paths in linked_mod from any CC to ending modulator (m0) connected to gen GEN_FILTERFC:\n");
    printf(  "  GEN_FILTERFC<--m0<--m1<--CC20\n");
    printf(  "                 m0<--m2<--CC21\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);

        /*
         Check ordering of complex modulators in linked_mod:
         The first member of any complex modulator is considered to have a
         relative index 0 regardless its index in list_mod. This first member
         is connected to a generator (it is called the "ending modulator").
         Members following the first have index relative to the first member.
         They are connected to others modulators always backward in linked_mod.
         Their destination field are index relative to the first member.
        */

        //check ordering of complex modulator in linked_mod: m0,m1,m2
        {
            fluid_mod_t  *m0, *m1, *m2; // members of linked_mod
            // Check m0 in linked_mod. m0 must be identic to mod1
            m0 = linked_mod;
            // m0 must be identic to mod1
            TEST_ASSERT(fluid_mod_test_identity(m0, mod1));
            // m0 amount must be identic to mod1 amount
            TEST_ASSERT(fluid_mod_get_amount(m0) == fluid_mod_get_amount(mod1));

            // Check m1 in linked_mod. m1 must be identic to mod0
            m1 = m0->next;
            // m1 destination must be index of m0 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m1) == (FLUID_MOD_LINK_DEST|0));
            // m1 amount must be identic to mod0 amount
            TEST_ASSERT(fluid_mod_get_amount(m1) == fluid_mod_get_amount(mod0));
            // m1 src1 must be identic to mod0 src1
            TEST_ASSERT(fluid_mod_get_source1(m1)== fluid_mod_get_source1(mod0));
            // m1 flags1 must be identic to mod0 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m1)== fluid_mod_get_flags1(mod0));
            // m1 src2 must be identic to mod0 src2
            TEST_ASSERT(fluid_mod_get_source2(m1)== fluid_mod_get_source2(mod0));
            // m1 flags2 must be identic to mod0 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m1)== fluid_mod_get_flags2(mod0));

            // Check m2 in linked_mod. m2 must be identic to mod2
            m2 = m1->next;
            // m2 destination must be index of m0 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m2) == (FLUID_MOD_LINK_DEST|0));
            // m2 amount must be identic to mod2 amount
            TEST_ASSERT(fluid_mod_get_amount(m2) == fluid_mod_get_amount(mod2));
            // m2 src1 must be identic to mod2 src1
            TEST_ASSERT(fluid_mod_get_source1(m2)== fluid_mod_get_source1(mod2));
            // m2 flags1 must be identic to mod2 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m2)== fluid_mod_get_flags1(mod2));
            // m2 src2 must be identic to mod2 src2
            TEST_ASSERT(fluid_mod_get_source2(m2)== fluid_mod_get_source2(mod2));
            // m2 flags2 must be identic to mod2 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m2)== fluid_mod_get_flags2(mod2));
        }
        delete_fluid_mod_list(linked_mod);
    }

    // Test 3.3: Same list as 3.2 but we add another complex modulator in list_of_mods.
    printf("\nTest 3.3: Same list as 3.2 but we add another complex modulator\n");
    printf(  " List:m0,m1,m2, m3,m4,m5\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m1) connected to gen GEN_FILTERFC:\n");
    printf(  "  CC20-->m0-->m1-->GEN_FILTERFC\n");
    printf(  "  CC21-->m2-->m1\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m5) connected to gen GEN_MODLFOTOVOL:\n");
    printf(  "  CC22-->m3-->m5-->GEN_MODLFOTOVOL\n");
    printf(  "  CC23-->m4-->m5\n");
    {
        // add new complex modulator in list_of_mods
        // CC22-->m3->m5
        fluid_mod_set_source1(mod3, 22, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 400);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 5);

        // CC23-->m4-->m5
        fluid_mod_set_source1(mod4, 23, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod4, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod4, 500);
        fluid_mod_set_dest   (mod4, FLUID_MOD_LINK_DEST | 5);

        // link-->m5->gen
        fluid_mod_set_source1(mod5, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod5, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod5, 600);
        fluid_mod_set_dest   (mod5, GEN_MODLFOTOVOL);

        mod2->next = mod3;
        mod3->next = mod4;
        mod4->next = mod5;

        // We don't want return linked modulator (i.e linked_mod is set to NULL).
        // Return count must be: 6
        linked_count = fluid_mod_check_linked_mod("test_zone_with_2_complex_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 6);

        // order not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == mod4);
        TEST_ASSERT(list_of_mods->next->next->next->next->next == mod5);
        TEST_ASSERT(list_of_mods->next->next->next->next->next->next == NULL);

        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 400);
        TEST_ASSERT(fluid_mod_get_amount(mod4) == 500);
        TEST_ASSERT(fluid_mod_get_amount(mod5) == 600);
    }

    // Test 3.3.1: Same test as 3.3 but with linked_mod not NULL. The function expects to return any
    // 2 complex modulators in linked_mod pointer.
    // Actually linked_mod should contain 6 linked modulators.
    printf("\nTest 3.3.1: Same test as 3.3 but with linked_mod not NULL\n");
    printf(  " List:m0,m1,m2, m3,m4,m5\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m1) connected to gen GEN_FILTERFC:\n");
    printf(  "  CC20-->m0-->m1-->GEN_FILTERFC\n");
    printf(  "  CC21-->m2-->m1\n");
    printf(  " Paths in list_of_mods from any CC to ending modulator (m5) connected to gen GEN_MODLFOTOVOL:\n");
    printf(  "  CC22-->m3-->m5-->GEN_MODLFOTOVOL\n");
    printf(  "  CC23-->m4-->m5\n");
    printf(  " Paths in linked_mod from any CC to ending modulator (m0) connected to gen GEN_FILTERFC:\n");
    printf(  "  GEN_FILTERFC<-----m0<--m1<--CC20\n");
    printf(  "                    m0<--m2<--CC21\n");
    printf(  " Paths in linked_mod from any CC to ending modulator (m3) connected to gen GEN_MODLFOTOVOL:\n");
    printf(  "  GEN_MODLFOTOVOL<--m3<--m4<--CC22\n");
    printf(  "                    m3<--m5<--CC23\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 6 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 6);
        TEST_ASSERT(linked_mod != NULL);

        /*
         Check ordering of complex modulators in linked_mod:
         The first member of any complex modulator is considered to have a
         relative index 0 regardless its index in list_mod. This first member
         is connected to a generator (it is called the "ending modulator").
         Members following the first have index relative to the first member.
         They are connected to others modulators always backward in linked_mod.
         Their destination field are index relative to the first member.
        */
        {
            fluid_mod_t  *m0, *m1, *m2; // members of 1st complex modulator
            fluid_mod_t  *m3, *m4, *m5; // members of 2nd complex modulator
            //-----------------------------------------------------------------
            // Check ordering of 1st complex modulator in linked_mod: m0,m1,m2.
            // The result must be the same as for Test 3.2.1

            // Check ending modulator (m0) in linked_mod. m0 must be identic to mod1
            m0 = linked_mod;
            // m0 must be identic to mod1
            TEST_ASSERT(fluid_mod_test_identity(m0, mod1));
            // m0 amount must be identic to mod1 amount
            TEST_ASSERT(fluid_mod_get_amount(m0) == fluid_mod_get_amount(mod1));

            // Check m1 in linked_mod. m1 must be identic to mod0
            m1 = m0->next;
            // m1 destination must be index of m0 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m1) == (FLUID_MOD_LINK_DEST|0));
            // m1 amount must be identic to mod0 amount
            TEST_ASSERT(fluid_mod_get_amount(m1) == fluid_mod_get_amount(mod0));
            // m1 src1 must be identic to mod0 src1
            TEST_ASSERT(fluid_mod_get_source1(m1)== fluid_mod_get_source1(mod0));
            // m1 flags1 must be identic to mod0 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m1)== fluid_mod_get_flags1(mod0));
            // m1 src2 must be identic to mod0 src2
            TEST_ASSERT(fluid_mod_get_source2(m1)== fluid_mod_get_source2(mod0));
            // m1 flags2 must be identic to mod0 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m1)== fluid_mod_get_flags2(mod0));

            // Check m2 in linked_mod. m2 must be identic to mod2
            m2 = m1->next;
            // m2 destination must be index of m0 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m2) == (FLUID_MOD_LINK_DEST|0));
            // m2 amount must be identic to mod2 amount
            TEST_ASSERT(fluid_mod_get_amount(m2) == fluid_mod_get_amount(mod2));
            // m2 src1 must be identic to mod2 src1
            TEST_ASSERT(fluid_mod_get_source1(m2)== fluid_mod_get_source1(mod2));
            // m2 flags1 must be identic to mod2 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m2)== fluid_mod_get_flags1(mod2));
            // m2 src2 must be identic to mod2 src2
            TEST_ASSERT(fluid_mod_get_source2(m2)== fluid_mod_get_source2(mod2));
            // m2 flags2 must be identic to mod2 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m2)== fluid_mod_get_flags2(mod2));

            //-----------------------------------------------------------------
            // Check ordering of 2nd complex modulator in linked_mod: m3,m4,m5.

            // Check ending modulator (m3) in linked_mod. m3 must be identic to mod5
            m3 = m2->next;
            // m3 must be identic to mod5
            TEST_ASSERT(fluid_mod_test_identity(m3, mod5));
            // m3 amount must be identic to mod5 amount
            TEST_ASSERT(fluid_mod_get_amount(m3) == fluid_mod_get_amount(mod5));

            // Check m4 in linked_mod. m4 must be identic to mod3
            m4 = m3->next;
            // m4 destination must be relative index of m3 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m4) == (FLUID_MOD_LINK_DEST|0));
            // m4 amount must be identic to mod3 amount
            TEST_ASSERT(fluid_mod_get_amount(m4) == fluid_mod_get_amount(mod3));
            // m4 src1 must be identic to mod3 src1
            TEST_ASSERT(fluid_mod_get_source1(m4)== fluid_mod_get_source1(mod3));
            // m4 flags1 must be identic to mod3 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m4)== fluid_mod_get_flags1(mod3));
            // m4 src2 must be identic to mod3 src2
            TEST_ASSERT(fluid_mod_get_source2(m4)== fluid_mod_get_source2(mod3));
            // m4 flags2 must be identic to mod3 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m4)== fluid_mod_get_flags2(mod3));

            // Check m5 in linked_mod. m4 must be identic to mod4
            m5 = m4->next;
            // m5 destination must be relative index of m3 (i.e 0)
            TEST_ASSERT(fluid_mod_get_dest(m5) == (FLUID_MOD_LINK_DEST|0));
            // m5 amount must be identic to mod4 amount
            TEST_ASSERT(fluid_mod_get_amount(m5) == fluid_mod_get_amount(mod4));
            // m5 src1 must be identic to mod4 src1
            TEST_ASSERT(fluid_mod_get_source1(m5)== fluid_mod_get_source1(mod4));
            // m5 flags1 must be identic to mod4 flags1
            TEST_ASSERT(fluid_mod_get_flags1(m5)== fluid_mod_get_flags1(mod4));
            // m5 src2 must be identic to mod4 src2
            TEST_ASSERT(fluid_mod_get_source2(m5)== fluid_mod_get_source2(mod4));
            // m5 flags2 must be identic to mod4 flags2
            TEST_ASSERT(fluid_mod_get_flags2(m5)== fluid_mod_get_flags2(mod4));
        }
        delete_fluid_mod_list(linked_mod);
    }

    // same list that test 3, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC
    printf("\nTest 4: same list that test 3, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m1-->m0-->gen\n");
	printf(  "  CC------->m2-->m0\n");
    {
        // re-initialize list of mod as for test 2.
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);

        fluid_mod_set_source1(mod1, 20, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1
        
        mod3->next = NULL;
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        // Return must be 3
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 3);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0); // invalidated because mod1 without FLUID_MOD_LINK_SRC
    }

    // Test 4.1: Same test as 4 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 3 linked modulator.
    printf("\nTest 4.1: Same test as 4 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }

    // same list than test 4, with additional mod3 and valid mod1 this time
    printf("\nTest 5: same list than test 4, with additional mod3 and valid mod1 this time\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m1-->m0-->gen\n");
	printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        // Return must be 4
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 4);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 50);
    }

    // Test 5.1: Same test as 5 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 4 linked modulators.
    printf("\nTest 5.1: Same test as 5 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 4 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 4);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }
    
    // Look like circular circular complex modulators, but it isn't not circular !
 	printf("\nTest 6: false circular complex modulators, CC->mod3->mod1->mod3 => circular path ?\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "       m3<--m1\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m2-->m0-->gen\n");
 	printf(  " Question: is it a circular path ?\n");
 	printf(  " Answer: no, because mod3's src1 is without FLUID_MOD_LINK_SRC !\n");
    {
       fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 3);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        // Return must be 2
	    linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0); // invalid destination
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0); // path without destination
    }

    // Circular complex modulators
 	printf("\nTest 6.0: true circular complex modulators, CC->mod3->mod1->mod1 => circular path\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "          <--m1\n");
	printf(  "  CC-->m3-*->m1\n");
	printf(  "  CC-------->m2-->m0-->gen\n");
    {
        // restore test 5 state:
		fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0); // mod1->mod0
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_amount (mod3, 50);

		// new path: mod1->mod1, so new circurlar path is: CC->mod3->mod1->mod1 => circular path.
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 1);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        // It remains at least one linked path : mod2->mod0. Return must be 2
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated: mod3,mod1
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0);
    }

    // Test 6.0.1: Same test as 6.0 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 2 linked modulators.
    printf("\nTest 6.0.1: Same test as 6.0 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 2 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 2);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }

	// Another circular complex modulators
 	printf("\nTest 6.1: another circular complex modulators CC->mod2->mod0->mod1->mod0 => circular path\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "            m1<--m0\n");
	printf(  "  CC-->m3-->m1-->m0\n");
	printf(  "  CC------->m2-->m0\n");
    {
        // restore test 5 state:
		fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0); // mod1->mod0
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_amount (mod3, 50);

		// new path: mod0->mod1, so new circurlar path is: CC->mod2, mod0, mod1.
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 1);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;

        // Circular path is: CC->mod2, mod0, mod1.
		// Remaining path CC->mod3-> is without destination.
        // It remains no linked path : mod2->mod0. Return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT((mod0->path & FLUID_MOD_VALID) == 0); // part of circular path
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0); // part of circular path
        TEST_ASSERT((mod2->path & FLUID_MOD_VALID) == 0); // part of circular path
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0); // without destination.
    }

    // Test 6.1.1: Same test as 6.1 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 0 linked modulators.
    printf("\nTest 6.1.1: Same test as 6.1 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - NULL must be returned in linked_mod
        //  - 0 must be returned in linked_count
        linked_mod = mod0; // initialize linked_mod to not NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 0);
        TEST_ASSERT(linked_mod == NULL);
    }

    // circular complex modulators, but detected isolated because none of these
    // have a CC or GC on sources. These modulators have all src1 source linked.
    printf("\nTest 6.2: circular complex modulators m3->m2->m1->m0->m3\n");
    {
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 3);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 1);
        
        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 2);
        
        // return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT((mod0->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod2->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0);
    }
    
    // circular complex modulators, but detected isolated because none of these
    // have a CC or GC on sources. These modulators have all src1 source linked.
    printf("\nTest 6.3: circular complex modulators m3+m2->m1->m0->m3\n");
    {
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 3);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 1);
        
        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1);
        
        // return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        TEST_ASSERT((mod0->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod2->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0);
    }
    
    // circular complex modulators m1->m0->m3->m1, but detected isolated because none of these
    // have a CC or GC on sources. These modulators have all src1 source linked.
    printf("\nTest 6.4: circular complex modulators m1->m0->m3->m1 and cc->m2->gen\n");
    {
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 3);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, 21,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, GEN_MODENVHOLD);
        
        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1);
        
        // return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT((mod0->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0);
    }
    
    // circular complex modulators m3->m1->m3, but detected isolated because none of these
    // have a CC or GC on sources. These modulators have all src1 source linked.
    printf("\nTest 6.5: circular complex modulators m3->m1->m3 and cc->m2->m0->gen\n");
    {
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, 20,
                                FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_EXCLUSIVECLASS);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 3);
        
        fluid_mod_set_source1(mod2, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, 21,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1);
        
        // return must be 2
        linked_count = fluid_mod_check_linked_mod("test_zone_with_circular_linked_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0);
    }
    
    // invalid list of complex modulators: path without destination, isolated
 	printf("\nTest 7: invalid list of complex modulators: path without destination, isolated\n");
 	printf(  " List:m1,m0,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "            m1<--m0\n");
	printf(  "  CC-->m3-->m1-->m0\n");
	printf(  "  CC------->m2-->m0\n");
    {
        mod2->next = NULL;
        mod0->next = mod2;
        mod1->next = mod0;
        list_of_mods = mod1;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        // return must be 0
        linked_count = fluid_mod_check_linked_mod("test_zone_with_invalid_linked modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 0);

        // order is not changed
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod0);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0); // path without destination
        TEST_ASSERT((mod0->path & FLUID_MOD_VALID) == 0); // invalid isolated path
        TEST_ASSERT((mod2->path & FLUID_MOD_VALID) == 0); // path without destination
    }

    // invalid list of complex modulators: invalid destinations
 	printf("\nTest 8: invalid list of complex modulators: invalid destinations\n");
 	printf(  " List:m0,m1,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m2-->m0-->gen\n");
	printf(  "  CC-->m1-->42\n");
    {
        mod2->next = NULL;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 42);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        // return must be 2
        linked_count = fluid_mod_check_linked_mod("test_zone_with_invalid_linked modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 2);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);

        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0); // path without destination
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }

    // Invalid isolated paths
    printf("\nTest 9: same list than test 5, but path from m3 is isolated\n");
    printf(  " List:m0,m1,m2,m3\n");
    printf(  " Paths from any CC to ending modulator connected to gen:\n");
    printf(  "       m3-->m1\n");
    printf(  "            m1-->m0-->gen\n");
    printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);

        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1

        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;

        // return must be 2
        linked_count = fluid_mod_check_linked_mod("test_zone_with_isolated_modulators",
                                                   list_of_mods, 0, NULL, 0);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);

        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT((mod1->path & FLUID_MOD_VALID) == 0); // Invalided because isolated
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT((mod3->path & FLUID_MOD_VALID) == 0); // Invalided because isolated
    }

    // Test 9.1: Same test as 9 but with linked_mod not NULL. The function expects to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod should contain 2 linked modulator.
    printf("\nTest 9.1: same test as 9 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 2 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_mod_check_linked_mod("test_zone_with_linked_modulators",
                                                   list_of_mods, 0, &linked_mod, 0);
        TEST_ASSERT(linked_count == 2);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_mod_list(linked_mod);
    }

    // Test 10: check and get a list of linked modulators in a table supplied by the caller.
    // Check a given valid complex modulateur 2 times but returned in two distinct
    // lists of linked modulators(linked_mod1, linked_mod2):
    // - linked_mod1 is allocated internally by fluid_mod_check_linked_mod().
    // - linked_mod2 is a table given by the caller.
    // Both linked_mod1 and linked_mod2 lists are compared and expected to have identic
    // element.
    printf("\nTest 10: check and get a list of linked modulators in a table supplied by the caller\n");
    {
        /* One valid complex modulators with 5 members */
        /* table  :  att<-m0<-m1<-m2<-  */
        /*                m0<-m3<-m4<-  */
        fluid_mod_t mod_table[] =
        {
            /* Gen<-mod0-link<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                1.0      , 0.0 , 0   , NULL
            },
            /* mod0<-mod1<- */
            {
                // dest              , src1              , flags1        , src2              , flags2
                0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_VELOCITY, FLUID_MOD_GC,
                // amount, link, path, next
                2.0      , 0.0 , 0   , NULL
            },
            /* mod1<-mod2<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                3.0      , 0.0 , 0   , NULL
            },
            /* mod0<-mod3<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                4.0      , 0.0 , 0   , NULL
            },
            /* mod3<-mod4<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                3|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                5.0      , 0.0 , 0   , NULL
            }
        };
        //--- Input lists build from the table mod_table
        int mod_count; // number of modulators in table mod_table.
        fluid_mod_t * list_mod1;   // first input list, build from mod_table.
        fluid_mod_t * list_mod2;   // second input list, build from mod_table.

        //--- first output list of linked modulators (allocated internally):
        fluid_mod_t * linked_mod1; // first output list of linked modulators.
        int linked_count1_out;     // count of modulators returned in linked_mod1.
        //--- second output list of linked modulators. The table is given by the caller:
        fluid_mod_t * linked_mod2; // second output list of linked modulators (in table).
        int linked_count2_in;      // length of linked_mod2 table (in modulator number).
        int linked_count2_out;     // count of modulators returned in linked_mod2.

        //---------------------------------------------
        // build input lists list_mod1, list_mod2:
        mod_count = sizeof(mod_table) / sizeof(fluid_mod_t);
        TEST_ASSERT(mod_count == 5);
        list_mod1 = fluid_build_list(mod_table, mod_count);
        // list_mod1 and list_mod2 are expected not NULL, and count of modulators
        // is expected to be equal to mod_count.
        TEST_ASSERT(list_mod1 != NULL);
        TEST_ASSERT(fluid_mod_get_list_count(list_mod1) == mod_count);

        list_mod2 = fluid_build_list(mod_table, mod_count);
        TEST_ASSERT(list_mod2 != NULL);
        TEST_ASSERT(fluid_mod_get_list_count(list_mod2) == mod_count);
        // compare input lists list_mod1, and list_mod2.
        TEST_ASSERT(fluid_list_test_identity(list_mod1, list_mod2) == TRUE);

        //---------------------------------------------
        // building output list linked_mod1 by calling fluid_mod_check_linked_mod().
        // linked_mod1 is allocated internally.
        linked_mod1 = NULL;
        linked_count1_out = fluid_mod_check_linked_mod("linked_mod1(internal)",
                                                   list_mod1, 0, &linked_mod1, 0);
        // linked_mod1 is expected to be not NULL
        TEST_ASSERT(linked_mod1 != NULL);
        // Modulators in input list list_mod1 are fully valid. This leads to
        // linked_count1_out that is expected to be equal to mod_count.
        TEST_ASSERT(linked_count1_out == mod_count);

        //---------------------------------------------
        // building output list linked_mod2 by calling fluid_mod_check_linked_mod().
        // linked_mod2 is allocated externally on (stack) in a table.
        linked_count2_in = fluid_mod_get_list_count(list_mod2);
        linked_mod2 = alloca( linked_count2_in * sizeof(fluid_mod_t));
        // linked_mod2 is expected to be not NULL
        TEST_ASSERT(linked_mod2 != NULL);
        linked_count2_out = fluid_mod_check_linked_mod("linked_mod2(external)",
                                                   list_mod2, 0, &linked_mod2,
                                                   linked_count2_in);
        // Modulators in input list list_mod2 are fully valid. This leads to
        // linked_count2_out that is expected to be equal to mod_count.
        TEST_ASSERT(linked_count2_out == mod_count);
        // Both linked_mod1 and linked_mod2 lists are compared and expected to have
        // identic element.
        TEST_ASSERT(fluid_list_test_identity(linked_mod1, linked_mod2) == TRUE);

        // freeing: linked_mod2 is supplied as a table on stack. It must not freed !
        delete_fluid_mod_list(list_mod1);
        delete_fluid_mod_list(list_mod2);
        delete_fluid_mod_list(linked_mod1);
	}

    // Test 11: same as test 10 except input list_mod2 is a table supplied by the caller
    // - List_mod2 is expected to be initialized as a list. Both list_mod1 and list_mod2
    //   lists are compared and expected to have identic element.
    //
    // - Both linked_mod1 and linked_mod2 lists are compared and expected to have identic
    // element.
    printf("\nTest 11: same as test 10 except list_mod2 is a table supplied by the caller\n");
    {
        /* One valid complex modulators with 5 members */
        /* table  :  att<-m0<-m1<-m2<-  */
        /*                m0<-m3<-m4<-  */
        fluid_mod_t mod_table[] =
        {
            /* Gen<-mod0-link<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                1.0      , 0.0 , 0   , NULL
            },
            /* mod0<-mod1<- */
            {
                // dest              , src1              , flags1        , src2              , flags2
                0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_VELOCITY, FLUID_MOD_GC,
                // amount, link, path, next
                2.0      , 0.0 , 0   , NULL
            },
            /* mod1<-mod2<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                3.0      , 0.0 , 0   , NULL
            },
            /* mod0<-mod3<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                4.0      , 0.0 , 0   , NULL
            },
            /* mod3<-mod4<- */
            {
                // dest              , src1              , flags1        , src2            , flags2
                3|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
                // amount, link, path, next
                5.0      , 0.0 , 0   , NULL
            }
        };
        //--- Input lists build from the table mod_table
        int mod_count; // number of modulators in table mod_table.
        fluid_mod_t * list_mod1;   // first input list, build from mod_table.
        fluid_mod_t * list_mod2;   // supplied by the caller as a table.

        //--- first output list of linked modulators (allocated internally):
        fluid_mod_t * linked_mod1; // first output list of linked modulators.
        int linked_count1_out;     // count of modulators returned in linked_mod1.
        //--- second output list of linked modulators. The table is given by the caller:
        fluid_mod_t * linked_mod2; // second output list of linked modulators (in table).
        int linked_count2_in;      // length of linked_mod2 table (in modulator number).
        int linked_count2_out;     // count of modulators returned in linked_mod2.

        //---------------------------------------------
        // Same as test 10: build input lists list_mod1:
        mod_count = sizeof(mod_table) / sizeof(fluid_mod_t);
        TEST_ASSERT(mod_count == 5);
        list_mod1 = fluid_build_list(mod_table, mod_count);
        // list_mod1 and list_mod2 are expected not NULL, and count of modulators
        // is expected to be equal to mod_count.
        TEST_ASSERT(list_mod1 != NULL);
        TEST_ASSERT(fluid_mod_get_list_count(list_mod1) == mod_count);

        //---------------------------------------------
        // Same as test 10: building output list linked_mod1 by calling fluid_mod_check_linked_mod().
        // list_mod1 is supplied as a list.
        // linked_mod1 is allocated internally.
        linked_mod1 = NULL;
        linked_count1_out = fluid_mod_check_linked_mod("linked_mod1(internal)",
                                                        list_mod1, 0, &linked_mod1, 0);
        // linked_mod1 is expected to be not NULL
        TEST_ASSERT(linked_mod1 != NULL);
        // Modulators in input list list_mod1 are fully valid. This leads to
        // linked_count1_out that is expected to be equal to mod_count.
        TEST_ASSERT(linked_count1_out == mod_count);

        //---------------------------------------------
        // building output list linked_mod2 by calling fluid_mod_check_linked_mod().
        // list_mod2 is supplied as a table.
        list_mod2 = mod_table;

        // linked_mod2 is allocated externally on (stack) in a table.
        linked_count2_in = mod_count;
        linked_mod2 = alloca( linked_count2_in * sizeof(fluid_mod_t));
        // linked_mod2 is expected to be not NULL
        TEST_ASSERT(linked_mod2 != NULL);

        linked_count2_out = fluid_mod_check_linked_mod("list_mod2(table)-linked_mod2(external)",
                                                        list_mod2, mod_count, &linked_mod2,
                                                        linked_count2_in);

        // Table list_mod2 is espected to be initialized as a list.
        TEST_ASSERT(fluid_mod_get_list_count(list_mod2) == mod_count);
        // Both list_mod1 and list_mod2 lists are compared and expected to have identic
        // element.
        TEST_ASSERT(fluid_list_test_identity(list_mod1, list_mod2) == TRUE);

        // Same as test 10. Modulators in input list list_mod2 are fully valid. This leads to
        // linked_count2_out that is expected to be equal to mod_count.
        TEST_ASSERT(linked_count2_out == mod_count);
        // Both linked_mod1 and linked_mod2 lists are compared and expected to have
        // identic element.
        TEST_ASSERT(fluid_list_test_identity(linked_mod1, linked_mod2) == TRUE);

        // freeing: list_mod2 is supplied as a table. It must not freed !
        //          linked_mod2 is supplied as a table on stack. It must not freed !
        delete_fluid_mod_list(list_mod1);
        delete_fluid_mod_list(linked_mod1);
    }

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    delete_fluid_mod(mod3);
    delete_fluid_mod(mod4);
    delete_fluid_mod(mod5);
    
    return EXIT_SUCCESS;
}


/*
 A convenience function that builds a list of modulators from a modulator table
 mod_table (duplicated in test_fluid_zone_check_mod.c).
*/
static fluid_mod_t * fluid_build_list(fluid_mod_t mod_table[], int count_mod)
{
	int i;
	fluid_mod_t * prev = NULL;
	fluid_mod_t *list_mod = NULL;
	/* build list_mod containing test modulators from mod_table */
	for(i = 0; i < count_mod; i++)
	{
		/* Make a copy of this modulator */
		fluid_mod_t * mod = new_fluid_mod();
		if(mod == NULL)
		{
			FLUID_LOG(FLUID_ERR, "Out of memory");
			delete_fluid_mod_list(list_mod);

			return NULL;
		}
		fluid_mod_clone(mod, &mod_table[i]);
		mod->next = NULL;
		/* add to list_mode */
		if(prev == NULL)
		{
			list_mod = mod;
		}
		else
		{
			prev->next = mod;
		}
		prev =mod;
	}
	return list_mod;
}

/**
 * Return TRUE if list list_mod1 is identic to list list_mod2,
 * FALSE otherwise.
 * Both list are identic if that have equal count of modulators and
 * respective modulator's structure are fully equal.
*/
static int fluid_list_test_identity(fluid_mod_t *list_mod1, fluid_mod_t *list_mod2)
{
    int count1 = fluid_mod_get_list_count(list_mod1);
    int count2 = fluid_mod_get_list_count(list_mod2);
    if (count1 != count2)
    {
        return FALSE;
    }

    while(list_mod1)
    {
        if( ! fluid_mod_test_identity(list_mod1, list_mod2) ||
               list_mod1->amount != list_mod2->amount)
        {
            return FALSE;
        }
        list_mod1 = list_mod1->next;
        list_mod2 = list_mod2->next;
    }
    return TRUE;
}
