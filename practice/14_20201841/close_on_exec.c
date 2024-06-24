#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
	int fd;
	int val;

	if ((fd = open("exec_copy.txt", O_CREAT)) < 0) {
		fprintf(stderr, "open error for %s\n", "exec_copy.txt");
		exit(1);
	}
	val = fcntl(fd, F_GETFD, 0); // fd의 파일 디스크립터 플래그를 가져옴

    // FD_CLOEXEC 비트가 설정되어 있는지 확인
	if (val & FD_CLOEXEC)
		printf("close-on-exec bit on\n");
	else
		printf("close-on-exec bit off\n");

	val |= FD_CLOEXEC; // FD_CLOEXEC 비트를 설정

    // FD_CLOEXEC 비트가 설정되어 있는지 다시 확인
	if(val & FD_CLOEXEC)
		printf("close-on-exec bit on\n");
	else
		printf("close-on-exec bit off\n");

	fcntl(fd, F_SETFD, val); // 변경된 파일 디스크립터 플래그를 fd에 설정
	execl("/home/seyeon/loop", "./loop", NULL);
	// "/home/seyeon/loop" 프로그램을 실행, 이때 현재 프로세스의 메모리 공간이 대체됨
	
	// execl 호출 이후에는 코드가 실행되지 않음
}
