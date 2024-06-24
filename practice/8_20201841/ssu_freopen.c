#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
	char *fname = "ssu_test.txt";
	int fd;

	printf("First printf : Hello, OSLAB!\n");

	if ((fd = open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

    // freopen으로 파일을 쓰기 모드로 다시 열고 stdout(표준출력)에 연결
	if (freopen(fname, "w", stdout) != NULL)
		printf("Second printf : Hello, OSLAB!!\n"); // 출력되는 내용은 콘솔이 아닌 ssu_test.txt에 출력됨

	exit(0);
}
