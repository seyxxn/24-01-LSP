#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define CREAT_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char buf1[] = "1234567890";
char buf2[] = "ABCDEFGHIJ";

int main(void){
	char *fname = "ssu_hole.txt"; // 생성하려는 파일명
	int fd; // 파일 디스크립터

    // creat 함수를 이용해 파일을 생성하고 fd로 파일 디스크립터를 반환받음
    // 이때 fd가 음수하면 에러 처리 필요
	if ((fd = creat(fname, CREAT_MODE)) < 0){
		fprintf(stderr, "creat error for %s\n", fname); // creat에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}

    // write 함수를 이용하여 파일에 데이터를 씀
    // 오프셋 위치의 기본값은 0이기 때문에 가장 맨 앞을 가리키고 있음
    // 따라서 파일의 맨 앞에서부터 12byte만큼의 데이터가 써짐
    // write는 기록된 바이트 수를 반환하므로, 정해진 바이트만큼의 데이터가 기록되지 않았다면 에러 처리 필요
	if (write(fd, buf1, 12) != 12){
		fprintf(stderr, "buf1 write error\n"); // buf1을 write하는 데에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}

    // lseek 함수를 이용하여 파일 내의 커서를 이동시킴
    // lseek(fd, 15000, SEEK_SET) -> 현재 가리키고 있는 곳에서부터 15000bytes를 이동시키겠다는 의미
    // lseek는 성공 시 위치한 seek pointer 위치를 반환하기에 음수가 반환되면 에러 처리 필요
	if (lseek(fd, 15000, SEEK_SET) < 0){
		fprintf(stderr, "lseek error\n"); // lseek에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}

	if (write(fd, buf2, 12) != 12) {
		fprintf(stderr, "buf2 write error\n"); // buf2를 write하는 데에서 에러가 발생했음을 알리는 출력문
		exit(1); // 강제종료
	}

	exit(0); // 정상종료
}
