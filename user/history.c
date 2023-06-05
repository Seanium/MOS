#include <lib.h>

int main(int argc, char **argv) {
    if (argc == 1) {
        int his = open(".history", O_RDONLY | O_CREAT);
        char ch;
        int cnt = 1, flag = 1;
        while (read(his, &ch, 1) == 1) {
            if (flag == 1) {
                fprintf(1, "\033[34m%d\033[0m ", cnt);
                flag = 0;
            }
            fprintf(1, "%c", ch);
            if (ch == '\n') {
                cnt++;
                flag = 1;
            }
        }
    } else {
        user_panic("history format wrong");
    }
    return 0;
}