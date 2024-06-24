#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define MODE_EXEC (S_IXUSR | S_IXGRP | S_IXOTH)
// user의 실행권한, group의 실행권한, other의 실행권한

int main(int argc, char *argv[])
{
	struct stat statbuf;
	int i;
	
    // 파일명을 인자로 받지 못한 경우 에러 처리
	if (argc < 2) {
		fprintf(stderr, "usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (stat(argv[i], &statbuf) < 0) { // stat함수로 파일의 정보를 statbuf에 저장
			fprintf(stderr, "%s : stat error\n", argv[i]);
			continue;
		}

		statbuf.st_mode |= MODE_EXEC;
		// 파일의 현재 접근 권한에서 MODE를 추가(위의 주석에 있는 권한들)
		
		statbuf.st_mode ^= (S_IXGRP | S_IXOTH);
		// XOR연산자로 반전 시킴.
		// group과 other의 실행권한을 다시 제거
		
		if (chmod(argv[i], statbuf.st_mode) < 0)
			fprintf(stderr, "%s : chmod error\n", argv[i]);
		else
			printf("%s : file permission was changed.\n", argv[i]);
	}
	exit(0);
}

// /bin/su : 일반 사용자 권한으로 파일 접근 권한을 변경할 수 없음 -> 에러메세지 출력
// /home/user/새로만든디렉터리 : 파일의 권한이 바뀌었음을 출력