#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define RW_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int main(void){
	char *fname1 = "ssu_file1";
	char *fname2 = "ssu_file2";

	umask(0);

	if (creat(fname1, RW_MODE) < 0) { // 위에서 선언해준 RW_MODE로 파일을 생성 (rw-rw-rw-)
		fprintf(stderr, "creat error for %s\n", fname1);
		exit(1);
	}

	umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	// umask 함수를 통해 파일 생성 시에 기본 권한을 제한하는 마스크를 일시적으로 변경함
	// 현재 넘긴 인자로는 Gropu의 읽기, 쓰기 권한과
	// Other의 읽기, 쓰기 권한이 제한됨

	if (creat(fname2, RW_MODE) < 0){
		fprintf(stderr, "creat error for %s\n", fname2);
		exit(1);
	}
	exit(0);
}
