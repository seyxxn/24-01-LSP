#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(void) {
	struct stat statbuf;
	char *fname = "ssu_myfile";
	int fd;

	if ((fd = open(fname, O_RDWR | O_CREAT, 0600)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	close(fd);
	stat(fname, &statbuf); // fname의 파일 정보를 statbuf에 저장
	printf("# 1st stat call # UID:%d	GID:%d\n", statbuf.st_uid, statbuf.st_gid);
	//소유자 ID와 그룹 사용자 ID를 출력
    
    // chown함수를 이용해 UID, GID를 변경
	if (chown(fname, 501, 300) < 0){
		fprintf(stderr, "chown error for %s\n", fname);
		exit(1);
	}

	stat(fname, &statbuf); // fname의 파일 정보를 statbuf에 다시 저장
	printf("# 2nd stat call # UID:%d	GID:%d\n", statbuf.st_uid, statbuf.st_gid);
	// 바뀐 소유자 ID와 그룹 사용자 ID를 출력

	if (unlink(fname) < 0) {
		fprintf(stderr, "unlink error for %s\n", fname);
		exit(1);
	}

	exit(0);
}
