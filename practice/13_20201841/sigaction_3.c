#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// SIGINT 시그널 핸들러 함수
static void ssu_signal_handler1(int signo);

// SIGQUIT 시그널 핸들러 함수
static void ssu_signal_handler2(int signo);

int main(void) {
    struct sigaction act_int, act_quit; // 시그널 액션 구조체 정의

    // SIGINT 시그널 핸들러 설정
    act_int.sa_handler = ssu_signal_handler1; // SIGINT 시그널에 대한 핸들러 설정
    sigemptyset(&act_int.sa_mask); // 시그널 마스크 초기화
    sigaddset(&act_int.sa_mask, SIGQUIT); // 시그널 마스크에 SIGQUIT 추가 (SIGINT 핸들러가 실행 중일 때 SIGQUIT 블록)
    act_quit.sa_flags = 0; // 플래그 초기화

    // SIGINT 시그널에 대한 액션 설정, 실패 시 에러 메시지 출력 후 종료
    if (sigaction(SIGINT, &act_int, NULL) < 0) {
        fprintf(stderr, "sigaction(SIGINT) error\n");
        exit(1);
    }

    // SIGQUIT 시그널 핸들러 설정
    act_quit.sa_handler = ssu_signal_handler2; // SIGQUIT 시그널에 대한 핸들러 설정
    sigemptyset(&act_quit.sa_mask); // 시그널 마스크 초기화
    sigaddset(&act_quit.sa_mask, SIGINT); // 시그널 마스크에 SIGINT 추가 (SIGQUIT 핸들러가 실행 중일 때 SIGINT 블록)
    act_int.sa_flags = 0; // 플래그 초기화

    // SIGQUIT 시그널에 대한 액션 설정, 실패 시 에러 메시지 출력 후 종료
    if (sigaction(SIGQUIT, &act_quit, NULL) < 0) {
        fprintf(stderr, "sigaction(SIGQUIT) error\n");
        exit(1);
    }

    // 시그널이 발생할 때까지 대기
    pause();
    exit(0);
}

// SIGINT 시그널 핸들러 함수 정의
static void ssu_signal_handler1(int signo) {
    printf("Signal handler of SIGINT : %d\n", signo);
    printf("SIGQUIT signal is blocked : %d\n", signo);
    printf("sleeping 3 sec\n");
    sleep(3); // 3초 대기
    printf("Signal handler of SIGINT ended\n");
}

// SIGQUIT 시그널 핸들러 함수 정의
static void ssu_signal_handler2(int signo) {
    printf("Signal handler of SIGQUIT : %d\n", signo);
    printf("SIGINT signal is blocked : %d\n", signo);
    printf("sleeping 3 sec\n");
    sleep(3); // 3초 대기
    printf("Signal handler of SIGQUIT ended\n");
}
