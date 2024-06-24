#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define MSG "message will be written to Terminal\n"  // 출력할 메시지

int main(void) {
	int new_fd;  // 새로운 파일 디스크립터

	// STDOUT_FILENO(표준 출력)를 복사하여 새로운 파일 디스크립터 생성
	if ((new_fd = fcntl(STDOUT_FILENO, F_DUPFD, 3)) == -1) {
		fprintf(stderr, "Error : Copying File Descriptor\n");  // 복사 실패 시 에러 메시지 출력
		exit(1);  // 프로그램 종료
	}

	// 표준 출력 파일 디스크립터 닫기
	close(STDOUT_FILENO);

	// 파일 디스크립터 3을 통해 메시지 출력
	write(3, MSG, strlen(MSG));

	exit(0);  // 프로그램 종료
}
