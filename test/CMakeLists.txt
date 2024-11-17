
ENABLE_TESTING()
include ( FluidUnitTest )

# first define the test target, used by the macros below
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure)


## add unit tests here ##
ADD_FLUID_TEST(test_synth_reset_cc)
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)
ADD_FLUID_TEST(test_sample_rate_change)
ADD_FLUID_TEST(test_preset_sample_loading)
ADD_FLUID_TEST(test_preset_pinning)
ADD_FLUID_TEST(test_bug_635)
ADD_FLUID_TEST(test_settings_unregister_callback)
ADD_FLUID_TEST(test_pointer_alignment)
ADD_FLUID_TEST(test_seqbind_unregister)
ADD_FLUID_TEST(test_synth_chorus_reverb)
ADD_FLUID_TEST(test_snprintf)
ADD_FLUID_TEST(test_synth_process)
ADD_FLUID_TEST(test_ct2hz)
ADD_FLUID_TEST(test_sample_validate)
ADD_FLUID_TEST(test_sfont_unloading)
ADD_FLUID_TEST(test_sfont_zone)
ADD_FLUID_TEST(test_seq_event_queue_sort)
ADD_FLUID_TEST(test_seq_scale)
ADD_FLUID_TEST(test_seq_evt_order)
ADD_FLUID_TEST(test_seq_event_queue_remove)
ADD_FLUID_TEST(test_jack_obtaining_synth)
ADD_FLUID_TEST(test_utf8_open)

if( LIBSNDFILE_SUPPORT )
    ADD_FLUID_TEST(test_fast_render)
endif()

ADD_FLUID_TEST_UTIL(dump_sfont)

ADD_FLUID_SF_DUMP_TEST(VintageDreamsWaves-v2.sf2)

if ( LIBSNDFILE_HASVORBIS )
    ADD_FLUID_TEST(test_sf3_sfont_loading)
    ADD_FLUID_SF_DUMP_TEST(VintageDreamsWaves-v2.sf3)
endif ( LIBSNDFILE_HASVORBIS )


# Prepare the manual test suite down here
if(NOT DEFINED GENERAL_USER_GS2)
    add_custom_target(check_manual
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red "In order to run the manual test suite, you need to set variable GENERAL_USER_GS2 to the path of the soundfont.")
else()

    set(IIR_FILTER_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/iir_filter")
    set(AWE32_NRPN_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/awe32_nrpn")

    if(LIBSNDFILE_SUPPORT)
      set(FEXT "wav")
    else()
      set(FEXT "raw")
    endif()

    # Add an empty pseudo target
    add_custom_target(check_manual)

    add_custom_target(create_iir_dir
        COMMAND ${CMAKE_COMMAND} -E make_directory ${IIR_FILTER_RENDER_DIR} ${AWE32_NRPN_RENDER_DIR}
        VERBATIM)

    add_custom_target(render1415
        COMMAND fluidsynth -R 0 -C 0 -g 1 -F ${IIR_FILTER_RENDER_DIR}/1415_the-nervous-filter.${FEXT} "The Nervous Filter trimmed.mid" ${GENERAL_USER_GS2}
        COMMENT "Rendering testfile of issue 1415"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1415_the-nervous-filter/
        VERBATIM
    )

    add_custom_target(render1417
        COMMAND fluidsynth -R 0 -C 0 -g 1 -F ${IIR_FILTER_RENDER_DIR}/1417_filter-envelope-noise.${FEXT} filter-envelope-noise.mid ${GENERAL_USER_GS2}
        COMMENT "Rendering testfile of issue 1417"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1417_filter-envelope-noise/
        VERBATIM
    )

    add_custom_target(render1424
        COMMAND fluidsynth -R 0 -C 0 -g 1 -F ${IIR_FILTER_RENDER_DIR}/1424_clicks-on-ModEnv-FilterFc-change.${FEXT} 1424.mid echo_drops.sf2
        COMMENT "Rendering testfile of issue 1424"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1424_clicks-on-ModEnv-FilterFc-change/
        VERBATIM
    )

    add_custom_target(render1427
        COMMAND fluidsynth -R 0 -C 0 -g 5 -F ${IIR_FILTER_RENDER_DIR}/1427_high-Q-note-cutoff.${FEXT} high_Q_note_cutoff_test.mid high_Q_note_cutoff_test.sf2
        COMMENT "Rendering testfile of issue 1427"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1427_high-Q-note-cutoff/
        VERBATIM
    )


    add_custom_target(renderAltitude
        COMMAND fluidsynth -R 1 -C 1 -g 0.5 -F ${AWE32_NRPN_RENDER_DIR}/Altitude.${FEXT} Altitude.mid Altitude.sf2
        COMMENT "Rendering AWE32 NRPN test file Altitude.mid"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Altitude/
        VERBATIM
    )

    add_custom_target(renderFilter
        COMMAND fluidsynth -R 0 -C 0 -F "${AWE32_NRPN_RENDER_DIR}/The Nervous Filter.${FEXT}" "The Nervous Filter.mid" ${GENERAL_USER_GS2}
        COMMENT "Rendering AWE32 NRPN test file The Nervous Filter.mid"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Filter/
        VERBATIM
    )

    add_custom_target(renderUplift
        COMMAND fluidsynth -R 1 -C 1 -F ${AWE32_NRPN_RENDER_DIR}/Uplift.${FEXT} Uplift.mid Uplift.sf2
        COMMENT "Rendering AWE32 NRPN test file Uplift.mid"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Uplift/
        VERBATIM
    )

    # Add a dependency so that rendering targets depends on check_manual
    add_dependencies(check_manual render1415)
    add_dependencies(check_manual render1417)
    add_dependencies(check_manual render1424)
    add_dependencies(check_manual render1427)
    add_dependencies(check_manual renderAltitude)
    add_dependencies(check_manual renderFilter)
    add_dependencies(check_manual renderUplift)

endif()
