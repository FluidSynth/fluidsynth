/*----------------------------------------------------------------------------
Test of fluid_zone_check_mod() , fluid_linked_mod_test_identity() functions.

Usage:
 test_fluid_zone_check_mod      execute all tests
 test_fluid_zone_check_mod help print usage
 test_fluid_zone_check_mod list print list of test
 test_fluid_zone_check_mod 1_2  execute only one test number
                                (i.e \"test 1_2: unlinked, sources src1 none\")
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_mod.h"
#include "utils/fluid_sys.h"

//----------------------------------------------------------------------------
/* external functions */

// implemented in fluid_mod.c
// Required: fluid_dump_modulator() need to add DEBUG preprocessor macro while
// compiling fluidsynth.
void fluid_dump_modulator(fluid_mod_t * mod);

// implemented in fluid_defsfont.c
int fluid_zone_check_mod(char *zone_name, fluid_mod_t **list_mod,
                         fluid_mod_t **linked_mod);
// implemented in fluid_defsfont.c
void delete_fluid_list_mod(fluid_mod_t *list_mod);

// implemented in fluid_defsfont.c
void fluid_dump_linked_mod(fluid_mod_t *mod, int mod_idx, int offset);

//----------------------------------------------------------------------------
static fluid_mod_t * fluid_build_list(fluid_mod_t mod_table[], int count_mod);
static int test_fluid_zone_check_mod(char * name_test,
					 fluid_mod_t mod_table0[],int count_mod0,
					 fluid_mod_t mod_table1[],int count_mod1
					 );
static void fluid_linked_mod_dump_test_identity(fluid_mod_t *mod0, fluid_mod_t *mod1);

static int all_test_fluid_zone_check_mod(char *num_test);

static void print_lists(fluid_mod_t *list_mod, fluid_mod_t *linked_mod);
static void fluid_dump_list_linked_mod(fluid_mod_t *mod);
static void print_list_linked_mod(char *header, char *name_list, fluid_mod_t *mod);
static void fluid_dump_list_mod(fluid_mod_t *mod);
static void print_list_mod(char *name_list, fluid_mod_t *mod);

/*----------------------------------------------------------------------------
 test tables: to add a test see all_test_fluid_zone_check_mod() below
------------------------------------------------------------------------------*/
/*------ unlinked modulators tests -------------------------------------------*/

/* test 1: sources non-cc, modulator unlinked, sources non-cc valid */
char test_1_unlinked_non_cc_valid[] = "test 1: unlinked, sources non-cc valid";
fluid_mod_t mod_table_1[] =
{
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0     , 0.0 , NULL
	}
};

/* test 1_1: sources non-cc, modulator unlinked sources non-cc valid, amount = 0 */
char test_1_1_unlinked_non_cc_valid[] = "test 1_1: unlinked, sources non-cc valid, amount = 0";
fluid_mod_t mod_table_1_1[] =
{
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	}
};

/* test 1_2: sources non-cc src1 none, modulator unlinked   */
char test_1_2_unlinked_non_cc_valid[] = "test 1_2: unlinked, sources non-cc src1 none";
fluid_mod_t mod_table_1_2[] =
{
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_NONE   , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0      , 0.0 , NULL
	},
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_KEY    , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	}
};

/* test 1_3: sources non-cc src2 none, modulator unlinked */
char test_1_3_unlinked_non_cc_invalid[] = "test 1_3: unlinked, sources non-cc src2 none, bipolar";
fluid_mod_t mod_table_1_3[] =
{
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION, FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC|FLUID_MOD_BIPOLAR,
		// amount, link, next
		0.0      , 0.0 , NULL
	}
};

/* test 2: sources non-cc, modulator unlinked, sources non-cc invalid */
char test_2_unlinked_non_cc_invalid[] = "test 2: unlinked, sources non-cc invalid";
fluid_mod_t mod_table_2[] =
{
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION, FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	},
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION , 1                , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0      , 0.0 , NULL
	}
};

/* test 3: sources cc, modulator unlinked, sources cc valid */
char test_3_unlinked_cc_valid[] = "test 3: unlinked, sources cc valid";
fluid_mod_t mod_table_3[] =
{
	{
		// dest         , src1             , flags1      , src2          , flags2
		GEN_ATTENUATION , 3                , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0      , 0.0 , NULL
	}
};

/* test 4: sources cc, modulator unlinked, sources cc invalid */
char test_4_unlinked_cc_invalid[] = "test 4: unlinked, sources cc invalid";
fluid_mod_t mod_table_4[] =
{
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	},
	{
		// dest         , src1              , flags1      , src2          , flags2
		2               , 0                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0      , 0.0 , NULL
	}
};

