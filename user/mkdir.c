#include <lib.h>

int main(int argc, char **argv) {
    int r;
    if (argc != 2) {
        user_panic("mkdir format wrong");
    }
    if ((r = open(argv[1], O_WRONLY | O_MKDIR)) < 0) {
        user_panic("mkdir failed");
    }
    return 0;
}
