#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[]) {
	// 명령행 인자가 2개가 아닌 경우 오류 메시지 출력 후 종료
	if (argc != 2){
		fprintf(stderr, "usage: %s [Process ID]\n", argv[0]);
		exit(1);
	}
	else
		// 명령행 인자로 전달된 프로세스 ID에 SIGKILL 시그널을 보냄
		kill(atoi(argv[1]), SIGKILL);
	exit(0);
}
