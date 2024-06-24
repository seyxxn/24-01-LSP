#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// SIGINT 시그널을 처리하는 핸들러 함수의 선언
void signal_handler1(int signo);
// SIGUSR1 시그널을 처리하는 핸들러 함수의 선언
void signal_handler2(int signo);

int main(void){
	// SIGINT 시그널에 대한 핸들러 함수 등록 및 오류 처리
	if (signal(SIGINT, signal_handler1) == SIG_ERR) {
		fprintf(stderr, "cannot hanler SIGINT\n");
		exit(EXIT_FAILURE);
	}

	// SIGUSR1 시그널에 대한 핸들러 함수 등록 및 오류 처리
	if (signal(SIGUSR1, signal_handler2) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGUSR1\n");
		exit(EXIT_FAILURE);
	}

	// SIGINT 시그널 발생
	raise(SIGINT);
	// SIGUSR1 시그널 발생
	raise(SIGUSR1);
	// "main return" 출력
	printf("main return\n");
	exit(0);
}

// SIGINT 시그널을 처리하는 핸들러 함수의 정의
void signal_handler1(int signo) {
	printf("SIGINT 시그널 발생\n");
}

// SIGUSR1 시그널을 처리하는 핸들러 함수의 정의
void signal_handler2(int signo) {
	printf("SIGUSR1 시그널 발생\n");
}
