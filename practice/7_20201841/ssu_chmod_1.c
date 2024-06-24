#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void){
	struct stat statbuf;
	char *fname1 = "ssu_file1";
	char *fname2 = "ssu_file2";

    // stat함수로 fname1의 파일 정보를 statbuf에 저장
	if(stat(fname1, &statbuf) < 0)
		fprintf(stderr, "stat error %s\n", fname1);

    // chmod함수로 접근 권한을 변경
    // (statbuf.st_mode & ~S_IXGRP) : 현재 접근 권한에서 S_IXGRP(그룹의 실행)권한을 제거
    // | S_ISUID : set user id 권한을 추가
	if (chmod(fname1, (statbuf.st_mode & ~S_IXGRP) | S_ISUID) < 0)
		fprintf(stderr, "chmod error %s\n", fname1);

    // user의 읽기, 쓰기 권한과 group의 읽기 권한과 other의 읽기, 실행권한을 부여
	if (chmod(fname2, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXOTH) < 0)
		fprintf(stderr, "chmod error %s\n", fname2);

	exit(0);
}
