#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH_MAX 1024

int main(void){
	char *pathname;

    // chdir로 현재 작업 디렉터리를 /home/seyeon으로 변경
	if (chdir("/home/seyeon") < 0) {
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

	pathname = malloc(PATH_MAX);

    // getcwd 함수로 현재 작업디렉터리를 pathname에 저장
	if (getcwd(pathname, PATH_MAX) == NULL){
		fprintf(stderr, "getcwd error\n");
		exit(1);
	}

	printf("current directory = %s\n", pathname); // 현재 작업 디렉터리 확인
	exit(0);
}
