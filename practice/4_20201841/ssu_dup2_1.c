#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
	char *fname = "ssu_test.txt";
	int fd;

    // creat로 파일을 생성하고 파일 디스크립트를 받아 fd에 저장
    // 일반적으로 파일 디스크립터 0은 표준 입력, 1은 표준 출력, 2는 표준 에러로
    // 그 다음인 3번부터 일반 파일에 대한 디스크립터 번호가 부여됨
	if ((fd = creat(fname, 0666)) < 0) {
		printf("creat error for %s\n", fname);
		exit(1);
	}

	printf("First printf is on the screen.\n");
	dup2(fd, 1); // ssu_test.txt의 디스크립터(fd)를 표준 출력 1번으로 복사 (표준 출력 없어짐)
	// 여기서부터는 표준 출력이 ssu_test.txt로 바뀜
	printf("Second printf is in this file.\n"); // 표준출력이 ssu_test.txt로 바뀌면서 ssu_test.txt에서 해당 문자열이 출력됨
	exit(0);
}
