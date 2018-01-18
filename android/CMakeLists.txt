cmake_minimum_required(VERSION 3.4.1)

set(src_DIR ../src)

set (fluidsynth_sources
        ${src_DIR}/midi/fluid_midi.c
        ${src_DIR}/midi/fluid_midi_router.c
        ${src_DIR}/midi/fluid_seq.c
        ${src_DIR}/midi/fluid_seqbind.c
        ${src_DIR}/rvoice/fluid_adsr_env.c
        ${src_DIR}/rvoice/fluid_chorus.c
        ${src_DIR}/rvoice/fluid_iir_filter.c
        ${src_DIR}/rvoice/fluid_lfo.c
        ${src_DIR}/rvoice/fluid_rev.c
        ${src_DIR}/rvoice/fluid_rvoice.c
        ${src_DIR}/rvoice/fluid_rvoice_dsp.c
        ${src_DIR}/rvoice/fluid_rvoice_event.c
        ${src_DIR}/rvoice/fluid_rvoice_mixer.c
        ${src_DIR}/sfloader/fluid_defsfont.c
        ${src_DIR}/sfloader/fluid_ramsfont.c
        ${src_DIR}/synth/fluid_chan.c
        ${src_DIR}/synth/fluid_event.c
        ${src_DIR}/synth/fluid_gen.c
        ${src_DIR}/synth/fluid_mod.c
        ${src_DIR}/synth/fluid_synth.c
        ${src_DIR}/synth/fluid_tuning.c
        ${src_DIR}/synth/fluid_voice.c
        ${src_DIR}/utils/fluid_conv.c
        ${src_DIR}/utils/fluid_hash.c
        ${src_DIR}/utils/fluid_list.c
        ${src_DIR}/utils/fluid_ringbuffer.c
        ${src_DIR}/utils/fluid_settings.c
        ${src_DIR}/bindings/fluid_filerenderer.c
        ${src_DIR}/utils/fluid_sys.c
        src/bindings/fluid_cmd.c
        src/drivers/fluid_adriver.c
        src/drivers/fluid_mdriver.c
        )

add_library(fluidsynth SHARED ${fluidsynth_sources})

# Specify directories which the compiler should look for headers
target_include_directories(fluidsynth PRIVATE
                            . include
                            ${src_DIR}/../include
                            ${src_DIR}/synth
                            ${src_DIR}/midi
                            ${src_DIR}/rvoice
                            ${src_DIR}/sfloader
                            ${src_DIR}/utils
                            ${src_DIR}/bindings
                            ${src_DIR}/drivers
                            )

target_include_directories(fluidsynth INTERFACE
                            . include
                            ${src_DIR}/../include
                            )

target_compile_options(fluidsynth PRIVATE -std=gnu11
                            PRIVATE -Wall
                            PRIVATE -DHAVE_CONFIG_H
                            PRIVATE -Wno-unused-variable
                            PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config