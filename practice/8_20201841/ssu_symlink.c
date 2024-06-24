#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    
    // 첫번째 인자로 실제 파일의 경로 이름, 두번째 인자로 심볼릭 링크의 이름을 받아야함
	if (argc != 3){
		fprintf(stderr, "usage: %s <actualname> <symname>\n", argv[0]);
		exit(1);
	}

    // symlink 함수를 통해 첫번째 파일의 심볼릭 링크를 생성, 생성한 심볼릭 링크의 이름은 두번째 인자로 지정
	if (symlink(argv[1], argv[2]) < 0){
		fprintf(stderr, "symlink error\n");
		exit(1);
	}
	else {
		printf("symlink: %s -> %s\n", argv[2], argv[1]);
	}

	exit(0);
}
