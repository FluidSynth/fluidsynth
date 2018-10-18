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

#include "fluid_androidasset.h"
#include "fluid_sys.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


AAssetManager *fluid_android_asset_manager;

fluid_sfloader_t* new_fluid_android_asset_sfloader(fluid_settings_t *settings, AAssetManager *assetManager)
{
    fluid_sfloader_t *loader;
    
    fluid_return_val_if_fail(settings != NULL, NULL);
    
    if (!fluid_android_asset_manager)
		fluid_android_asset_manager = assetManager;

    fluid_return_val_if_fail(fluid_android_asset_manager != NULL, NULL);
    
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
	AAssetManager *am;
	
	fluid_return_if_fail(assetManager != NULL);
	
    fluid_android_asset_manager = AAssetManager_fromJava (env, assetManager);
}

void *asset_open(const char *path)
{
    fluid_return_val_if_fail(fluid_android_asset_manager != NULL, NULL);
    
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
