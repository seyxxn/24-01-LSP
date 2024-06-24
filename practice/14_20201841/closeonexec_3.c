#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

int main(void) {
	int flag;

	// 표준 출력(STDOUT_FILENO)에 대한 파일 디스크립터 플래그를 가져옴
	if ((flag = fcntl(STDOUT_FILENO, F_DUPFD)) == -1) {
		// 플래그를 가져오는 데 실패할 경우 에러 메시지를 출력하고 프로그램 종료
		fprintf(stderr, "Error : Checking CLOSE_ON_EXEC\n");
		exit(1);
	}

	// 플래그 값을 출력
	printf("CLOSE ON EXEC flag is = %d\n", flag);
	exit(0);
}
