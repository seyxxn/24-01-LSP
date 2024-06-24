#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(void){
	char *fname = "ssu_test.txt";
	off_t fsize; // 파일 사이즈를 받을 변수 선언
	int fd; // 파일 디스크립터

    // open 함수를 이용해 파일을 읽기 전용으로 열고 fd를 반환받음
    // 이때 fd가 음수라면 에러 처리 필요
	if ((fd = open(fname, O_RDONLY)) < 0 ) {
		fprintf(stderr, "open error for %s\n", fname); // open에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}

    // lseek 함수를 이용하여 파일의 크기를 구할 수 있음
    // lseek(fd,0,SEEK_END) -> 파일 커서가 파일 내용의 가장 끝을 가리키고 있으므로 파일의 크기와 일치한다.
    // 이때 fsize가 음수라면 에러 처리 필요
	if ((fsize = lseek(fd, 0, SEEK_END)) < 0){
		fprintf(stderr, "lseek error\n"); // lseek에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}
    
	printf("The size of <%s> is %ld bytes.\n", fname, fsize); // 파일명과 파일의 크기를 출력

	exit(0); // 정상종료
}
