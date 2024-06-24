#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
	char *fname = "ssu_test.txt"; // 생성하고자 하는 파일명
	int fd; // 파일 디스트립터

    // creat 함수를 이용하여 파일을 생성하고 fd로 파일 디스크립터를 반환 받음
    // 이때 0보다 작은 값이 반환되었다면 에러처리 필요
	if ((fd = creat(fname, 0666)) < 0){
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1); // 에러 발생 시 강제종료
	}
    // 성공한 경우
    else
    {
		printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);
		close(fd); // 파일 닫기
	}
	exit(0); // 정상종료
}
