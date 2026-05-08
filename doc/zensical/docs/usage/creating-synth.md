# Creating the synthesizer

To create the synthesizer, you pass it the settings object, as in the
following example: 

```c
#include <fluidsynth.h>

int main(int argc, char** argv) 
{
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    /* Do useful things here */

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return 0;
}
```

For a full list of available **synthesizer settings**, please
refer to the `synth` documentation.
