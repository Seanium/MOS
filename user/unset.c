#include <lib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        user_panic("unset format wrong");
    } else {
        int cur_shell_id = syscall_get_shell_id();
        syscall_unset_env_value(argv[1], cur_shell_id);
    }
    return 0;
}