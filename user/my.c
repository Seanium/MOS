#include <lib.h>

int main(){
	debugf("hello!\n");
	int pid = fork();
	debugf("%d\n", pid);
	debugf("bye!\n");
	return 0;
}
