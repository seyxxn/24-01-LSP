#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    // 첫 번째 자식 프로세스 생성
    if (fork() == 0) { // 자식 프로세스인 경우
        execl("/bin/echo", "echo", "this is", "message one", (char *)0); // echo 명령어를 실행하여 "this is message one" 출력
        fprintf(stderr, "exec error\n"); // execl 함수가 실패한 경우 에러 메시지 출력
        exit(1); // 자식 프로세스 종료
    }

    // 두 번째 자식 프로세스 생성
    if (fork() == 0) { // 자식 프로세스인 경우
        execl("/bin/echo", "echo", "this is", "message two", (char *)0); // echo 명령어를 실행하여 "this is message two" 출력
        fprintf(stderr, "exec error\n"); // execl 함수가 실패한 경우 에러 메시지 출력
        exit(1); // 자식 프로세스 종료
    }

    // 세 번째 자식 프로세스 생성
    if (fork() == 0) { // 자식 프로세스인 경우
        execl("/bin/echo", "echo", "this is", "message three", (char *)0); // echo 명령어를 실행하여 "this is message three" 출력
        fprintf(stderr, "exec error\n"); // execl 함수가 실패한 경우 에러 메시지 출력
        exit(1); // 자식 프로세스 종료
    }

    printf("Parent program ending\n"); // 부모 프로세스에서 출력되는 메시지
    exit(0); // 부모 프로세스 종료
}
