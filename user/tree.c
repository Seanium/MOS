#include <lib.h>

void print_tab(int num) {
    for(int i = 0; i < num; i++)
    {
        fprintf(1, "    ");
    }
}

void get_tree(int depth, char *path) {
    int r;
    if((r = open(path, O_RDONLY)) < 0) {
        user_panic("tree open dir failed");
    }

    struct Fd *fd = (struct Fd *) num2fd(r);
    struct Filefd *fileFd = (struct FileFd *) fd;

    fprintf(1, "\033[34m%s\033[0m\n", fileFd->f_file.f_name);
    
    u_int num = ROUND(fileFd->f_file.f_size, sizeof(struct File)) / sizeof(struct File);
    struct File *file = (struct File *) fd2data(fd);

    for(int i = 0; i < num; i++)
    {
        if(file[i].f_name[0] == '\0') continue;
        print_tab(depth);
        fprintf(1, "|---");
        if (file[i].f_type == FTYPE_DIR)
        {
            char newpath[MAXPATHLEN];

            strcpy(newpath, path);
            int len = strlen(path);
            if(newpath[len - 1] != '/') {
                *(newpath + len++) = '/';
            }
            strcpy(newpath + len, file[i].f_name);
            get_tree(depth + 1, newpath);
        }
        else
        {
            fprintf(1, "%s\n", file[i].f_name);
        }
    }
}
int main(int argc, char **argv) {
    if (argc == 1) {
        get_tree(0, "/");
    } else if (argc == 2) {
        get_tree(0, argv[1]);
    } else {
        user_panic("tree format wrong");
    }
    return 0;
}
