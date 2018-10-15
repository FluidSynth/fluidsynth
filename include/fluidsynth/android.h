/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#ifndef _FLUIDSYNTH_ANDROID_H
#define _FLUIDSYNTH_ANDROID_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file android.h
 * @brief Functions for Android asset based soundfont loader.
 * @defgroup Android Functions for android asset based soundfont loader
 *
 * Defines functions for Android asset based soundfont loader. Use new_fluid_android_asset_sfloader() to create a new sfloader. It is just a default sfloader with the callbacks which are Android Assets API.
 */

FLUIDSYNTH_API fluid_sfloader_t *new_fluid_android_asset_sfloader(JNIEnv *env, fluid_settings_t *settings);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_ANDROID_H */
