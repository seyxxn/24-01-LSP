#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define EXIT_CODE 1

int main(void) {
    pid_t pid;
    int ret_val, status;

    // 자식 프로세스 생성
    if ((pid = fork()) == 0) { // 자식 프로세스에서는 fork()의 반환값이 0이므로 if문 내부 실행
        printf("child: pid = %d ppid = %d exit_code = %d\n", getpid(), getppid(), EXIT_CODE); // 자식 프로세스에서 출력: 자식의 PID, 부모의 PID, 종료 코드 출력
        exit(EXIT_CODE); // 자식 프로세스 종료
    }

    printf("parent: waiting for child = %d\n", pid); // 부모 프로세스에서 출력: 자식 프로세스의 PID 대기 중
    ret_val = wait(&status); // 부모 프로세스는 자식 프로세스의 종료를 기다림
    printf("parent: return value = %d, ", ret_val); // 부모 프로세스에서 출력: wait 함수의 반환값 출력
    printf("child's status = %x", status); // 부모 프로세스에서 출력: 자식 프로세스의 종료 상태 출력
    printf(" and shifted = %x\n", (status >> 8)); // 부모 프로세스에서 출력: 자식 프로세스의 종료 상태를 비트 시프트하여 출력
    exit(0); // 프로그램 종료
}
