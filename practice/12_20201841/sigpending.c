#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t pendingset; // 대기 중인 시그널 집합을 저장할 변수
	sigset_t sig_set; // 시그널 집합을 저장할 변수

	int count = 0; // 카운트를 위한 변수

	sigfillset(&sig_set); // 시그널 집합을 모두 채움 (모든 시그널을 포함)
	sigprocmask(SIG_SETMASK, &sig_set, NULL); // 현재 시그널 마스크를 시그널 집합으로 설정

    // 무한 루프
	while (1) {
		printf("count: %d\n", count++); // 카운트 출력
		sleep(1); // 1초 쉬기

        // 대기 중인 시그널 확인
		if (sigpending(&pendingset) == 0) {
		    // SIGINT가 대기 중인지 확인
			if (sigismember(&pendingset, SIGINT)) {
				printf("SIGINT가 블록되어 대기 중. 무한 루프를 종료.\n"); // SIGINT가 블록되어 있으면 메시지 출력
				break; // 무한 루프 종료
			}
		}
	}
	exit(0); // 프로그램 종료
}