/* test 5: modulators identic */
char test_5_mod_identic[] = "test 5: modulator identic";
fluid_mod_t mod_table_5[] =
{
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	},
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , 3                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		10.0     , 0.0 , NULL
	},
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION, FLUID_MOD_VELOCITY , FLUID_MOD_GC , FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	},
	{
		// dest         , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	}
};

/*------ linked modulators tests ---------------------------------------------*/

/* test 6: modulator linked valid: src1->mod0->mod2 */
char test_6_mod_linked[] = "test 6: linked modulators valid: src1->mod0->mod2";
fluid_mod_t mod_table_6[] =
{
	{
		// dest              , src1              , flags1      , src2          , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest         , src1                  , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_VELOCITY    , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest         , src1                  , flags1      , src2          , flags2
		GEN_ATTENUATION , FLUID_MOD_LINK_SRC    , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* test 6_1: linked modulators valid: src1->mod4->mod0->mod2->gen1
                                            src2->mod1->mod2

                                      src2->mod5->mod7->gen2
                                            mod6->mod7
*/
char test_6_1_mod_linked[] = "test 6_1: linked modulators valid: src1->mod0->mod2, src2->mod1->mod2";
fluid_mod_t mod_table_6_1[] =
{
	/* mod complexe */
	{
		// dest              , src1              , flags1      , src2          , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_KEY     , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_KEY     , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	},
    /* mod complexe */
	{
		// dest              , src1              , flags1      , src2          , flags2
		7|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		6.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		7|FLUID_MOD_LINK_DEST, FLUID_MOD_KEY     , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		7.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD, FLUID_MOD_LINK_SRC       ,FLUID_MOD_GC , FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		8.0      , 0.0 , NULL
	}
};

/* test 6_2: linked modulators valid: src2->mod3->mod0->mod2 */
char test_6_2_mod_linked[] = "test 6_2: linked modulators valid: mod3->mod0->mod2";
fluid_mod_t mod_table_6_2[] =
{
	{
		// dest              , src1              , flags1      , src2          , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	}
};

//-----
/* test 6_3: 2 complex modulators identic:  att<-mod0<-mod1<- ,  att<-mod0<-mod1<- */
char test_6_3_mod_linked[] = "test 6_3: complex mod identical: att<-mod0<-mod1<-, att<-mod0<-mod1<-";
/* table 0:  att<-mod0<-mod1<-  */
fluid_mod_t mod_table_6_3_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	}
};

/* table 1:  att<-mod0<-mod1<-  */
fluid_mod_t mod_table_6_3_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

//-----
/* test 6_4: 2 complex modulators identic:  att<-mod0<-mod1<-mod2<- ,  att<-mod0<-mod1<- */
char test_6_4_mod_linked[] = "test 6_4: complex mod not identical: att<-mod0<-mod1<-mod2<-, att<-mod0<-mod1<-";
/* table 0:  att<-mod0<-mod1<-mod2<-  */
fluid_mod_t mod_table_6_4_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* table 1:  att<-mod0<-mod1<-  */
fluid_mod_t mod_table_6_4_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

//-----
/* test 6_5: 2 complex modulators identic:  att<-mod0<-mod1<-mod2<- ,  att<-mod0<-mod1<-mod2<- */
char test_6_5_mod_linked[] = "test 6_5: complex mod identical: att<-mod0<-mod1<-mod2<-, att<-mod0<-mod1<-mod2<-";
/* table 0:  att<-mod0<-mod1<-mod2<-  */
fluid_mod_t mod_table_6_5_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* table 1:  att<-mod0<-mod1<-mod2<-  */
fluid_mod_t mod_table_6_5_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		6.0      , 0.0 , NULL
	}
};

//-----
/* test 6_6: 2 complex modulators identic:  att<-mod0<-mod1<-mod2<- ,  att<-mod0<-mod1<- */
/*                                                                          mod0<-mod2<- */
char test_6_6_mod_linked[] = "test 6_6: complex cm0:att<-mod0<-mod1<-mod2<-, cm1:att<-mod0<-mod1, mod0<-mod2<-";
/* table 0:  att<-mod0<-mod1<-mod2<-  */
fluid_mod_t mod_table_6_6_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* table 1:  att<-mod0<-mod1<-mod2<-  */
/*                mod0<-mod2<-  */
fluid_mod_t mod_table_6_6_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_NONE    , FLUID_MOD_GC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	},
	/* mod0<-mod2<- */
	{
		// dest              , src1              , flags1      , src2          , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
		// amount, link, next
		6.0      , 0.0 , NULL
	}
};

