#include <lib.h>

int main(int argc, char **argv) {
    int r;
    if (argc != 2) {
        user_panic("touch format wrong");
    }
    if ((r = open(argv[1], O_WRONLY | O_CREAT)) < 0) {
        user_panic("touch failed");
    }
    return 0;
}
