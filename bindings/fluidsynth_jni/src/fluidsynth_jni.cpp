#include <sndfile.h>
#include "fluidsynth_jni.h"

const int fluid_jni_maxsamples = 1024;

static int _initialized = 0;
static fluid_settings_t* _settings = 0;
static fluid_synth_t* _synth = 0;
static fluid_audio_driver_t* _adriver = 0;
static fluid_midi_driver_t* _mdriver = 0;
static fluid_sfont_t* _sfloader = 0;
static fluid_sample_t* _sample[fluid_jni_maxsamples];



void fluid_jni_init()
{
  if (_initialized == 0) {
    _initialized++;
    for (int i = 0; i < fluid_jni_maxsamples; i++) {
      _sample[i] = 0;
    }
  }
}

int fluid_jni_new_synth()
{
  if (_synth != 0) {
    return -1;
  }

  fluid_jni_init();

  _settings = new_fluid_settings();
  if (_settings == 0) {
    goto error_recovery;
  }
  
  fluid_settings_setstr(_settings, "midi.driver", "midishare");

  _synth = new_fluid_synth(_settings);
  if (_synth == 0) {
    goto error_recovery;
  }
  
  _adriver = new_fluid_audio_driver(_settings, _synth);
  if (_adriver == 0) {
    goto error_recovery;
  }
  
  _mdriver = new_fluid_midi_driver(_settings, fluid_synth_handle_midi_event, _synth);
  if (_mdriver == 0) {
    goto error_recovery;
  }
  

  _sfloader = fluid_ramsfont_create_sfont();
  if (_sfloader == 0) {
    goto error_recovery;
  }
  fluid_ramsfont_set_name((fluid_ramsfont_t*) _sfloader->data, "Tada");
  
  fluid_synth_add_sfont(_synth, _sfloader);

  return 0;

 error_recovery:
  fluid_jni_delete_synth(0);
  return -1;
}


int fluid_jni_delete_synth(int num)
{
  if (_mdriver) {

    delete_fluid_midi_driver(_mdriver);
	
    _mdriver = 0;
  }
  if (_adriver) {

    delete_fluid_audio_driver(_adriver);

    _adriver = 0;
  }
  if (_synth) {
	
    delete_fluid_synth(_synth);

    _synth = 0;
  }
  if (_settings) {
    delete_fluid_settings(_settings);
    _settings = 0;
  }
  return 0;
}

int fluid_jni_sfload(const char*  filename)
{
  if (_synth == 0) {
    return -1;
  }
  fluid_synth_sfload(_synth, filename, 1);
  return 0;
}

int fluid_jni_add(int samplenum, int bank, int preset, int lokey, int hikey)
{
  if (_synth == 0) {
    return -1;
  }
  if (_sfloader == 0) {
    return -1;
  }
  fluid_sample_t* sample = fluid_jni_get_sample(samplenum);
  if (sample == 0) {
    return -2;
  }
  
  
  
  if (fluid_ramsfont_add_izone((fluid_ramsfont_t*) _sfloader->data, bank, 
			       preset, sample, lokey, hikey) != 0) {
    
  
    return -3;
  }
  
  
  
  fluid_synth_program_select(_synth, 0, _sfloader->id, bank, preset);
  return 0;
}

int fluid_jni_remove(int samplenum, int bank, int preset)
{
  if (_synth == 0) {
    return -1;
  }
  fluid_sample_t* sample = fluid_jni_get_sample(samplenum);
  if (sample == 0) {
    return -2;
  }
  if (fluid_ramsfont_remove_izone((fluid_ramsfont_t*) _sfloader->data, 
				  bank, preset, sample) != 0) {
    return -3;
  }
 

  return 0;
}

int fluid_jni_get_sample_num()
{
  for (int i = 0; i < fluid_jni_maxsamples; i++) {
    if (_sample[i] == 0) {
      return i;
    }
  }
  return -1;
}

int fluid_jni_new_sample(const char* filename, int rootkey)
{
  SF_INFO sfinfo;
  SNDFILE* sndfile = 0;
  fluid_sample_t* sample = 0;
  short *data = 0;
  sf_count_t count;
  int err;
  
  int num = fluid_jni_get_sample_num();
  if (num < 0) {
    return -1;
  }

  
  

  sndfile = sf_open(filename, SFM_READ, &sfinfo) ;
  if (sndfile == 0) {
    return -2;
  }
  
  //printf("fluid_jni_new_sample: channels=%i, srate=%i, frames=%i\n", 
//	 sfinfo.channels, sfinfo.samplerate, sfinfo.frames);
  
  
  if (sfinfo.channels != 1) {
    err = -3;
    goto error_recovery;
  }
  if (sfinfo.samplerate != 44100) {
    err = -4;
    goto error_recovery;
  }
  sample = new_fluid_ramsample();
  if (sample == 0) {
    err = -5;
    goto error_recovery;
  }
  data = new short[sfinfo.frames];
  if (data == 0) {
    err = -6;
    goto error_recovery;
  }

 // printf("fluid_jni_new_sample 2\n");
  
  count = sf_readf_short(sndfile, data, sfinfo.frames);
  if (count != sfinfo.frames) {
    err = -7;
    goto error_recovery;
  }

  //printf("fluid_jni_new_sample 3\n");
  
  if (fluid_sample_set_sound_data(sample, data,  sfinfo.frames, 1, rootkey) != 0) {
    err = -8;
    goto error_recovery;
  }

  //printf("fluid_jni_new_sample 4: sample=%p\n", sample);
  
  _sample[num] = sample;

  sf_close(sndfile);
  delete data;

  return num;
  
 error_recovery:
  
  if (sndfile) {
    sf_close(sndfile);
  }
  if (sample) {
    delete_fluid_ramsample(sample);
  }
  if (data) {
    delete data;
  }
  return err;
}

fluid_sample_t* fluid_jni_get_sample(int num)
{
  if ((num >= 0) 
      && (num < fluid_jni_maxsamples) 
      && (_sample[num] != 0)) {
    return _sample[num];
  }
  return 0;
}

int fluid_jni_delete_sample(int num)
{
  if ((num >= 0) 
      && (num < fluid_jni_maxsamples) 
      && (_sample[num] != 0)) {
    delete_fluid_ramsample(_sample[num]);
    _sample[num] = 0;
  }
  return 0;
}
