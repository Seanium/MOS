#include <args.h>
#include <lib.h>

//lab6-challenge
int shell_id = 0;

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	//lab6-challenge
	if (*s == '\"') {
		*p1 = ++s;
		while (*s != 0 && *s != '\"') {
			s++;
		}
		if (*s == 0) {
			debugf("match failed\n");
			return 0;
		}
		*s++ = 0;
		*p2 = s;
		return 'w';
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			if ((fd = open(t, O_RDONLY)) < 0) {
				user_panic("< open failed");
			}
			dup(fd, 0);
			close(fd);
			//user_panic("< redirection not implemented");
			
			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			if ((fd = open(t, O_WRONLY | O_CREAT)) < 0) {
				user_panic("> open failed");
			}
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			pipe(p);
			if ((*rightpipe = fork()) == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			} else {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			//user_panic("| not implemented");

			break;
		case ';':;
			int pid;
			if ((pid = fork()) == 0) {
				return argc;
			} else {
				wait(pid);
				return parsecmd(argv, rightpipe);
			}
			break;
		case '&':
			if (fork() == 0) {
				return argc;
			} else {
				return parsecmd(argv, rightpipe);
			}
			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

int resetGetcmd = 1;
// type为0表示向上，为1表示向下
int getcmd(char *buf, int type) {
	static int fdnum;
	static struct Fd *fd;
	static char *c;
	static char *begin;
	static char *end;
	char *p;
	if (resetGetcmd == 1) {
		if ((fdnum = open(".history", O_RDONLY)) < 0) {
			user_panic(".history open failed");
			return 0;
		}
		fd = (struct Fd *) num2fd(fdnum);
		begin = fd2data(fd);
		end = begin + ((struct Filefd*)fd)->f_file.f_size;
		c = end - 1;
	}
	if (type == 0) { //向上
		//c回跳过上条指令末尾空白或换行符
		while (*c == 0 || *c == '\n') {
			c--;
		}
		//c回跳到上条指令开头前一个字符，或文件开头
		while (*c != '\n' && c > begin) {
			c--;
		}
		//设置p为上条指令第一个字符
		if (*c == '\n') {
			p = c + 1;
		} else {
			p = c;
		}
		int i = 0;
		//复制上条指令内容到buf
		while (*p != '\n' && *p != 0) {
			buf[i] = *p;
			i++;
			p++;
		}
		buf[i] = 0;
		// fprintf(1, "buf: %s len:%d i:%d\n", buf, strlen(buf), i);
		return i;
	} else { //向下
		//c跳过上条指令末尾换行符
		while (*c == '\n' && c < end) {
			c++;
		}
		//c跳到本条指令末尾换行符
		while (*c != '\n' && c < end) {
			c++;
		}
		p = c + 1;
		if (p >= end) {
			buf[0] = 0;
			return 0;
		} else {
			int i = 0;
			while (*p != '\n') {
				buf[i] = *p;
				i++;
				p++;
			}
			buf[i] = 0;
			return i;
		}
	}
}

void readline(char *buf, u_int n) {
	int r;
	int left = 0; //lab6-challenge
	for (int i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				//lab6-challenge
				i--;
				buf[i + 1] = 0;
				for (int j = i - left; j <= i - 1; j++) {
					buf[j] = buf[j + 1];
				}
				buf[i] = 0;
				fprintf(1, "\x1b[%dD", i - left + 1);
				fprintf(1, "\x1b[K");
				fprintf(1, buf);
				if (left > 1) {
					fprintf(1, "\x1b[%dD", left - 1);
				} else if (left == 0) {
					fprintf(1, "\x1b[C");
				}
				i--;
			} else {
				i = -1;
			}
			if (buf[i] != '\b') {
				printf("\b");
			}
		}
		else if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}
		//lab6-challenge
		else if (buf[i] == 27) {
			char c1, c2;
			read(0, &c1, 1);
			read(0, &c2, 1);
			//上
			if(c1 == 91 && c2 == 65) {
				// fprintf(1, "\x1b[B");
				// fprintf(1, "\x1b[%dD", i);
				// char s[] = "testpassed";
				// fprintf(1, "\x1b[K");
				// fprintf(1, s);
				// int len = strlen(s);
				// for (int i = 0; i < len; i++) {
				// 	buf[i] = s[i];
				// }
				// buf[len] = 0;
				// i = strlen(s) - 1;
				fprintf(1, "\x1b[B"); //向下
				if (i > 0) {
					fprintf(1, "\x1b[%dD", i); //左移到开头
				}
				fprintf(1, "\x1b[K"); //删除当前位置到行末
				i = getcmd(buf, 0) - 1; //获取命令（上）
				fprintf(1, "%s", buf); //打印命令
				left = 0; //重置left
			}
			//下
			else if (c1 == 91 && c2 == 66) {
				if (i > 0) {
					fprintf(1, "\x1b[%dD", i); //左移到开头
				}
				fprintf(1, "\x1b[K"); //删除当前位置到行末
				i = getcmd(buf, 1) - 1; //获取命令（下）
				fprintf(1, "%s", buf); //打印命令
				left = 0; //重置left
			}
			//左
			else if (c1 == 91 && c2 == 68) {
				left++;
				buf[i] = 0;
				i--;
				// printf("%d\n", strlen(buf));
				//printf("%c\n", buf[strlen(buf) - 1]);
			}
			//右
			else if (c1 == 91 && c2 == 67) {
				left--;
				buf[i] = 0;
				i--;
			}
			resetGetcmd = 0;
		}
		else {
			resetGetcmd = 1;
			if (left > 0) {
				//printf("%d %d\n", i, left);
				/*
				buf[i - left] = buf[i];
				buf[i] = 0;
				i--;
				left--;
				*/
				char c = buf[i];
				for (int j = i; j >= i - left + 1; j--) {
					buf[j] = buf[j - 1];
				}
				buf[i - left] = c;
				buf[i + 1] = 0;
				fprintf(1, "\x1b[%dD", i - left + 1);
				fprintf(1, "\x1b[K");
				fprintf(1, buf);
				fprintf(1, "\x1b[%dD", left);
			}
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

//lab6-challenge
int savecmd(char *cmd) {
	int r;
	if ((r = open(".history", O_CREAT | O_WRONLY | O_APPEND)) < 0) {
		user_panic(".history open failed");
		return r;
	}
	write(r, cmd, strlen(cmd));
	write(r, "\n", 1);
	return 0;
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	
	//lab6-challenge
	shell_id = syscall_create_shell_id();
	printf("\033[34mcur_shell_id: %d\033[0m\n", shell_id);

	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		savecmd(buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
