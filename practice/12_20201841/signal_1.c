#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 함수 선언
void signal_handler(int signo);

// 함수 포인터 선언
void (*func)(int);

int main(void){
	// SIGINT 시그널에 대한 핸들러 함수를 func에 할당하고, 현재 등록된 핸들러 함수를 func에 저장
	func = signal(SIGINT, signal_handler);

	while(1) {
		printf("processing running...\n");
		sleep(1);
	}
	exit(0);
}

// SIGINT 시그널에 대한 핸들러 함수
void signal_handler(int signo) {
	printf("SIGINT 시그널 발생.");
	printf("SIGINT를 SIG_DEL로 재설정 함.\n");
	// 이전에 등록된 SIGINT 시그널 핸들러 함수를 func에 저장된 함수로 재설정
	signal(SIGINT, func);
}
