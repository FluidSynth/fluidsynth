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

#include "fluid_androidasset.h"

#if defined(ANDROID) || defined(__DOXYGEN__)

#include "fluidsynth.h"
#include "fluid_sys.h"
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

JavaVM* android_jvm;
jobject android_asset_manager;

fluid_sfloader_t *new_fluid_android_asset_sfloader(JNIEnv* env, fluid_settings_t *settings)
{
    fluid_sfloader_t *loader;
    
    fluid_return_val_if_fail(env != NULL, NULL);
    fluid_return_val_if_fail(settings != NULL, NULL);
    
    /* seriously? */
    (*env)->GetJavaVM (env, &android_jvm);
    
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

void *asset_open(const char *path)
{
	JNIEnv *env;
	AAssetManager *am;

    fluid_return_val_if_fail(android_jvm != NULL, NULL);    
    fluid_return_val_if_fail(android_asset_manager != NULL, NULL);
    
    (*android_jvm)->AttachCurrentThread (android_jvm, &env, NULL);
	am = AAssetManager_fromJava (env, android_asset_manager);
	return AAssetManager_open (am, path, AASSET_MODE_RANDOM);
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
