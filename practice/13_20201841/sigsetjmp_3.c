#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

// 함수 프로토타입 선언
static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

// 전역 변수로 점프 버퍼 선언
sigjmp_buf jmp_buf1;
sigjmp_buf jmp_buf2;

int main(void){
    struct sigaction act_sig1;
    struct sigaction act_sig2;
    int i, ret;

    // 현재 프로세스 ID 출력
    printf("My PID is %d\n", getpid());

    // 첫 번째 점프 버퍼 설정
    ret = sigsetjmp(jmp_buf1, 1);

    if (ret == 0) {
        // sigsetjmp에서 처음 호출된 경우
        act_sig1.sa_handler = ssu_signal_handler1;
        sigaction(SIGINT, &act_sig1, NULL); // SIGINT에 대한 핸들러 설정
    }
    else if (ret == 3){
        // SIGINT로 인해 siglongjmp로 돌아온 경우
        printf("----------------\n");
    }

    printf("Starting\n");

    // 두 번째 점프 버퍼 설정
    sigsetjmp(jmp_buf2, 2);
    act_sig2.sa_handler = ssu_signal_handler2;
    sigaction(SIGUSR1, &act_sig2, NULL); // SIGUSR1에 대한 핸들러 설정

    // 20번 반복하면서 1초마다 i를 출력
    for(i = 0; i < 20; i++) {
        printf("i = %d\n", i);
        sleep(1);
    }

    exit(0);
}

// SIGINT 신호에 대한 핸들러
static void ssu_signal_handler1(int signo) {
    fprintf(stderr, "\nInterrupted\n"); // 인터럽트 메시지 출력
    siglongjmp(jmp_buf1, 3); // 첫 번째 점프 버퍼로 점프, 3을 반환값으로 설정
}

// SIGUSR1 신호에 대한 핸들러
static void ssu_signal_handler2(int signo) {
    fprintf(stderr, "\nSIGUSR1\n"); // SIGUSR1 메시지 출력
    siglongjmp(jmp_buf2, 2); // 두 번째 점프 버퍼로 점프, 2를 반환값으로 설정
}
