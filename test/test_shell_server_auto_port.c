#include "test.h"
#include "fluidsynth.h"

#define TEST_SHELL_AUTO_PORT_START 9800
#define TEST_TCP_PORT_MAX          65535

int main(void)
{
#ifdef NETWORK_SUPPORT
    fluid_settings_t *settings1, *settings2;
    fluid_server_t *server1, *server2;
    int port1, port2;

    settings1 = new_fluid_settings();
    settings2 = new_fluid_settings();
    TEST_ASSERT(settings1 != NULL);
    TEST_ASSERT(settings2 != NULL);

    TEST_SUCCESS(fluid_settings_setint(settings1, "shell.port", 0));
    TEST_SUCCESS(fluid_settings_setint(settings2, "shell.port", 0));

    server1 = new_fluid_server2(settings1, NULL, NULL, NULL);
    TEST_ASSERT(server1 != NULL);
    TEST_SUCCESS(fluid_settings_getint(settings1, "shell.port", &port1));
    TEST_ASSERT(port1 >= TEST_SHELL_AUTO_PORT_START && port1 <= TEST_TCP_PORT_MAX);

    server2 = new_fluid_server2(settings2, NULL, NULL, NULL);
    TEST_ASSERT(server2 != NULL);
    TEST_SUCCESS(fluid_settings_getint(settings2, "shell.port", &port2));
    TEST_ASSERT(port2 >= TEST_SHELL_AUTO_PORT_START && port2 <= TEST_TCP_PORT_MAX);
    TEST_ASSERT(port1 != port2);

    delete_fluid_server(server2);
    delete_fluid_server(server1);
    delete_fluid_settings(settings2);
    delete_fluid_settings(settings1);
#endif

    return EXIT_SUCCESS;
}
