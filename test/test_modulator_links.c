
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_mod.h"
#include "utils/fluid_sys.h"

int fluid_list_check_linked_mod(char *zone_name, fluid_mod_t *list_mod,
                            fluid_mod_t **linked_mod);
void delete_fluid_list_mod(fluid_mod_t *list_mod);

// tests the linked "nature" of modulators, i.e. fluid_list_check_linked_mod()
int main(void)
{
    int linked_count;
	fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();
    fluid_mod_t *mod3 = new_fluid_mod();
    
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
        linked_count = fluid_list_check_linked_mod("test zone with simple modulators", list_of_mods, NULL);
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

    // Test 1.1: Same test as 1 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod doesn't contain any linked modulator.
    printf("\nTest 1.1: same test as 1 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - NULL must be returned in linked_mod
        //  - 0 must be returned in linked_count
        linked_mod = mod0; // initialize linked_mod to non NULL.
        linked_count = fluid_list_check_linked_mod("test zone with simple modulators", list_of_mods, &linked_mod);
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
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, NULL);
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
    
    // Test 2.1: Same test as 2 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 3 linked modulator.
    printf("\nTest 2.1: same test as 2 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
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
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, NULL);
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

    // Test 3.1: Same test as 3 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 3 linked modulator.
    printf("\nTest 3.1: same test as 3 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
    }
    
    // same list that test 3, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC
    printf("\nTest 4: same list that test 3, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m1-->m0-->gen\n");
	printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        // Return must be 3
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, NULL);
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
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // invalidated because mod1 without FLUID_MOD_LINK_SRC
    }

    // Test 4.1: Same test as 4 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 3 linked modulator.
    printf("\nTest 4.1: Same test as 4 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 3);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
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
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, NULL);
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

    // Test 5.1: Same test as 5 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 4 linked modulators.
    printf("\nTest 5.1: Same test as 5 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 3 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 4);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
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
	    linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // invalid destination
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // path without destination
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated: mod3,mod1
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
    }

    // Test 6.0.1: Same test as 6.0 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 2 linked modulators.
    printf("\nTest 6.0.1: Same test as 6.0 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 2 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 2);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
		TEST_ASSERT(fluid_mod_get_amount(mod0) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // without destination.
    }

    // Test 6.1.1: Same test as 6.1 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 0 linked modulators.
    printf("\nTest 6.1.1: Same test as 6.1 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - NULL must be returned in linked_mod
        //  - 0 must be returned in linked_count
        linked_mod = mod0; // initialize linked_mod to not NULL.
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, &linked_mod);
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 0);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
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
        linked_count = fluid_list_check_linked_mod("test zone with circular linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
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
        linked_count = fluid_list_check_linked_mod("test zone with invalid linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 0);

        // order is not changed
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod0);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // path without destination
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 0); // invalid isolated path
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0); // path without destination
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
        linked_count = fluid_list_check_linked_mod("test zone with invalid linked modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 2);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);

        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // path without destination
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
        linked_count = fluid_list_check_linked_mod("test zone with isolated modulators", list_of_mods, NULL);
        TEST_ASSERT(linked_count == 2);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);

        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // Invalided because isolated
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);// Invalided because isolated
    }

    // Test 9.1: Same test as 9 but with linked_mod not NULL. The function is excepted to return any
    // linked modulator list in linked_mod pointer.
    // Actually linked_mod contains 2 linked modulator.
    printf("\nTest 9.1: same test as 9 but with linked_mod not NULL\n");
    {
        fluid_mod_t * linked_mod;
        // On return:
        //  - not NULL must be returned in linked_mod
        //  - 2 must be returned in linked_count
        linked_mod = NULL; // initialize linked_mod to NULL.
        linked_count = fluid_list_check_linked_mod("test zone with linked modulators", list_of_mods, &linked_mod);
        TEST_ASSERT(linked_count == 2);
        TEST_ASSERT(linked_mod != NULL);
        delete_fluid_list_mod(linked_mod);
    }

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    delete_fluid_mod(mod3);
    
    return EXIT_SUCCESS;
}
