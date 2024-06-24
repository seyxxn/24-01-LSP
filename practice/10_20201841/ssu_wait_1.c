#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void ssu_echo_exit(int status);

int main(void) {
    pid_t pid;
    int status;

    // 첫 번째 자식 프로세스 생성
    if ((pid = fork()) < 0) { // fork 실패 시
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (pid == 0) { // 자식 프로세스에서 실행할 코드
        exit(7); // 종료 상태를 7로 설정하고 종료
    }

    // 첫 번째 자식 프로세스의 종료를 기다림
    if (wait(&status) != pid) { // 자식 프로세스의 종료 상태를 기다리고 그 PID가 반환되지 않을 경우
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    // 종료 상태 출력 함수 호출
    ssu_echo_exit(status);

    // 두 번째 자식 프로세스 생성
    if ((pid = fork()) < 0) { // fork 실패 시
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (pid == 0) { // 자식 프로세스에서 실행할 코드
        abort(); // 프로세스를 비정상적으로 종료
    }

    // 두 번째 자식 프로세스의 종료를 기다림
    if (wait(&status) != pid) { // 자식 프로세스의 종료 상태를 기다리고 그 PID가 반환되지 않을 경우
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    // 종료 상태 출력 함수 호출
    ssu_echo_exit(status);

    // 세 번째 자식 프로세스 생성
    if ((pid = fork()) < 0) { // fork 실패 시
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (pid == 0) { // 자식 프로세스에서 실행할 코드
        status /= 0; // 0으로 나누어 예외 발생
    }

    // 세 번째 자식 프로세스의 종료를 기다림
    if (wait(&status) != pid) { // 자식 프로세스의 종료 상태를 기다리고 그 PID가 반환되지 않을 경우
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    // 종료 상태 출력 함수 호출
    ssu_echo_exit(status);

    exit(0); // 프로그램 종료
}

// 종료 상태를 출력하는 함수 정의
void ssu_echo_exit(int status) {
    if (WIFEXITED(status)) // 정상 종료인 경우
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); // 종료 상태 출력
    else if (WIFSIGNALED(status)) // 시그널에 의한 종료인 경우
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
               WCOREDUMP(status) ? " (core file generated)" : ""); // 코어 덤프 여부 출력
#else
               "");
#endif
    else if (WIFSTOPPED(status)) // 중지된 경우
        printf("child stopped, signal number = %d\n", WSTOPSIG(status)); // 중지된 시그널 번호 출력
}
