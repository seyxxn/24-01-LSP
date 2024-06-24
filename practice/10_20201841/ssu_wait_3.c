#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    // 첫 번째 자식 프로세스 생성 후 "echo" 명령어 실행
    if (fork() == 0)
        execl("/bin/echo", "echo", "this is", "message one", (char *)0);

    // 두 번째 자식 프로세스 생성 후 "echo" 명령어 실행
    if (fork() == 0)
        execl("/bin/echo", "echo", "this is", "message Two", (char *)0);

    printf("parent: waiting for children\n"); // 부모 프로세스에서 출력: 자식 프로세스 종료 대기 중

    // 자식 프로세스의 종료를 기다림
    while(wait((int*)0) != -1);

    printf("parent: all children terminated\n"); // 부모 프로세스에서 출력: 모든 자식 프로세스 종료됨
    exit(0); // 프로그램 종료
}
