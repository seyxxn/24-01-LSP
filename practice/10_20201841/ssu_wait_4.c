#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t child1, child2; // 변수 선언: 자식 프로세스의 PID를 저장할 변수
    int pid, status; // 변수 선언: 자식 프로세스의 종료 상태를 저장할 변수

    // 첫 번째 자식 프로세스 생성
    if ((child1 = fork()) == 0) // 자식 프로세스에서는 fork()의 반환값이 0이므로 if문 내부 실행
        execlp("date", "date", (char*)0); // date 명령어 실행

    // 두 번째 자식 프로세스 생성
    if ((child2 = fork()) == 0) // 자식 프로세스에서는 fork()의 반환값이 0이므로 if문 내부 실행
        execlp("who", "who", (char*)0); // who 명령어 실행

    printf("parent: waiting for children\n"); // 부모 프로세스에서 출력: 자식 프로세스 종료 대기 중

    // 자식 프로세스의 종료를 기다림
    while ((pid = wait(&status)) != -1) {
        if (child1 == pid) // 첫 번째 자식 프로세스 종료 시
            printf("parent: first child: %d\n", (status >> 8)); // 종료 상태 출력
        else if (child2 == pid) // 두 번째 자식 프로세스 종료 시
            printf("parent: second child %d\n", (status >> 8)); // 종료 상태 출력
    }

    printf("parent: all children terminated\n"); // 부모 프로세스에서 출력: 모든 자식 프로세스 종료됨
    exit(0); // 프로그램 종료
}
