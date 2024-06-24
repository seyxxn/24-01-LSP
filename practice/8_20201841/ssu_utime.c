#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>

int main(int argc, char *argv[])
{
	struct utimbuf time_buf;
	struct stat statbuf;
	int fd;
	int i;

	for (i = 1; i < argc; i++){
	    // 인자로 받는 파일들의 정보를 stat 구조체에 저장
		if(stat(argv[i], &statbuf) < 0) {
			fprintf(stderr, "stat error for %s\n", argv[i]);
			continue;
		}
        // 파일을 읽기/쓰기 모드로 열고, O_TRUNC 옵션으로 파일의 내용을 비움
		if ((fd = open(argv[i], O_RDWR | O_TRUNC)) < 0) {
			fprintf(stderr,"open error for %s\n", argv[i]);
			continue;
		}

		close(fd); // 파일디스크립터 닫음
		time_buf.actime = statbuf.st_atime;
		// 파일의 마지막 접근시간을 utimbuf 구조체 변수에 저장
		time_buf.modtime = statbuf.st_mtime;
		// 파일의 마지막 수정시간을 utimbuf 구조체 변수에 저장

        // utime으로 위에서 저장한 파일의 마지막 접근 시간과 마지막 수정 시간을 원래대로 복구
		if (utime(argv[i], &time_buf) < 0) {
			fprintf(stderr, "utime error for %s\n", argv[i]);
			continue;
		}
	}
	exit(0);
}