//-----
/* test 6_7: 2 complex modulators identic:  att<-mod0<-mod1<- ,  att<-mod0<-mod1<- */
/*                                               mod0<-mod2<-         mod0<-mod2<- */
char test_6_7_mod_linked[] = "test 6_7: complex cm0:att<-m0<-m1 m0<-m2<-, cm1:att<-m0<-m1 m0<-m2<-";
/* table 0:  att<-mod0<-mod1<-mod2<-  */
/*                mod0<-mod2<-  */
fluid_mod_t mod_table_6_7_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2             , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, 5                 ,FLUID_MOD_CC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2              , flags2
		0|FLUID_MOD_LINK_DEST, 4                 , FLUID_MOD_CC, FLUID_MOD_VELOCITY, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod0<-mod2<- */
	{
		// dest              , src1              , flags1      , src2              , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE    , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* table 1:  att<-mod0<-mod1<-mod2<-  */
/*                mod0<-mod2<-  */
fluid_mod_t mod_table_6_7_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1      , src2              , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC, 5                 , FLUID_MOD_CC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1      , src2              , flags2
		0|FLUID_MOD_LINK_DEST, 4                 , FLUID_MOD_CC, FLUID_MOD_VELOCITY, FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	},
	/* mod0<-mod2<- */
	{
		// dest              , src1              , flags1      , src2              , flags2
		0|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC, FLUID_MOD_NONE    , FLUID_MOD_GC,
		// amount, link, next
		6.0      , 0.0 , NULL
	}
};

//-----
/* test 6_8: 2 complex modulators identic:  att<-m0<-m1<-m2 ,  att<-m0<-m1<-m2 */
/*                                               m0<-m3<-m4         m0<-m3<-m4 */
char test_6_8_mod_linked[] = "test 6_8: complex cm0:att<-m0<-m1<-m2 m0<-m3<-m4, cm1:att<-m0<-m1<-m2 m0<-m3<-m4";
/* table 0:  att<-m0<-m1<-m2<-  */
/*                m0<-m3<-m4<-  */
fluid_mod_t mod_table_6_8_0[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1        , src2              , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_VELOCITY, FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	},
	/* mod0<-mod3<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	/* mod3<-mod4<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		3|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		5.0      , 0.0 , NULL
	}
};

/* table 1:  att<-m0<-m1<-m2<-  */
/*                m0<-m3<-m4<-  */
fluid_mod_t mod_table_6_8_1[] =
{
	/* Gen<-mod0-link<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_VOLENVHOLD       , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		6.0      , 0.0 , NULL
	},
	/* mod0<-mod1<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		7.0      , 0.0 , NULL
	},
	/* mod1<-mod2<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		1|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		8.0      , 0.0 , NULL
	},
	/* mod0<-mod3<- */
	{
		// dest              , src1              , flags1        , src2              , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_VELOCITY, FLUID_MOD_GC,
		// amount, link, next
		9.0      , 0.0 , NULL
	},
	/* mod3<-mod4<- */
	{
		// dest              , src1              , flags1        , src2            , flags2
		3|FLUID_MOD_LINK_DEST, 2                 , FLUID_MOD_CC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		10.0      , 0.0 , NULL
	}
};

