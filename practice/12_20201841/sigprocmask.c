#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t sig_set; // 시그널 집합을 저장할 변수
	int count;

	sigemptyset(&sig_set); // 시그널 집합을 초기화
	sigaddset(&sig_set, SIGINT); // 시그널 집합에 SIGINT를 추가
	sigprocmask(SIG_BLOCK, &sig_set, NULL); // 시그널 집합에 포함된 시그널을 블록

    // 3부터 카운트를 시작하여 0이 될 때까지 반복
	for(count = 3; 0 < count; count--) {
		printf("count %d\n", count); // 카운트 출력
		sleep(1); // 1초 쉬기
	}

	printf("Ctrl-C에 대한 블록을 해제\n");
	sigprocmask(SIG_UNBLOCK, &sig_set, NULL); // 시그널 집합에 포함된 시그널 블록 해제
	printf("count중 Ctrl-C입력하면 이 문장은 출력 되지 않음.\n");

	while(1); // 무한 루프로 프로그램이 종료되지 않도록 함

	exit(0); // 프로그램 종료

}
