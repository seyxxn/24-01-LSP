#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define DIRECTORY_SIZE MAXNAMLEN

int main(int argc, char *argv[]){
	struct dirent *dentry;
	struct stat statbuf;
	char filename[DIRECTORY_SIZE + 1];
	DIR *dirp;

    // 디렉터리 경로를 인자로 받음
	if (argc <2) {
		fprintf(stderr, "usage: %s <directory>\n", argv[0]);
		exit(1);
	}

    // opendir로 사용자가 지정한 디렉터리를 연다
    // chdir로 현재 작업 디렉터리를 사용자가 지정한 디렉터리로 변경
    // 둘 중 하나라도 오류가 발생하면 오류 메세지 출력 후 강제 종료
	if ((dirp = opendir(argv[1])) == NULL || chdir(argv[1]) == -1) {
		fprintf(stderr, "opendir, chdir error for %s\n", argv[1]);
		exit(1);
	}

    // readdir 함수를 반복문으로 호출하여 디렉터리 내의 모든 항목에 대한 정보를 가져옴
	while((dentry = readdir(dirp)) != NULL) {
		// d_ino == 0이면 유효하지 않은 엔트리로 무시
		if (dentry->d_ino == 0)
			continue;

		memcpy(filename, dentry->d_name, DIRECTORY_SIZE);
		// 디렉터리 엔드리에서 파일이름은 filename 배열로 복사

        // stat 함수로 현재 파일의 정보를 statbuf에 저장
		if (stat(filename, &statbuf) == -1) {
			fprintf(stderr, "stat error for %s\n", filename);
			break;
		}

        // 파일이 일반 파일인 경우에만 파일의 이름과 크기를 출력
		if ((statbuf.st_mode & S_IFMT) == S_IFREG)
			printf("%-14s %ld\n", filename, statbuf.st_size);
		else
			printf("%-14s\n", filename);
	}
	exit(0);
}
