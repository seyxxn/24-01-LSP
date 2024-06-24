#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 시그널 핸들러 함수 정의
void ssu_signal_handler(int signo) {
    // 시그널 핸들러가 호출되었음을 알리는 메시지 출력
    printf("ssu_signal_handler control\n");
}

int main(void) {
    struct sigaction sig_act; // 시그널 액션 구조체 정의
    sigset_t sig_set; // 시그널 집합 정의

    // sig_act 구조체 초기화
    sigemptyset(&sig_act.sa_mask); // sa_mask를 비운다
    sig_act.sa_flags = 0; // 플래그 초기화
    sig_act.sa_handler = ssu_signal_handler; // 시그널 핸들러 설정

    // SIGUSR1 시그널에 대해 sig_act를 설정
    sigaction(SIGUSR1, &sig_act, NULL);

    // 첫 번째 kill() 호출 전 메시지 출력
    printf("before first kill()\n");
    
    // 현재 프로세스에 SIGUSR1 시그널을 보낸다
    kill(getpid(), SIGUSR1);
    
    // sig_set 집합 초기화
    sigemptyset(&sig_set);
    
    // sig_set에 SIGUSR1 시그널 추가
    sigaddset(&sig_set, SIGUSR1);
    
    // SIGUSR1 시그널을 블록(차단)하도록 시그널 마스크 설정
    sigprocmask(SIG_SETMASK, &sig_set, NULL);

    // 두 번째 kill() 호출 전 메시지 출력
    printf("before second kill()\n");
    
    // 현재 프로세스에 SIGUSR1 시그널을 보낸다 (하지만 차단되어 있음)
    kill(getpid(), SIGUSR1);
    
    // 두 번째 kill() 호출 후 메시지 출력
    printf("after second kill()\n");

    // 프로그램 정상 종료
    exit(0);
}
