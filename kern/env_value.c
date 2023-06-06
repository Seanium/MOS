#include <printk.h>

int get_shell_id();
int create_shell_id();
int declare_env_value(char *name, char *value, int shell_id, int rdonly, int global);
int unset_env_value(char *name, int shell_id);
int get_env_value(char *name, int shell_id, int op, char *value);

struct ENV_VALUE {
    char name[32];
    char value[32];
    int shell_id;   //为0表示全局
    int rdonly;
    int valid;
} env_value[128];

static int env_value_cnt = 0;
static int cur_shell_id = 0;

int get_shell_id() {
    return cur_shell_id;
}

int create_shell_id() {
    return ++cur_shell_id;
}

//比较两字符串是否相同
int sameStr(char *str1, char *str2){
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if (len1 != len2) {
        return 0;
    }
    for (int i = 0; i < len1; i++) {
        if (str1[i] != str2[i]) {
            return 0;
        }
    }
    return 1;
}

//如果存在，返回该环境变量的下标
//如果不存在，返回-1
int exist_env_value(char *name, int shell_id) {
    for (int i = 0; i < env_value_cnt; i++) {
        if ((env_value[i].shell_id == 0 || env_value[i].shell_id == shell_id) 
        && env_value[i].valid && sameStr(name, env_value[i].name)) {
            return i;
        }
    }
    return -1;
}

int declare_env_value(char *name, char *value, int shell_id, int rdonly, int global) {
    int r;
    if ((r = exist_env_value(name, shell_id)) != -1) { //如果name存在
        if (env_value[r].rdonly == 1) {
            printk("\033[33m%s: read only\033[0m\n", env_value[r].name);
            return -1;
        } else {
            strcpy(env_value[r].value, value);
            env_value[r].shell_id = global? 0: shell_id;
            env_value[r].rdonly = rdonly;
        }
    } else { //如果name不存在
        strcpy(env_value[env_value_cnt].name, name);
        strcpy(env_value[env_value_cnt].value, value);
        env_value[env_value_cnt].shell_id = global? 0: shell_id;
        env_value[env_value_cnt].rdonly = rdonly;
        env_value[env_value_cnt].valid = 1;
        env_value_cnt++;
    }
    return 0;
}

int unset_env_value(char *name, int shell_id) {
    int r;
    if ((r = exist_env_value(name, shell_id)) != -1) {
        if (env_value[r].rdonly == 1) {
            printk("\033[33m%s: read only\033[0m\n", env_value[r].name);
        } else {
            env_value[r].valid = 0;
        }
    } else {
        printk("\033[33mcannot find %s\033[0m\n", name);
        return -1;
    }
    return 0;
}

int get_env_value(char *name, int shell_id, int op, char *value) {
    if (op == 0) {
        for (int i = 0; i < env_value_cnt; i++) {
            if ((env_value[i].shell_id == 0 || env_value[i].shell_id == shell_id)
             && env_value[i].valid == 1) {
                if(env_value[i].shell_id == 0) {
                    printk("\033[33mname: %s, value: %s, shell_id: GLOBAL, readonly: %d\033[0m\n",
                     env_value[i].name, env_value[i].value, env_value[i].rdonly);
                } else {
                    printk("\033[33mname: %s, value: %s, shell_id: %d, readonly: %d\033[0m\n",
                     env_value[i].name, env_value[i].value, env_value[i].shell_id, env_value[i].rdonly);
                }
            }
        }
    } else if (op == 1) {
        int r;
        if ((r = exist_env_value(name, shell_id)) != -1) {
            printk("\033[33m%s\033[0m\n", env_value[r].value);
        } else {
            printk("\033[33mcannot find %s\033[0m\n", name);
            return -1;
        }
    }
    return 0;
}