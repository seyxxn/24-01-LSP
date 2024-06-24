#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

// 시그널 핸들러 함수 선언
void ssu_signal_handler(int signo);

// 전역 점프 버퍼 선언
jmp_buf jump_buffer;

int main(void) {
    // SIGINT 시그널에 대해 ssu_signal_handler를 핸들러로 설정
    signal(SIGINT, ssu_signal_handler);

    // 무한 루프
    while (1) {
        // setjmp로 점프 지점 설정
        if (setjmp(jump_buffer) == 0) {
            // 초기 호출 시 메시지 출력 및 pause로 대기
            printf("Hit Ctrl-c at anytime ... \n");
            pause();
        }
    }

    exit(0);
}

// 시그널 핸들러 함수 정의
void ssu_signal_handler(int signo) {
    char character;

    // SIGINT 시그널을 일시적으로 무시
    signal(signo, SIG_IGN);
    printf("Did you hit Ctrl-c?\n" "Do you really want to quit? [y/n] ");
    // 사용자 입력 받기
    character = getchar();

    // 사용자가 'y' 또는 'Y'를 입력하면 프로그램 종료
    if (character == 'y' || character == 'Y')
        exit(0);
    else {
        // 그렇지 않으면 SIGINT 시그널 핸들러를 다시 설정하고 longjmp로 점프
        signal(SIGINT, ssu_signal_handler);
        longjmp(jump_buffer, 1);
    }
}
