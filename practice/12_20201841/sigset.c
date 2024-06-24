#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(void){
	sigset_t set; // 시그널 집합을 저장할 변수

	sigemptyset(&set); // 시그널 집합을 초기화
	sigaddset(&set, SIGINT); // SIGINT 시그널을 시그널 집합에 추가

    // SIGINT가 시그널 집합에 포함되어 있는지 확인하고 결과에 따라 메시지 출력
	switch (sigismember(&set, SIGINT))
	{
		case 1:
			printf("SIGINT is included. \n");
			break;
		case 0 :
			printf("SIGINT is not included. \n");
			break;
		default :
			printf("failed to call sigismember() \n");
	}
    
    // SIGSYS가 시그널 집합에 포함되어 있는지 확인하고 결과에 따라 메시지 출력
	switch(sigismember(&set, SIGSYS))
	{
		case 1:
			printf("SIGSYS is included. \n");
			break;
		case 0:
			printf("SIGSYS is not included. \n");
			break;
		default :
			printf("failed to call sigismember() \n");
	}

	exit(0); // 프로그램 종료
}
