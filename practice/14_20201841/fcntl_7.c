#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
	char *filename = "test.txt";
	int fd1, fd2;
	int flag;

	// "test.txt" 파일을 읽기/쓰기 및 추가 모드로 엶
	if ((fd1 = open(filename, O_RDWR | O_APPEND, 0644)) < 0) {
		// 파일 열기에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "open error for %s\n", filename);
		exit(1);
	}

	// fd1에 FD_CLOEXEC 플래그를 설정
	if (fcntl(fd1, F_SETFD, FD_CLOEXEC) == -1) {
		// 플래그 설정에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_SETFL error\n");
		exit(1);
	}

	// fd1의 파일 상태 플래그를 가져옴
	if ((flag = fcntl(fd1, F_GETFL, 0)) == -1) {
		// 플래그 가져오기에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	// O_APPEND 플래그가 설정되어 있는지 확인
	if (flag & O_APPEND)
		printf("fd1 : O_APPEND flag is set.\n");
	else
		printf("fd1 : O_APPEND flag is NOT set.\n");

	// fd1의 파일 디스크립터 플래그를 가져옴
	if ((flag = fcntl(fd1, F_GETFD, 0)) == -1) {
		// 플래그 가져오기에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}

	// FD_CLOEXEC 플래그가 설정되어 있는지 확인
	if (flag & FD_CLOEXEC)
		printf("fd1 : FD_CLOEXEC flag is set.\n");
	else
		printf("fd1 : FD_CLOEXEC flag is NOT set.\n");

	// fd1을 복제하여 fd2에 할당
	if ((fd2 = fcntl(fd1, F_DUPFD, 0)) == -1) {
		// 복제에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_DUPFD error\n");
		exit(1);
	}

	// fd2의 파일 상태 플래그를 가져옴
	if ((flag = fcntl(fd2, F_GETFL, 0)) == -1) {
		// 플래그 가져오기에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	// O_APPEND 플래그가 설정되어 있는지 확인
	if (flag & O_APPEND)
		printf("fd2 : O_APPEND flag is set.\n");
	else
		printf("fd2 : O_APPEND flag is NOT set.\n");

	// fd2의 파일 디스크립터 플래그를 가져옴
	if ((flag = fcntl(fd2, F_GETFD, 0)) == -1) {
		// 플래그 가져오기에 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}

	// FD_CLOEXEC 플래그가 설정되어 있는지 확인
	if (flag & FD_CLOEXEC)
		printf("fd2 : FD_CLOEXEC flag is set.\n");
	else
		printf("fd2 : FD_CLOEXEC flag is NOT set.\n");

	exit(0);
}
