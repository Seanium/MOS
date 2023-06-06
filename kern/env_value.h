int create_shell_id();
int declare_env_value(char *name, int value, int shell_id, int rdonly);
int unset_env_value(char *name, int shell_id);
int get_env_value(char *name, int shell_id, int op);