#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int signo);

int count = 0; // 알람 횟수를 저장하기 위한 변수

int main(void)
{
	signal(SIGALRM, signal_handler); // SIGALRM 시그널에 대한 시그널 핸들러 등록
	alarm(1); // 1초 후에 SIGALRM 시그널을 발생시키도록 알람 설정

	while(1); // 무한 루프를 통해 프로그램을 실행하는 동안 대기

	exit(0); // 프로그램 종료
}

void signal_handler(int signo){
	printf("alarm %d\n", count++); // 알람이 발생할 때마다 알람 횟수를 출력하고 증가시킴
	alarm(1); // 1초 후에 다음 알람을 설정
}
