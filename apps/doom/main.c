#include "doomgeneric-src/doomgeneric/doomgeneric.h"

void termuos_shim_init(void);

int main(void)
{
    static char arg0[] = "doom";
    static char arg1[] = "-iwad";
    static char arg2[] = "/doom1.wad";
    static char *argv[] = {arg0, arg1, arg2, 0};
    termuos_shim_init();
    doomgeneric_Create(3, argv);
    for (;;)
        doomgeneric_Tick();
}
