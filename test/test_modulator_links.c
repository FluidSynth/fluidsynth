
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_mod.h"
#include "utils/fluid_sys.h"

int fluid_zone_check_linked_mod(char *zone_name, fluid_mod_t *list_mod);

// tests the linked "nature" of modulators, i.e. fluid_zone_check_linked_mod()
int main(void)
{    
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();
    fluid_mod_t *mod3 = new_fluid_mod();
    fluid_mod_t *mod4 = new_fluid_mod();
    
    fluid_mod_t *list_of_mods = NULL;
    
    // set up a list of simple modulators
    {
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
        
        fluid_mod_set_source1(mod1, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 100);
        fluid_mod_set_dest   (mod1, GEN_VOLENVATTACK);
        
        fluid_mod_set_source1(mod2, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod2, 11025);
        fluid_mod_set_dest   (mod2, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod3, 34, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, 0);
        fluid_mod_set_amount (mod3, 960);
        fluid_mod_set_dest   (mod3, GEN_ATTENUATION);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with simple modulators", list_of_mods) == FALSE);
        
        // order of modulators remains the same
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
    }
    
    // set up a valid list of complex modulators
    {
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
    
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 100);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod2, 20,
                                FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 200);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 300);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        // order not changed
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
    }
    
    // same list, but changed order
    {
        mod2->next = NULL;
        mod3->next = mod2;
        mod1->next = mod3;
        list_of_mods = mod1;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod3);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
    }
    
    // same list, but with additional mod that points to mod2 without mod2 having FLUID_MOD_LINK_SRC
    {
        fluid_mod_set_source1(mod4, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod4, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod4, 50);
        fluid_mod_set_dest   (mod4, FLUID_MOD_LINK_DEST | 1); // link to mod2
        
        mod3->next = mod4;
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == mod4);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod4) == 0); // invalidated because mod2 without FLUID_MOD_LINK_SRC
    }
    
        
    // same list, with additional mod4 and valid mod2 this time
    {
        fluid_mod_set_source1(mod2, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_amount (mod4, 50);
        
        mod3->next = mod4;
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == mod4);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod4) == 50);
    }
            
    // circular complex modulators
    {
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 3);
        
        mod3->next = mod4;
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with circular linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == mod4);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod4) == 0);
    }
    
    // invalid list of complex modulators: the first modulator should not have a linked destination
    {
        mod3->next = NULL;
        mod1->next = mod3;
        mod2->next = mod1;
        list_of_mods = mod2;
    
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 100);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod2, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 200);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 300);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with invalid linked modulators", list_of_mods) == FALSE);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod2);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
    }

    // invalid list of complex modulators: valid first modulator but invalid destinations
    {
        mod3->next = NULL;
        mod2->next = mod3;
        mod1->next = mod2;
        list_of_mods = mod1;
    
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 100);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod2, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 200);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 42);
        
        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 300);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with invalid linked modulators", list_of_mods) == TRUE);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 300);
    }

    
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    delete_fluid_mod(mod3);
    delete_fluid_mod(mod4);
    
    return EXIT_SUCCESS;
}
