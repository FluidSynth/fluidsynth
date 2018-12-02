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

#if defined(ANDROID) || defined(__DOXYGEN__)

#define FLUIDSYNTH_API 
#include <stdlib.h>
#include <jni.h>
#include "fluid_androidasset.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


AAssetManager *fluid_android_asset_manager;

fluid_sfloader_t* new_fluid_android_asset_sfloader(fluid_settings_t *settings, void *assetManager)
{
    fluid_sfloader_t *loader;
    
    if (settings == NULL)
		return NULL;
    
    if (!fluid_android_asset_manager)
		fluid_android_asset_manager = (AAssetManager*) assetManager;

    if (fluid_android_asset_manager == NULL)
		return NULL;
    
    loader = new_fluid_defsfloader(settings);
    if (loader == NULL)
		return NULL;
    
    fluid_sfloader_set_callbacks(loader,
                                 asset_open,
                                 asset_read,
                                 asset_seek,
                                 asset_tell,
                                 asset_close);

	return loader;
}

/* This is a compromised solution for JNAerator for that 1) it cannot handle jobject with JNIEnv as parameters, and that 2) the returned pointer can be converted in the same manner that JNAerated methods. (Most likely my JNA usage issue but no one has answer for it.) */
void Java_fluidsynth_androidextensions_NativeHandler_setAssetManagerContext(JNIEnv *env, jobject _this, jobject assetManager)
{
	if (assetManager == NULL)
		return;
	
    fluid_android_asset_manager = AAssetManager_fromJava (env, assetManager);
}

void *asset_open(const char *path)
{
    if (fluid_android_asset_manager == NULL)
		return NULL;
    
	return AAssetManager_open (fluid_android_asset_manager, path, AASSET_MODE_RANDOM);
}

int asset_close(void *handle)
{
	AAsset *asset;
	
	asset = (AAsset*) handle;
    AAsset_close (asset);
    return 0;
}

long asset_tell(void *handle)
{
	AAsset *asset;
	
	asset = (AAsset*) handle;
    return AAsset_getLength(asset) - AAsset_getRemainingLength(asset);
}

int asset_seek(void *handle, long offset, int origin)
{
	AAsset *asset;
	
	asset = (AAsset*) handle;
	return AAsset_seek (asset, (off_t) offset, origin);
}

int asset_read(void *buf, int count, void *handle)
{
	AAsset *asset;
	
	asset = (AAsset*) handle;
	return AAsset_read (asset, buf, (size_t) count);
}

#endif /* if defined(ANDROID) || defined(__DOXYGEN__) */
