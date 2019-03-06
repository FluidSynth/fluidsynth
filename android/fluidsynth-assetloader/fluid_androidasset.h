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


#ifndef _PRIV_FLUID_ANDROIDASSET_H
#define _PRIV_FLUID_ANDROIDASSET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fluidsynth/types.h>
#include <fluidsynth/sfont.h>

fluid_sfloader_t* new_fluid_android_asset_sfloader(fluid_settings_t *settings, void *assetManager);
void Java_fluidsynth_androidextensions_NativeHandler_setAssetManagerContext(JNIEnv *env, jobject _this, jobject assetManager);

void *asset_open(const char *path);
int asset_close(void *handle);
long asset_tell(void *handle);
int asset_seek(void *handle, long offset, int origin);
int asset_read(void *buf, int count, void *handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _PRIV_FLUID_ANDROIDASSET_H */
