#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
	char *fname = "ssu_dump.txt";

	if (open(fname, O_RDWR) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

    // unlink함수로 파일의 링크를 해제
	if (unlink(fname) < 0) {
		fprintf(stderr, "unlink error for %s\n", fname);
		exit(1);
	}

	printf("File unlinked\n"); // 파일이 해제되었음을 알리는 출력문
	sleep(20);
	printf("Done\n");
	exit(0);
}
