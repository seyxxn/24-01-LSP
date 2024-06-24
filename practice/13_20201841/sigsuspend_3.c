#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// SIGALRM 시그널 핸들러 함수
void ssu_signal_handler(int signo);

// 현재 시간을 출력하는 함수
void ssu_timestamp(char *str);

int main(void)
{
    struct sigaction sig_act; // 시그널 액션 구조체
    sigset_t blk_set; // 블록할 시그널 집합

    // 블록할 시그널 집합 설정 (모든 시그널을 포함)
    sigfillset(&blk_set);
    // SIGALRM은 블록하지 않음
    sigdelset(&blk_set, SIGALRM);
    
    // 시그널 액션 구조체 초기화
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sig_act.sa_handler = ssu_signal_handler;

    // SIGALRM 시그널에 대한 핸들러 등록
    sigaction(SIGALRM, &sig_act, NULL);

    // 현재 시간 출력
    ssu_timestamp("before sigsuspend()");

    // 5초 후에 SIGALRM 시그널이 발생하도록 설정
    alarm(5);

    // SIGALRM을 제외한 다른 모든 시그널을 블록하고 대기
    sigsuspend(&blk_set);

    // sigsuspend 종료 후 현재 시간 출력
    ssu_timestamp("after sigsuspend()");

    exit(0);
}

// SIGALRM 시그널 핸들러 함수
void ssu_signal_handler(int signo) {
    printf("in ssu_signal_handler() function\n");
}

// 현재 시간을 출력하는 함수
void ssu_timestamp(char *str) {
    time_t time_val;

    // 현재 시간을 구하고 출력
    time(&time_val);
    printf("%s the time is %s\n", str, ctime(&time_val));
}
