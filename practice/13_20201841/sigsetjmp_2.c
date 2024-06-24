#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

// 함수 프로토타입 선언
static void ssu_alarm(int signo);
static void ssu_func(int signo);
void ssu_mask(const char *str);

// 전역 변수
static volatile sig_atomic_t can_jump; // 점프가 가능한지 확인하는 플래그
static sigjmp_buf jump_buf; // longjmp를 위해 호출 환경을 저장하는 버퍼

int main(void) {
    // SIGUSR1 신호에 대한 핸들러 설정
    if (signal(SIGUSR1, ssu_func) == SIG_ERR) {
        fprintf(stderr, "SIGUSR1 error");
        exit(1);
    }

    // SIGALRM 신호에 대한 핸들러 설정
    if (signal(SIGALRM, ssu_alarm) == SIG_ERR) {
        fprintf(stderr, "SIGALRM error");
        exit(1);
    }

    // 초기 신호 마스크를 출력
    ssu_mask("starting main: ");

    // 나중에 longjmp로 돌아올 호출 환경을 저장
    if (sigsetjmp(jump_buf, 1)) {
        // siglongjmp로 돌아올 때, 신호 마스크를 출력하고 종료
        ssu_mask("ending main: ");
        exit(0);
    }

    can_jump = 1; // 점프가 가능함을 나타내는 플래그 설정

    // 신호를 기다리는 무한 루프
    while (1)
        pause();

    exit(0);
}

// 현재 신호 마스크를 출력하는 함수
void ssu_mask(const char *str) {
    sigset_t sig_set;
    int err_num;

    err_num = errno; // 현재 errno를 저장

    // 현재 신호 마스크를 가져옴
    if (sigprocmask(0, NULL, &sig_set) < 0) {
        printf("sigprocmask() error");
        exit(1);
    }

    printf("%s", str);

    // 어떤 신호가 차단되어 있는지 확인하고 출력
    if (sigismember(&sig_set, SIGINT))
        printf("SIGINT ");

    if (sigismember(&sig_set, SIGQUIT))
        printf("SIGQUIT ");

    if (sigismember(&sig_set, SIGUSR1))
        printf("SIGUSR1 ");

    if (sigismember(&sig_set, SIGALRM))
        printf("SIGALRM ");

    printf("\n");
    errno = err_num; // errno를 복원
}

// SIGUSR1 신호에 대한 핸들러
static void ssu_func(int signo){
    time_t start_time;

    // 점프가 허용되지 않은 경우 반환
    if (can_jump == 0)
        return;

    // 신호 마스크를 출력하고 알람 설정
    ssu_mask("starting ssu_func: ");
    alarm(3);
    start_time = time(NULL);

    // 5초 동안 바쁜 대기
    while (1)
        if (time(NULL) > start_time + 5)
            break;

    // 신호 마스크를 출력하고, 플래그를 재설정하며, 저장된 환경으로 점프
    ssu_mask("ending ssu_func: ");
    can_jump = 0;
    siglongjmp(jump_buf, 1);
}

// SIGALRM 신호에 대한 핸들러
static void ssu_alarm(int signo){
    ssu_mask("in ssu_alarm");
}
