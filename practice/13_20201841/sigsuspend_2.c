#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

// SIGUSR1 신호 핸들러
static void ssu_func(int signo);

// 현재의 시그널 마스크를 출력하는 함수
void ssu_print_mask(const char *str);

int main(void)
{
    // 변수 선언
    sigset_t new_mask, old_mask, wait_mask;

    // 프로그램 시작 시 시그널 마스크 출력
    ssu_print_mask("program start: ");

    // SIGINT에 대한 시그널 핸들러 설정
    if (signal(SIGINT, ssu_func) == SIG_ERR) {
        fprintf(stderr, "signal(SIGINT) error\n");
        exit(1);
    }

    // wait_mask를 초기화하고 SIGUSR1을 추가
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGUSR1);

    // new_mask를 초기화하고 SIGINT를 추가하여 블록
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGINT);

    // 현재 시그널 마스크를 new_mask로 변경하고 이전 마스크(old_mask)를 저장
    if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0) {
        fprintf(stderr, "SIG_BLOCK() error\n");
        exit(1);
    }

    // 크리티컬 영역 진입 시 시그널 마스크 출력
    ssu_print_mask("in critical region: ");

    // SIGUSR1 시그널이 올 때까지 대기
    if (sigsuspend(&wait_mask) != -1) {
        fprintf(stderr, "sigsuspend() error\n");
        exit(1);
    }

    // sigsuspend에서 리턴 후 시그널 마스크 출력
    ssu_print_mask("after return from sigsuspend: ");

    // 이전 시그널 마스크(old_mask)로 변경
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) {
        fprintf(stderr, "SIG_SETMASK() error\n");
        exit(1);
    }

    // 프로그램 종료 시 시그널 마스크 출력
    ssu_print_mask("program exit: ");
    exit(0);
}

// 현재의 시그널 마스크를 출력하는 함수
void ssu_print_mask(const char *str) {
    sigset_t sig_set;
    int err_num;

    err_num = errno;

    // 현재 시그널 마스크를 가져와서 출력
    if (sigprocmask(0, NULL, &sig_set) < 0) {
        fprintf(stderr, "sigprocmask() error\n");
        exit(1);
    }

    printf("%s", str);

    if (sigismember(&sig_set, SIGINT))
        printf("SIGINT ");

    if (sigismember(&sig_set, SIGQUIT))
        printf("SIGQUIT ");

    if (sigismember(&sig_set, SIGUSR1))
        printf("SIGUSR1 ");

    if (sigismember(&sig_set, SIGALRM))
        printf("SIGALRM ");

    printf("\n");
    errno = err_num;
}

// SIGUSR1 시그널 핸들러
static void ssu_func(int signo) {
    ssu_print_mask("\nin ssu_func: ");
}
