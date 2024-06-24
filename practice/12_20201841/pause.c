#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_alarm(int signo); // SIGALRM 시그널을 처리할 함수의 선언

int main(void){
	printf("Alarm Setting\n"); // 알람 설정 메시지 출력
	signal(SIGALRM, ssu_alarm); // SIGALRM 시그널을 처리할 함수 등록
	alarm(2); // 2초 후에 SIGALRM 시그널 발생

	while(1) {
		printf("done\n");
		pause(); // 시그널이 발생할 때까지 대기
		alarm(2); // 2초 후에 다음 알람 설정
	}

	exit(0);
}

// SIGALRM 시그널을 처리하는 함수
void ssu_alarm(int signo) {
	printf("alarm..!!!\n"); // 알람 메시지 출력
}
