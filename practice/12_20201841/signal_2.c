#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// SIGINT, SIGTERM 시그널을 처리하는 핸들러 함수의 선언
static void signal_handler(int signo);

int main(void){
	// SIGINT 시그널에 대한 핸들러 함수 등록 및 오류 처리
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handler SIGINT\n");
		exit(EXIT_FAILURE);
	}

	// SIGTERM 시그널에 대한 핸들러 함수 등록 및 오류 처리
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handler SIGTERM\n");
		exit(EXIT_FAILURE);
	}

	// SIGPROF 시그널의 핸들러 함수를 기본값으로 재설정 및 오류 처리
	if (signal(SIGPROF, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "cannot reset SIGPROF\n");
		exit(EXIT_FAILURE);
	}

	// SIGHUP 시그널을 무시하도록 설정 및 오류 처리
	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "cannot ignore SIGHUP\n");
		exit(EXIT_FAILURE);
	}

	// 무한 루프에서 pause()를 호출하여 프로그램을 대기 상태로 진입
	while(1) {
		pause();
	}

	exit(0);
}

// SIGINT, SIGTERM 시그널을 처리하는 핸들러 함수의 정의
static void signal_handler(int signo) {
	// SIGINT 시그널 처리
	if (signo == SIGINT)
		printf("caught SIGINT\n");
	// SIGTERM 시그널 처리
	else if (signo == SIGTERM)
		printf("caught SIGTERM\n");
	// 예상치 못한 시그널 처리
	else {
		fprintf(stderr, "unexpected signal\n");
		exit(EXIT_FAILURE);
	}

	// 프로그램 정상 종료
	exit(EXIT_SUCCESS);
}
