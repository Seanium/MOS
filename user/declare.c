#include <lib.h>

int main(int argc, char **argv) {
    int cur_shell_id = syscall_get_shell_id();
    char name[32] = "";
    char value[32] = "";
    if (argc == 1) {
        syscall_get_env_value(NULL, cur_shell_id, 0, NULL);
    } else if (argc == 2) {
        strcpy(name, argv[1]);
        // fprintf(1, "name: %s", name);
        syscall_declare_env_value(name, value, cur_shell_id, 0, 0);
    } else if (argc == 3) {
        if (argv[2][0] == '=') {
            strcpy(name, argv[1]);
            strcpy(value, argv[2] + 1);
            syscall_declare_env_value(name, value, cur_shell_id, 0, 0);
        } else if (argv[1][0] == '-') {
            strcpy(name, argv[2]);
            int rdonly = 0;
            int global = 0;
            int len = strlen(argv[1]);
            for (int i = 1; i < len; i++) {
                if (argv[1][i] == 'x') {
                    global = 1; //全局
                }
                if (argv[1][i] == 'r') {
                    rdonly = 1;
                }
            }
            // fprintf(1, "name: %s, cur_shell_id: %d, rdonly: %d, value: %s\n", name, cur_shell_id, rdonly, value);
            syscall_declare_env_value(name, value, cur_shell_id, rdonly, global);
        } else {
            user_panic("declare format wrong");
        }
    } else if (argc == 4) {
        if (argv[1][0] != '-' || argv[3][0] != '=') {
            user_panic("declare format wrong");
        }
        strcpy(name, argv[2]);
        strcpy(value, argv[3] + 1);
        int rdonly = 0;
        int global = 0;
        int len = strlen(argv[1]);
        for (int i = 1; i < len; i++) {
            if (argv[1][i] == 'x') {
                global = 1; //全局
            }
            if (argv[1][i] == 'r') {
                rdonly = 1;
            }
        }
        // fprintf(1, "name: %s, cur_shell_id: %d, rdonly: %d, value: %s\n", name, cur_shell_id, rdonly, value);
        syscall_declare_env_value(name, value, cur_shell_id, rdonly, global);
    } else {
        user_panic("declare format wrong");
    }
    return 0;
}