/* test 7: linked modulator, invalid destination: src1->mod0->mod2(src1 unlinked) */
char test_7_mod_linked[] = "test 7: linked modulators invalid destination: src1->mod0->mod2(src1 unlinked)";
fluid_mod_t mod_table_7[] =
{
	{
		// dest              , src1                 , flags1        , src2            , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY   , FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest              , src1                 , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_KEYPRESSURE, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1                 , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY   , FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};


/* test 8: linked modulator invalid: src1->mod0->mod2(amount=0) */
char test_8_mod_linked[] = "test 8: linked modulators invalid: src1->mod0->mod2(amount=0)";
fluid_mod_t mod_table_8[] =
{
	{
		// dest              , src1              , flags1        , src2            , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		0.0      , 0.0 , NULL
	}
};

/* test 9: linked modulator invalid: src1->mod0->mod3(invalid index) */
char test_9_mod_linked[] = "test 9: linked modulators invalid: src1->mod0->mod3(invalid index)";
fluid_mod_t mod_table_9[] =
{
	{
		// dest              , src1                , flags1        , src2            , flags2
		3|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY  , FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest              , src1                 , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_KEYPRESSURE, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1               , flags1          , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY , FLUID_MOD_GC    , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};


/* test 10: invalid isolated linked modulator : src1 linked->mod0->mod2 */
char test_10_mod_linked[] = "test 10: linked modulators invalid isolated: src1 linked->mod0->mod2";
fluid_mod_t mod_table_10[] =
{
	{
		// dest              , src1              , flags1        , src2            , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_VELOCITY, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_ATTENUATION      , FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	}
};

/* test 11: invalid circular linked modulator: src2->mod3->mod0->mod2->mod0 */
char  test_11_mod_linked[] = "test 11: linked modulators circular: mod3->mod0->mod2->mod0";
fluid_mod_t mod_table_11[] =
{
	{
		// dest              , src1              , flags1        , src2            , flags2
		2|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		2.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		GEN_ATTENUATION, FLUID_MOD_VELOCITY      , FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		4.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_LINK_SRC, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		3.0      , 0.0 , NULL
	},
	{
		// dest              , src1              , flags1        , src2            , flags2
		0|FLUID_MOD_LINK_DEST, FLUID_MOD_VELOCITY, FLUID_MOD_GC  , FLUID_MOD_NONE  , FLUID_MOD_GC,
		// amount, link, next
		1.0      , 0.0 , NULL
	}
};

/* the list of test name, displayed by: test_fluid_zone_check_mod list */
char *test_names_list[] =
{
    // tests unlinked modulators
	test_1_unlinked_non_cc_valid, 
	test_1_1_unlinked_non_cc_valid, test_1_2_unlinked_non_cc_valid, 
	test_1_3_unlinked_non_cc_invalid, test_2_unlinked_non_cc_invalid,
	test_3_unlinked_cc_valid,  test_4_unlinked_cc_invalid, 
	test_5_mod_identic, 
	
    // tests valid linked modulators
	test_6_mod_linked, test_6_1_mod_linked, test_6_2_mod_linked,
	// tests complex modulators identic
	test_6_3_mod_linked, test_6_4_mod_linked, test_6_5_mod_linked, test_6_6_mod_linked,
	test_6_7_mod_linked,test_6_8_mod_linked, 
	
    // tests invalid linked modulators
	test_7_mod_linked, test_8_mod_linked, test_9_mod_linked, test_10_mod_linked,
	test_11_mod_linked,
	NULL //End of table
};


//-----------------------------------------------------------------------------
const char *usage ="\n\
 test_fluid_zone_check_mod      execute all tests\n\
 test_fluid_zone_check_mod help print usage\n\
 test_fluid_zone_check_mod list print list of test\n\
 test_fluid_zone_check_mod 1_2  execute only one test number(i.e \"test 1_2: unlinked, sources src1 none\")\n\
\n";
/*
  Test of fluid_zone_check_mod() , fluid_linked_mod_test_identity() functions

  test_fluid_zone_check_mod  [argument]
*/
int main(int argc, char **argv)
{
	char *arg = NULL /* execute all test */;
	if (argc > 1)
	{
		arg = argv[1];
		if(strcmp("help", arg) == 0)
		{
			printf("Usage:%s",usage);
			return 0;
		}
		/* print list of tests name */
		else if(strcmp("list", arg) == 0)
		{
			int i;
			for (i =0; test_names_list[i]!=NULL ; i++)
			{
				printf("%s\n",test_names_list[i]);
			}
			return 0;
		}
		/* or execute one test */
	}
	all_test_fluid_zone_check_mod(arg);
    return EXIT_SUCCESS;
}


/* return TRUE if arg is NULL or equal to num_test */
static int is_arg_num_test(char *arg, char *num_test)
{
	return (arg == NULL || strcmp(arg, num_test) == 0);
}


/*
 Does all test:
  - test of fluid_zone_check_mod() and
  - test of fluid_linked_mod_test_identity().

  Note: These tests display results on the console and are intended to be
  interpreted by a human reading the result.
  Unlike tests done in test_modulator_link.c, these tests aren't intended to
  do automatic tests using TEST_ASSERT.

 The modulators have to be presented in tables for convenience.
 It is easy to add a test:
  1) define the name of the test: name_x.
  2) add the name in test_names_list.
  2) define the first table mod_table0_x.
  3) define the second table mod_table1_x (only if you want to test
     complex modulators identic).
  4) call test_fluid_zone_check_mod(name_x, 
                               mod_table0_x, mod_count0_x,
                               mod_table1_x, mod_count1_x)


  @param arg, pointer of string describing the test number.
  if NULL all the tests are done.
  For example:  "1"  for test 1, "1_1" for test 1_1, ...

  @return FLUID_OK or FLUID_FAILED

*/

static int all_test_fluid_zone_check_mod(char *arg)
{
	FLUID_LOG(FLUID_INFO, "========================== fluid_all_test_check_mod =========================");

	/*------ unlinked modulators tests -------------------------------------*/
	/* test 1: unliked modulator, sources non-cc valid */ //Ok
	if (is_arg_num_test(arg,"1"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_1_unlinked_non_cc_valid,
		                            mod_table_1, sizeof(mod_table_1)/sizeof(fluid_mod_t),NULL,0));
	}
	/* test 1.1: unliked modulator, sources non-cc valid, amount = 0 */ //Ok
	if (is_arg_num_test(arg,"1_1"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_1_1_unlinked_non_cc_valid,
		                            mod_table_1_1, sizeof(mod_table_1_1)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 1.2: unliked modulator, sources non-cc src1 none  */ //Ok
	if (is_arg_num_test(arg,"1_2"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_1_2_unlinked_non_cc_valid,
		                            mod_table_1_2, sizeof(mod_table_1_2)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 1.3: unliked modulator, sources non-cc src2 none  */ //Ok
	if (is_arg_num_test(arg,"1_3"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_1_3_unlinked_non_cc_invalid,
		                            mod_table_1_3, sizeof(mod_table_1_3)/sizeof(fluid_mod_t),NULL,0));
	}
    
	/* test 2: unliked modulator, sources non-cc invalid */ //Ok
	if (is_arg_num_test(arg,"2"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_2_unlinked_non_cc_invalid,
		                            mod_table_2, sizeof(mod_table_2)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 3: unliked modulator, sources cc valid */ //Ok
	if (is_arg_num_test(arg,"3"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_3_unlinked_cc_valid,
		                            mod_table_3, sizeof(mod_table_3)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 4: sources cc: unliked modulator, sources cc invalid */ //Ok
	if (is_arg_num_test(arg,"4"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_4_unlinked_cc_invalid,
		                            mod_table_4, sizeof(mod_table_4)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 5::modulators identic */ // Ok
	if (is_arg_num_test(arg,"5"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_5_mod_identic,
		                            mod_table_5, sizeof(mod_table_5)/sizeof(fluid_mod_t),NULL,0));
	}
	
	/*------ valid linked modulators tests -------------------------------------*/
	/* test 6: valid linked modulators: mod0->mod2 */ // Ok
	if (is_arg_num_test(arg,"6"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_mod_linked,
		                            mod_table_6, sizeof(mod_table_6)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 6_1: valid linked modulators: src1->mod0->mod2 */ // Ok
	/*                                    src2->mod1->mod2 */
	if (is_arg_num_test(arg,"6_1"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_1_mod_linked,
		                            mod_table_6_1, sizeof(mod_table_6_1)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 6_2: valid linked modulators: src2->mod3->mod0->mod2 */ //Ok
	if (is_arg_num_test(arg,"6_2"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_2_mod_linked,
		                            mod_table_6_2, sizeof(mod_table_6_2)/sizeof(fluid_mod_t),NULL,0));
	}

	//--- complex modulators identity tests ----
	/* test 6_3: complex modulators identity test:  att<-mod0->mod1<- ,  att<-mod0->mod1<- */ //Ok
	if (is_arg_num_test(arg,"6_3"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_3_mod_linked,
		                            mod_table_6_3_0, sizeof(mod_table_6_3_0)/sizeof(fluid_mod_t),
		                            mod_table_6_3_1, sizeof(mod_table_6_3_1)/sizeof(fluid_mod_t)));
	}

	/* test 6_4: complex modulators identity test:  att<-mod0->mod1<-mod2<-   att<-mod0->mod1<- */ //Ok
	if (is_arg_num_test(arg,"6_4"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_4_mod_linked,
		                            mod_table_6_4_0, sizeof(mod_table_6_4_0)/sizeof(fluid_mod_t),
		                            mod_table_6_4_1, sizeof(mod_table_6_4_1)/sizeof(fluid_mod_t)));
	}

	/* test 6_5: complex modulators identity test:  att<-mod0->mod1<-mod2<-, att<-mod0->mod1<-mod2<-*/ //Ok
	if (is_arg_num_test(arg,"6_5"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_5_mod_linked,
		                            mod_table_6_5_0, sizeof(mod_table_6_5_0)/sizeof(fluid_mod_t),
		                            mod_table_6_5_1, sizeof(mod_table_6_5_1)/sizeof(fluid_mod_t)));
	}

	/* test 6_6: complex modulators identity test:  att<-mod0<-mod1<-mod2<- ,  att<-mod0->mod1<- */ //Ok
	/*                                                                              mod0->mod2<- */
	if (is_arg_num_test(arg,"6_6"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_6_mod_linked,
		                            mod_table_6_6_0, sizeof(mod_table_6_6_0)/sizeof(fluid_mod_t),
		                            mod_table_6_6_1, sizeof(mod_table_6_6_1)/sizeof(fluid_mod_t)));
	}

	/* test 6_7: complex modulators identity test:  att<-mod0<-mod1<-mod2<- ,  att<-mod0->mod1<- */ //Ok
	/*                                                   mod0->mod2<-               mod0->mod2<- */
	if (is_arg_num_test(arg,"6_7"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_7_mod_linked,
		                        mod_table_6_7_0, sizeof(mod_table_6_7_0)/sizeof(fluid_mod_t),
		                        mod_table_6_7_1, sizeof(mod_table_6_7_1)/sizeof(fluid_mod_t)));
	}


	/* test 6_8: complex modulators identity test:  att<-m0<-m1<-m2 ,  att<-m0<-m1<-m2 */ // Ok
	/*                                                   m0<-m3<-m4         m0<-m3<-m4 */
	if (is_arg_num_test(arg,"6_8"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_6_8_mod_linked,
		                            mod_table_6_8_0, sizeof(mod_table_6_8_0)/sizeof(fluid_mod_t),
		                            mod_table_6_8_1, sizeof(mod_table_6_8_1)/sizeof(fluid_mod_t)));
	}

	/*------ invalid linked modulators tests -------------------------------------*/
	/* test 7: invalid linked modulators (invalid destination): mod0->mod2(src1 unlinked) */ // Ok
	if (is_arg_num_test(arg,"7"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_7_mod_linked,
		                            mod_table_7, sizeof(mod_table_7)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 8: invalid linked modulators: mod0->mod2(amount=0) */ // Ok
	if (is_arg_num_test(arg,"8"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_8_mod_linked,
		                            mod_table_8, sizeof(mod_table_8)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 9: invalid linked modulators: mod0->mod3(invalid index) */ // Ok
	if (is_arg_num_test(arg,"9"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_9_mod_linked,
		                            mod_table_9, sizeof(mod_table_9)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 10: linked modulators isolated: src1 linked->mod0->mod2 */ // Ok
	if (is_arg_num_test(arg,"10"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_10_mod_linked,
		                            mod_table_10, sizeof(mod_table_10)/sizeof(fluid_mod_t),NULL,0));
	}

	/* test 11: linked modulators circular: src2->mod3->mod0->mod2->mod0 */ //Ok
	if (is_arg_num_test(arg,"11"))
	{
		TEST_SUCCESS(test_fluid_zone_check_mod(test_11_mod_linked,
		                            mod_table_11, sizeof(mod_table_11)/sizeof(fluid_mod_t),NULL,0));
	}
	return FLUID_OK;
}

/*
 1) Test the function fluid_zone_check_mod(). The modulators
    to check must be in table mod_table0.
 @name_test: name of the test
 @param mod_table0, pointer on modulator table.
 @param count_mod0, count of modulators in mod_table0.

 2) Test the function fluid_linked_mod_test_identity() (if mod_table1
    is not null). 
	The test does test identity between the first complex modulator
	found in mod_table0 and the first complex modulator found in mod_table1.

  Note: These tests display results on the console and are intended to be
  interpreted by a human reading the result.
  Unlike tests done in test_modulator_link.c, these tests aren't intended to
  do automatic tests using TEST_ASSERT.

 @param mod_table1, pointer on modulator table or NULL.
 @param count_mod1, count of modulators in mod_table1.
 @return FLUID_OK or FLUID_FAILED
*/
static int
test_fluid_zone_check_mod(char * name_test,
                          fluid_mod_t mod_table0[],int count_mod0,
                          fluid_mod_t mod_table1[],int count_mod1
					 )
{
	fluid_mod_t *list_mod0 = NULL;
	fluid_mod_t *linked_mod0 =NULL;
	fluid_mod_t *list_mod1 = NULL;
	fluid_mod_t *linked_mod1 =NULL;

	/* Do nothing if mod_table 0 is empty */
	if((mod_table0 == NULL) || (count_mod0 == 0))
	{
	    FLUID_LOG(FLUID_INFO, "mod table 0: empty");
		return FLUID_OK;
	}

	/* Put mod_table0 in list list_mod0 */
	list_mod0 = fluid_build_list(mod_table0, count_mod0);
	if(list_mod0 == NULL)
	{
		return FLUID_FAILED;
	}

	/* print list_mod */
	FLUID_LOG(FLUID_INFO, "===================================================");
	FLUID_LOG(FLUID_INFO, "***** Start %s ****", name_test);
	FLUID_LOG(FLUID_INFO, "Start test of fluid_zone_check_mod() --------------");
	FLUID_LOG(FLUID_INFO, "- lists list_mod0, linked_mod0 before calling fluid_zone_check_mod():");
	print_lists(list_mod0, linked_mod0);

	FLUID_LOG(FLUID_INFO, "------------------------------------------");
	FLUID_LOG(FLUID_INFO, "- calling fluid_zone_check_mod()");
	if(fluid_zone_check_mod("list_mod_test", &list_mod0, &linked_mod0) == FLUID_FAILED)
	{
		FLUID_LOG(FLUID_ERR, "fluid_zone_check_mod: failed");
	}

	FLUID_LOG(FLUID_INFO, "------------------------------------------");
	FLUID_LOG(FLUID_INFO, "- lists list_mod0, linked_mod0  after calling fluid_zone_check_mod():");
	print_lists(list_mod0, linked_mod0);
	FLUID_LOG(FLUID_INFO, "End test of fluid_zone_check_mod() ----------------");

	//--------------------------------------------------------------------------
	/* Check if fluid_linked_mod_test_identity() test is requested ? */
	FLUID_LOG(FLUID_INFO, "---------------------------------------------------");
	FLUID_LOG(FLUID_INFO, "Test fluid_linked_mod_test_identity() ? -----------");
	if((mod_table1) && (count_mod1))
	{
		list_mod1 = fluid_build_list(mod_table1, count_mod1);
		if(list_mod1 == NULL)
		{
			return FLUID_FAILED;
		}

		FLUID_LOG(FLUID_INFO, "Start test fluid_linked_mod_test_identity() -------");
		/* print list_mod */
		FLUID_LOG(FLUID_INFO, "- lists list_mod1, linked_mod1 before calling fluid_zone_check_mod():");
		print_lists(list_mod1, linked_mod1);

		FLUID_LOG(FLUID_INFO, "------------------------------------------");
		FLUID_LOG(FLUID_INFO, "- calling fluid_zone_check_mod()");
		if(fluid_zone_check_mod("list_mod_test", &list_mod1, &linked_mod1) == FLUID_FAILED)
		{
			FLUID_LOG(FLUID_ERR, "fluid_zone_check_mod: failed");
		}

		FLUID_LOG(FLUID_INFO, "------------------------------------------");
		FLUID_LOG(FLUID_INFO, "- lists list_mod1, linked_mod1  after calling fluid_zone_check_mod():");
		print_lists(list_mod1, linked_mod1);


		/* The test does test identity between the first complex modulator
		in linked_mod0 and the first complex modulator in linked_mod1. */
		fluid_linked_mod_dump_test_identity(linked_mod0,linked_mod1);
		FLUID_LOG(FLUID_INFO, "End test fluid_linked_mod_test_identity() ---------");
	}
	else
	{
	    FLUID_LOG(FLUID_INFO, "- no, mod_table 1 is empty, no test of fluid_linked_mod_test_identity()");
	}


	FLUID_LOG(FLUID_INFO, "***** End %s ****", name_test);
	FLUID_LOG(FLUID_INFO, "===================================================");

	delete_fluid_list_mod(list_mod0);
	delete_fluid_list_mod(linked_mod0);
	delete_fluid_list_mod(list_mod1);
	delete_fluid_list_mod(linked_mod1);
	return FLUID_OK;
}

/*
 A convenience function that builds a list of modulators from a modulator table
 mod_table.
*/
static fluid_mod_t * fluid_build_list(fluid_mod_t mod_table[], int count_mod)
{
	int i;
	fluid_mod_t * prev;
	fluid_mod_t *list_mod = NULL;
	/* build list_mod containing test modulators from mod_table */
	for(i = 0; i < count_mod; i++)
	{
		/* Make a copy of this modulator */
		fluid_mod_t * mod = new_fluid_mod();
		if(mod == NULL)
		{
			FLUID_LOG(FLUID_ERR, "Out of memory");
			delete_fluid_list_mod(list_mod);

			return NULL;
		}
		fluid_mod_clone(mod, &mod_table[i]);
		mod->next = NULL;
		/* add to list_mode */
		if(i == 0)
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

/*----Printing functions to the console --------------------------------------*/
/*
 Print identity information to the console. Print identity between the first
 complex modulator in mod0 list and the first complex modulator in mod1 list.
*/
static void fluid_linked_mod_dump_test_identity(fluid_mod_t *mod0, fluid_mod_t *mod1)
{    
	/* first complex modulator */
	fluid_mod_t *cm0; /* first modulator of cm0 */
	int cm0_count;    /* count modulators of cm0 */
    
	/* second complex modulator */
	fluid_mod_t *cm1; /* first modulator of cm1 */
	int cm1_count = 0;    /* count modulators of cm1 */

	/* First complex modulator : cm0_idx, cm0, cm0_count*/
	cm0 = mod0;
	cm0_count = fluid_get_num_mod(cm0);
	if(cm0_count)
	{

		/* 2nd complex modulator : cm1_idx, cm1, cm1_count*/
		cm1 = mod1;
		cm1_count = fluid_get_num_mod(cm1);
	}

	FLUID_LOG(FLUID_INFO, "------------------------------------------");
	FLUID_LOG(FLUID_INFO, "- test complex modulators identity:");
	if(cm0_count)
	{
		/* complex modulator cm0 present */
		FLUID_LOG(FLUID_INFO, "-- complex modulator cm0:");
		fluid_dump_linked_mod(cm0, 0, 0);
		if(cm1_count)
		{
			int r;
			/* complex modulator cm1 present */
			FLUID_LOG(FLUID_INFO, "-- complex modulator cm1:");
			fluid_dump_linked_mod(cm1, 0, 0);

			/* Calling fluid_linked_mod_test_identity() */
			r = fluid_linked_mod_test_identity(cm0, 0, cm1,
                                               FLUID_LINKED_MOD_TEST_ONLY);

			/* display identity result */
			if (r)
			{
			    FLUID_LOG(FLUID_INFO, "-- test identity: cm0 == cm1 ?, yes ");
			}
			else
			{
			    FLUID_LOG(FLUID_INFO, "-- test identity: cm0 == cm1 ?, no ");
			}
			if(r)
			{
				/* add amount to cm0 */
				fluid_linked_mod_test_identity(cm0, 0, cm1, FLUID_LINKED_MOD_TEST_ADD);
				FLUID_LOG(FLUID_INFO, "-- complex modulator added cm1 amount to cm0 amount");
				fluid_dump_linked_mod(cm0, 0, 0);

				/* overwrite cm0 amount by cm1 amount  */
				fluid_linked_mod_test_identity(cm0, 0, cm1, FLUID_LINKED_MOD_TEST_OVERWRITE);
				FLUID_LOG(FLUID_INFO, "-- complex modulator overwrite cm0 amount by cm1 amount");
				fluid_dump_linked_mod(cm0, 0, 0);
			}
		}
		else
		{
			/* complex modulator cm1 absent */
			FLUID_LOG(FLUID_INFO, "-- No complex modulator cm1, cannot test identity");
		}
	}
	else
	{
		FLUID_LOG(FLUID_INFO, "-- No complex modulators cm0, cm1, cannot test identity");
	}
}


/*
 print two list of modulators:
 - list_mod is the original list  that contains any modulator linked or not linked.
 - linked_mod is the list that contains complex (linked) modulators only.
*/
static void print_lists(fluid_mod_t *list_mod, fluid_mod_t *linked_mod)
{

	print_list_mod("list_mod",list_mod);
	
	print_list_linked_mod(NULL,"linked_mod",linked_mod);
}

/* -----------------------------------------*/
/* dump for linked_mod : complex mod by complex mod*/
static void fluid_dump_list_linked_mod(fluid_mod_t *mod)
{
	int count = 0;
	int mod_idx = 0;
	while(mod)
	{
		fluid_dump_linked_mod(mod, mod_idx,  count);
		count+=fluid_get_num_mod(mod);
		mod_idx++;
		mod = fluid_get_next_mod(mod);
	}
}

/* -----------------------------------------*/
/* Print a linked list containing complex modulators,
   complex mod by complex mod.
 @header, header
 @name_list, name of list concatened with header
 @mod list, list of complex modulators
*/
static void print_list_linked_mod(char *header, char *name_list, fluid_mod_t *mod)
{
	if(header)
	{
		FLUID_LOG(FLUID_INFO, "------------------------------------------");
		FLUID_LOG(FLUID_INFO, header, name_list);

	}
	FLUID_LOG(FLUID_INFO, "------------------------------------------");
	if(mod)
	{
		FLUID_LOG(FLUID_INFO, "-- list \"%s\":", name_list);
		fluid_dump_list_linked_mod(mod);
	}
	else FLUID_LOG(FLUID_INFO, "-- list \"%s\" empty.", name_list);
}

/* -----------------------------------------*/
/* dump for any list mod by mod
*/
static void fluid_dump_list_mod(fluid_mod_t *mod)
{
	int count = 0;
	while(mod)
	{
		printf("mod%02d ",count);
		fluid_dump_modulator(mod);
		mod = mod->next;
		count++;
	}
}

/*
 Print modulator in list, mod by mod.
 @name_list, name of list printed as header
 @mod, list of modulators
*/
static void print_list_mod(char *name_list, fluid_mod_t *mod)
{
	FLUID_LOG(FLUID_INFO, "------------------------------------------");
	if(mod)
	{
		FLUID_LOG(FLUID_INFO, "-- list \"%s\":", name_list);
		fluid_dump_list_mod(mod);
	}
	else FLUID_LOG(FLUID_INFO, "-- list \"%s\" empty.", name_list);
}
