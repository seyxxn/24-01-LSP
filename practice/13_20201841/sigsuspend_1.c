#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void) {
    sigset_t old_set;
    sigset_t sig_set;

    // 빈 신호 집합 초기화
    sigemptyset(&sig_set);

    // SIGINT 신호를 신호 집합에 추가
    sigaddset(&sig_set, SIGINT);

    // 현재 신호 마스크에 SIGINT 신호를 블록하여 설정, 이전 신호 마스크를 old_set에 저장
    sigprocmask(SIG_BLOCK, &sig_set, &old_set);

    // old_set에 의해 지정된 신호가 수신될 때까지 대기 (SIGINT 블록 해제)
    sigsuspend(&old_set);

    exit(0);
}
