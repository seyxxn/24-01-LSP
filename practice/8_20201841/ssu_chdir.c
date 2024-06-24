#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){
    // chdir로 현재 작업 디렉터리를 /etc로 변경
	if(chdir("/etc") < 0){
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

	printf("chdir to /etc succeeded.\n");
	exit(0);
}
