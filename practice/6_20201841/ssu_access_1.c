#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	int i;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file1> <file2>.. <fileN>\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (access(argv[i], F_OK) < 0) { // access 함수로 F_OK를 통해 파일의 존재여부를 확인 
			fprintf(stderr, "%s doesn't exist.\n", argv[i]); // 존재하지 않으면 에러처리
			continue;
		}

		if (access(argv[i], R_OK) == 0) // access 함수로 R_OK를 통해 읽기 권한 여부를 확인
			printf("User can read %s\n" ,argv[i]);

		if (access(argv[i], W_OK) == 0) // access 함수로 W_OK를 통해 쓰기 권한 여부를 확인
			printf("User can write %s\n", argv[i]); 

		if (access(argv[i], X_OK) == 0) // access 함수로 X_OK를 통해 실행 권한 여부를 확인
			printf("User can execute %s\n", argv[i]); 
	}
	exit(0);
}
