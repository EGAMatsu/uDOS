/* uDOS console command line
 *
 * Supports a small JCL runtime to help execute jobs
 */

#include <alloc.h>
#include <panic.h>
#include <string.h>
#include <user.h>

int ucli_init(void) {
    kprintf("\e[37;1m");
    kprintf("      $$$$$. .$$$$$. .$$$   \n");
    kprintf("\e[0m\e[37m");
    kprintf("      $    $ $     $ $      \n");
    kprintf("\e[36;1m");
    kprintf(" .  . $    $ $     $ `$$`.  \n");
    kprintf("\e[0m\e[36m");
    kprintf(" $  $ $    $ $     $     $  \n");
    kprintf("\e[34;1m");
    kprintf(" $  $ $    $ $     $     $  \n");
    kprintf("\e[0m\e[34m");
    kprintf(" `$$` $$$$$` `$$$$$` .$$.`  \n");
    kprintf("\e[0m");
    kprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    kprintf("JCL scripting is supported. \n");

    kprintf("An Operating System for mainframes and computers.\n");
    kprintf(
        "Copyright (C) 2021 superleaf1995. All rights reserved. Type `info` "
        "for more info.\n");
    kprintf("Use `help` command to see more commands");
    return 0;
}
