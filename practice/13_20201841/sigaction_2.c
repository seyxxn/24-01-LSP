#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 시그널이 대기 중인지 확인하는 함수
void ssu_check_pending(int signo, char *signame);

// 시그널 핸들러 함수 정의
void ssu_signal_handler(int signo);

int main(void) {
    struct sigaction sig_act; // 시그널 액션 구조체 정의
    sigset_t sig_set; // 시그널 집합 정의

    // sig_act 구조체 초기화
    sigemptyset(&sig_act.sa_mask); // sa_mask를 비운다
    sig_act.sa_flags = 0; // 플래그 초기화
    sig_act.sa_handler = ssu_signal_handler; // 시그널 핸들러 설정

    // SIGUSR1 시그널에 대해 sig_act를 설정, 실패 시 에러 메시지 출력 후 종료
    if (sigaction(SIGUSR1, &sig_act, NULL) != 0) {
        fprintf(stderr, "sigaction() error\n");
        exit(1);
    } else {
        // sig_set 집합 초기화 및 SIGUSR1 시그널 추가
        sigemptyset(&sig_set);
        sigaddset(&sig_set, SIGUSR1);

        // SIGUSR1 시그널을 블록(차단)하도록 시그널 마스크 설정, 실패 시 에러 메시지 출력 후 종료
        if (sigprocmask(SIG_SETMASK, &sig_set, NULL) != 0) {
            fprintf(stderr, "sigprocmask() error\n");
            exit(1);
        } else {
            // SIGUSR1 시그널이 블록되었음을 알리는 메시지 출력
            printf("SIGUSR1 signals are now blocked\n");

            // 현재 프로세스에 SIGUSR1 시그널을 보낸다
            kill(getpid(), SIGUSR1);
            printf("after kill()\n");

            // SIGUSR1 시그널이 대기 중인지 확인
            ssu_check_pending(SIGUSR1, "SIGUSR1");

            // 시그널 집합 초기화 및 시그널 마스크 해제
            sigemptyset(&sig_set);
            sigprocmask(SIG_SETMASK, &sig_set, NULL);

            // SIGUSR1 시그널이 더 이상 블록되지 않음을 알리는 메시지 출력
            printf("SIGUSR1 signals are no longer blocked\n");

            // SIGUSR1 시그널이 대기 중인지 다시 확인
            ssu_check_pending(SIGUSR1, "SIGUSR1");
        }
    }
    exit(0);
}

// 시그널이 대기 중인지 확인하는 함수
void ssu_check_pending(int signo, char *signame) {
    sigset_t sig_set; // 시그널 집합 정의

    // 현재 대기 중인 시그널들을 sig_set에 저장
    if (sigpending(&sig_set) != 0)
        printf("sigpending() error\n");
    else if (sigismember(&sig_set, signo))
        // sig_set에 signo 시그널이 포함되어 있으면 대기 중임을 알리는 메시지 출력
        printf("a %s signal is pending\n", signame);
    else
        // sig_set에 signo 시그널이 포함되어 있지 않으면 대기 중이지 않음을 알리는 메시지 출력
        printf("%s signals are not pending\n", signame);
}

// 시그널 핸들러 함수 정의
void ssu_signal_handler(int signo) {
    printf("in ssu_signal_handler function\n");
}
