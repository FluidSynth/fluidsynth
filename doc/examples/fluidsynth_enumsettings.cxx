/* FluidSynth Enum Settings - An example of using fluidsynth in C++
 * This source uses C++11 features (nullptr, lambda expressions, ...)
 *
 * This code is in the public domain.
 *
 * To compile:
 *   g++ -o fluidsynth_enumsettings -lfluidsynth fluidsynth_enumsettings.cpp
 *
 * To run:
 *   fluidsynth_enumsettings
 *
 * [Pedro López-Cabanillas <plcl@users.sf.net>]
 */

#include <list>
#include <string>
#include <iostream>
#include <iomanip>
#include <fluidsynth.h>

int main(int argc, char**)
{
    fluid_settings_t* settings = nullptr;
    void* context = nullptr;

    std::cout << "C++ enum settings of FluidSynth v" << fluid_version_str() << std::endl;

    if (argc > 1) {
        std::cerr << "Usage: fluidsynth_enumsettings" << std::endl;
        return 1;
    }

    /* Create the settings object. This example uses the default values for the settings. */
    settings = new_fluid_settings();
    if (settings == NULL) {
        std::cerr << "Failed to create the settings" << std::endl;
        return 2;
    }

    std::cout << std::left << std::setw(35) << "Setting" << std::setw(16) << "Type" << std::setw(16) << "Value" << "Options" << std::endl;
    std::cout << std::left << std::setw(35) << "-------" << std::setw(16) << "----" << std::setw(16) << "-----" << "-------" << std::endl;

    context = settings;
    fluid_settings_foreach(settings, context, [](void *inner_context, const char *name, int type) {
        int ok = 0;
        double dValue{0.0};
        int iValue{0};
        char *psValue = nullptr;
        fluid_settings_t* inner_settings = (fluid_settings_t*) inner_context;
        std::cout << std::left << std::setw(35) << name;
        switch (type) {
        case FLUID_NO_TYPE:
            std::cout << std::setw(16) << "Undefined";
            break;
        case FLUID_NUM_TYPE:
            ok = fluid_settings_getnum(inner_settings, name, &dValue);
            if (ok == FLUID_OK) {
                std::cout << std::setw(16) << "Numeric" << std::setw(16) << dValue;
            }
            break;
        case FLUID_INT_TYPE:
            ok = fluid_settings_getint(inner_settings, name, &iValue);
            if (ok == FLUID_OK) {
                std::cout << std::setw(16) << "Integer" << std::setw(16) << iValue;
            }
            break;
        case FLUID_STR_TYPE:
            ok = fluid_settings_dupstr(inner_settings, name, &psValue);
            if (ok == FLUID_OK) {
                std::cout << std::setw(16) << "String" << std::setw(16) << psValue;
                fluid_free(psValue);
            }
            break;
        case FLUID_SET_TYPE:
            std::cout << std::setw(16) << "Set";
            break;
        }
        std::list<std::string> options;
        fluid_settings_foreach_option(inner_settings, name, &options, [](void *context2, const char *, const char *option2){
            std::list<std::string> *options_list = (std::list<std::string> *) context2;
            if (!options_list->empty()) {
                options_list->push_back(", ");
            }
            options_list->push_back(option2);
        });
        if (!options.empty()) {
            for(auto it=options.begin(); it != options.end(); ++it) {
                std::cout << *it;
            }
        }
        std::cout << std::endl;
    });
    std::cout << "done" << std::endl;

    delete_fluid_settings(settings);
    return 0;
}
