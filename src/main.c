#include "fs.h"

int main(void) {
    fs_init();
    fs_shell_loop();
    fs_shutdown();
    return 0;
}
