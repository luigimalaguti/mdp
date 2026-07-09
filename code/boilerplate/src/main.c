#include "linc.h"

int main(int argc, char *argv[]) {
    linc_set_module_level(linc_default_module, LINC_LEVEL_TRACE);

    INFO("Running Boilerplate main...");
    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            TRACE("Argv[%d] = %s", i, argv[i]);
        }
    }
    DEBUG("Ended Boilerplate main...");

    return 0;
}
