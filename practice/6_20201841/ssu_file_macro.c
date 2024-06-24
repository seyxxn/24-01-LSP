#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
	struct stat file_info;  // 파일의 정보를 저장할 stat 구조체 선언
	char *str;
	int i;

	for(i = 1; i < argc; i++){
		printf("name = %s, type = ", argv[i]);

		if (lstat(argv[i], &file_info) < 0){ // lstat 함수로 파일의 정보를 file_info에 받음
			fprintf(stderr, "lstat error\n"); // 에러처리
			continue;
		}
		
        // file_info의 st_mode는 파일의 형식을 가지고 있음
        // 파일의 종류를 확인하는 매크로를 통해 파일의 종류 출력
		if (S_ISREG(file_info.st_mode))  // 일반 파일인 경우
			str = "regular";
		else if (S_ISDIR(file_info.st_mode)) // 디렉터리인 경우
			str = "directory";
		else if (S_ISCHR(file_info.st_mode)) // 문자 특수 파일인 경우
			str = "character special";
		else if (S_ISBLK(file_info.st_mode)) // 블럭 특수 파일인 경우
			str = "block special";
		else if (S_ISFIFO(file_info.st_mode)) // 프로세스간 통신에 쓰이는 파일인 경우
			str = "FIFO";
		else if (S_ISLNK(file_info.st_mode)) // 심볼릭링크인 경우(다른 파일을 가리키는 파일)
			str = "symbolic link";
		else if (S_ISSOCK(file_info.st_mode)) // 프로세스간 네트워크 통신에 쓰이는 파일인 경우
			str = "socket";
		else
			str = "unknown mode";

		printf("%s\n", str);
	}

	exit(0);
}
