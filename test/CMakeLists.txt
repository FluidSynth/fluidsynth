
ENABLE_TESTING()
include ( FluidUnitTest )

# first define the test target, used by the macros below
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure)


## add unit tests here ##
ADD_FLUID_TEST(test_synth_reset_cc)
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)
#ADD_FLUID_TEST(test_sample_rate_change)
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
    set ( GENERAL_USER_GS2 "${CMAKE_CURRENT_SOURCE_DIR}/manual/sf2/GeneralUser-GS/GeneralUser-GS.sf2" )
endif()

set(IIR_FILTER_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/iir_filter")
set(AWE32_NRPN_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/awe32_nrpn")
set(SFSPEC_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/SoundFont-Spec-Test")
set(GUGDEMO_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/sf2/GeneralUser-GS")
set(PORTAMENTO_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/portamento")
set(REVERB_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/reverb")
set(EXCL_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/exclusive_class")
set(DSPINTERP_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/dsp_interp")
set(DYNSAM_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/dynamic-sample-loading")
set(BANKSELECT_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/midi-bank-select")
set(STACKEDSF_RENDER_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual/stacked_sf2")

if(LIBSNDFILE_SUPPORT)
    set(FEXT "wav")
else()
    set(FEXT "raw")
endif()

# Add an empty pseudo target
add_custom_target(check_manual)

add_custom_target(create_iir_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IIR_FILTER_RENDER_DIR} ${AWE32_NRPN_RENDER_DIR} ${SFSPEC_RENDER_DIR} ${PORTAMENTO_RENDER_DIR} ${REVERB_RENDER_DIR} ${EXCL_RENDER_DIR} ${DSPINTERP_RENDER_DIR} ${DYNSAM_RENDER_DIR} ${STACKEDSF_RENDER_DIR} ${GUGDEMO_RENDER_DIR} ${BANKSELECT_RENDER_DIR}
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

add_custom_target(renderRealtimeIIR
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -niq "high poly audio breakup.mid" "high poly preset test.sf2" # -F "${IIR_FILTER_RENDER_DIR}/1481 high poly audio breakup.${FEXT}"
    COMMENT "Realtime-Rendering testfile of issue 1481"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1481_realtime-playback/
    VERBATIM
)

add_custom_target(render1481
    COMMAND fluidsynth -R 0 -C 0 -g 1.8 -F ${IIR_FILTER_RENDER_DIR}/1481_cutoff-resolution.${FEXT} "filter resolution test.mid" "filter resolution test.sf2"
    COMMENT "Rendering testfile of issue 1481"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/iir_filter/1481_cutoff-resolution/
    VERBATIM
)

add_custom_target(renderAltitude
    COMMAND fluidsynth -R 1 -C 1 -f fluidsynth.cfg -o synth.default-soundfont="../../sf2/synthgs-sf2_04-compat.sf2" -F ${AWE32_NRPN_RENDER_DIR}/Altitude.${FEXT} Altitude.mid 
    COMMENT "Rendering AWE32 NRPN test file Altitude.mid"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Altitude/
    VERBATIM
)

add_custom_target(renderFilter
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -F "${AWE32_NRPN_RENDER_DIR}/The Nervous Filter GenUserGS2.sf2.${FEXT}" "The Nervous Filter.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -F "${AWE32_NRPN_RENDER_DIR}/The Nervous Filter 1mgm.sf2.${FEXT}" "The Nervous Filter.mid" "1mgm.sf2"
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -F "${AWE32_NRPN_RENDER_DIR}/The Nervous Filter GM_Roland.sf2.${FEXT}" "The Nervous Filter.mid" "GM_Roland.sf2"
    COMMENT "Rendering AWE32 NRPN test file The Nervous Filter.mid"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Filter/
    VERBATIM
)

add_custom_target(renderUplift
    COMMAND fluidsynth -R 1 -C 0 -g 0.7 -F ${AWE32_NRPN_RENDER_DIR}/Uplift-fixed.${FEXT} Uplift-fixed.mid Uplift.sf2
    COMMENT "Rendering AWE32 NRPN test file Uplift.mid"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/awe32_nrpn/Uplift/
    VERBATIM
)

add_custom_target(render1TOWOW
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -F "${PORTAMENTO_RENDER_DIR}/1TOWOW_reduced_to_A3_C4.${FEXT}" "1TOWOW_reduced_to_A3_C4.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 0.5 -F "${PORTAMENTO_RENDER_DIR}/1TOWOW.${FEXT}" "1TOWOW.MID" ${GENERAL_USER_GS2}
    COMMENT "Rendering Portamento tests"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/portamento/
    VERBATIM
)

add_custom_target(renderDescent8
    COMMAND fluidsynth -R 1 -C 1 -g 0.5 -F ${PORTAMENTO_RENDER_DIR}/Game08.${FEXT} Game08.mid ${GENERAL_USER_GS2}
    COMMENT "Rendering Descent Game 8"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/portamento/
    VERBATIM
)

add_custom_target(renderSfSpecTest
    COMMAND fluidsynth -R 1 -C 1 -g 0.5 -F ${SFSPEC_RENDER_DIR}/sf_spec_test.${FEXT} sf_spec_test.mid sf_spec_test.sf2
    COMMENT "Rendering Christian Collins' SF2 spec test"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/SoundFont-Spec-Test/
    VERBATIM
)

add_custom_target(render1455
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=0.0 -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_000.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=0.5 -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_000.5.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=1.0 -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_001.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=5.0 -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_005.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=10  -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_010.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=50  -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_050.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=100 -F ${REVERB_RENDER_DIR}/mrbumpys_rev_width_test_100.0.${FEXT} mrbumpys_rev_width_test.mid ${GENERAL_USER_GS2}
    COMMENT "Rendering Christian Collins' reverb test issue 1455"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/reverb/
    VERBATIM
)

add_custom_target(render1496
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=0.0 -F ${REVERB_RENDER_DIR}/1496.${FEXT} mrbumpys_rev_width_test.mid 1496.sf2
    COMMENT "Rendering reverb test issue 1496"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/reverb/
    VERBATIM
)

set(DK64SF2 "../sf2/ANMP-data/soundfonts/N64/DK64.sf2")
add_custom_target(renderDK64JJU
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=0.0 -F ${REVERB_RENDER_DIR}/DK64_sparse04_000.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=0.5 -F ${REVERB_RENDER_DIR}/DK64_sparse04_000.5.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=1.0 -F ${REVERB_RENDER_DIR}/DK64_sparse04_001.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=5.0 -F ${REVERB_RENDER_DIR}/DK64_sparse04_005.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=10  -F ${REVERB_RENDER_DIR}/DK64_sparse04_010.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=50  -F ${REVERB_RENDER_DIR}/DK64_sparse04_050.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMAND fluidsynth -R 1 -C 0 -g 0.5 -o synth.reverb.width=100 -F ${REVERB_RENDER_DIR}/DK64_sparse04_100.0.${FEXT} DK64_sparse04.mid ${DK64SF2}
    COMMENT "Praise Grant Kirkhope!"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/reverb/
    VERBATIM
)

add_custom_target(renderExcl
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -F "${EXCL_RENDER_DIR}/exclusive class cutoff speed.${FEXT}" "exclusive class cutoff speed.mid" "exclusive class cutoff speed.sf2"
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -o synth.note-cut=0 -F "${EXCL_RENDER_DIR}/MIDInotecut0.${FEXT}" "MIDInotecut.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -o synth.note-cut=1 -F "${EXCL_RENDER_DIR}/MIDInotecut1.${FEXT}" "MIDInotecut.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -o synth.note-cut=2 -F "${EXCL_RENDER_DIR}/MIDInotecut2.${FEXT}" "MIDInotecut.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -F "${EXCL_RENDER_DIR}/percussion note cutoff.${FEXT}" "percussion note cutoff.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 1.4 -F "${EXCL_RENDER_DIR}/percussion-test.${FEXT}" "percussion-test.mid" ${GENERAL_USER_GS2}
    COMMENT "Rendering exclusive class cutoff tests"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/exclusive_class/
    VERBATIM
)

add_custom_target(rendere1m1
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=0 -F "${EXCL_RENDER_DIR}/e1m1_GeneralUser2_notecut0.${FEXT}" "e1m1.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=0 -F "${EXCL_RENDER_DIR}/e1m1_gzdoom_notecut0.${FEXT}" "e1m1.mid" "../sf2/gzdoom.sf2"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=1 -F "${EXCL_RENDER_DIR}/e1m1_GeneralUser2_notecut1.${FEXT}" "e1m1.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=1 -F "${EXCL_RENDER_DIR}/e1m1_gzdoom_notecut1.${FEXT}" "e1m1.mid" "../sf2/gzdoom.sf2"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=2 -F "${EXCL_RENDER_DIR}/e1m1_GeneralUser2_notecut2.${FEXT}" "e1m1.mid" ${GENERAL_USER_GS2}
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.note-cut=2 -F "${EXCL_RENDER_DIR}/e1m1_gzdoom_notecut2.${FEXT}" "e1m1.mid" "../sf2/gzdoom.sf2"
    COMMENT "Rendering Doom E1M1 with note-cut 0,1,2"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/exclusive_class/
    VERBATIM
)

add_custom_target(renderdspInterp
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${DSPINTERP_RENDER_DIR}/sample interpolation test.${FEXT}" "sample interpolation test.mid" "sample interpolation test.sf2"
    COMMENT "Rendering Interpolation examples"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/dsp_interp/
    VERBATIM
)

add_custom_target(render1484
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.dynamic-sample-loading=1 -F "${DYNSAM_RENDER_DIR}/DrTebz_dynamic-sample-loading_on.${FEXT}" "dynamic-sample-loading_test.mid" "DrTebz.sf2"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.dynamic-sample-loading=0 -F "${DYNSAM_RENDER_DIR}/DrTebz_dynamic-sample-loading_off.${FEXT}" "dynamic-sample-loading_test.mid" "DrTebz.sf2"
    COMMENT "Rendering dynamic sample loading test"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/dynamic-sample-loading/
    VERBATIM
)
add_custom_target(render1505
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.dynamic-sample-loading=1 -F "${DYNSAM_RENDER_DIR}/ran_dynamic-sample-loading_on.${FEXT}" ${GENERAL_USER_GS2} "ran.dls" "ran.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -o synth.dynamic-sample-loading=0 -F "${DYNSAM_RENDER_DIR}/ran_dynamic-sample-loading_off.${FEXT}" ${GENERAL_USER_GS2} "ran.dls" "ran.mid"
    COMMENT "Rendering dynamic sample loading test"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/dynamic-sample-loading/
    VERBATIM
)

add_custom_target(renderGeneralUserDemo
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Bond.${FEXT}" ${GENERAL_USER_GS2} "Bond.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Breakout.${FEXT}" ${GENERAL_USER_GS2} "Breakout.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Dance.${FEXT}" ${GENERAL_USER_GS2} "Dance.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Earth Day - by Richard Audd (fixed).${FEXT}" ${GENERAL_USER_GS2} "Earth Day - by Richard Audd (fixed).mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/J-cycle.${FEXT}" ${GENERAL_USER_GS2} "J-cycle.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Jump!.${FEXT}" ${GENERAL_USER_GS2} "Jump!.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Santa Claus is Comin' to Town.${FEXT}" ${GENERAL_USER_GS2} "Santa Claus is Comin' to Town.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/The HYBRID Collage (v2.0) - by S. Christian Collins.${FEXT}" ${GENERAL_USER_GS2} "The HYBRID Collage (v2.0) - by S. Christian Collins.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${GUGDEMO_RENDER_DIR}/Umi no Mieru Machi.${FEXT}" ${GENERAL_USER_GS2} "Umi no Mieru Machi.mid"
    COMMENT "Rendering General User MIDI Demos"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/manual/sf2/GeneralUser-GS/demo MIDIs"
    VERBATIM
)

add_custom_target(renderBankSelect
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${BANKSELECT_RENDER_DIR}/Dance_GM.${FEXT}" ${GENERAL_USER_GS2} "Dance_GM.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${BANKSELECT_RENDER_DIR}/Dance_GM2.${FEXT}" ${GENERAL_USER_GS2} "Dance_GM2.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${BANKSELECT_RENDER_DIR}/Dance_GS.${FEXT}" ${GENERAL_USER_GS2} "Dance_GS.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${BANKSELECT_RENDER_DIR}/Dance_original.${FEXT}" ${GENERAL_USER_GS2} "Dance_original.mid"
    COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${BANKSELECT_RENDER_DIR}/Dance_XG.${FEXT}" ${GENERAL_USER_GS2} "Dance_XG.mid"
    COMMENT "Rendering MIDI Bank selection tests"
    DEPENDS fluidsynth create_iir_dir
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/manual/midi-bank-select"
    VERBATIM
)

if ( LIBINSTPATCH_SUPPORT )
    add_custom_target(renderStreetwise
        COMMAND fluidsynth -R 0 -C 0 -g 0.6 -F "${STACKEDSF_RENDER_DIR}/Streetwise.${FEXT}" "Streetwise.mid" ${GENERAL_USER_GS2} "Streetwise.dls"
        COMMENT "Rendering testcase for stacked soundfonts"
        DEPENDS fluidsynth create_iir_dir
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/manual/stacked_sf2/Streetwise/
        VERBATIM
    )
else ( LIBINSTPATCH_SUPPORT )
    add_custom_target(renderStreetwise
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red "Cannot render Streetwise, no DLS support!"
        COMMENT "Rendering testcase for stacked soundfonts"
    )
endif (LIBINSTPATCH_SUPPORT )

# Add a dependency so that rendering targets depends on check_manual
add_dependencies(check_manual render1415)
add_dependencies(check_manual render1417)
add_dependencies(check_manual render1424)
add_dependencies(check_manual render1427)
add_dependencies(check_manual renderAltitude)
add_dependencies(check_manual renderFilter)
add_dependencies(check_manual renderUplift)
add_dependencies(check_manual render1TOWOW)
add_dependencies(check_manual renderDescent8)
add_dependencies(check_manual renderSfSpecTest)
add_dependencies(check_manual render1455)
add_dependencies(check_manual renderDK64JJU)
add_dependencies(check_manual renderExcl)
add_dependencies(check_manual rendere1m1)
add_dependencies(check_manual renderdspInterp)
add_dependencies(check_manual render1484)
add_dependencies(check_manual renderRealtimeIIR)
add_dependencies(check_manual render1481)
add_dependencies(check_manual renderStreetwise)

