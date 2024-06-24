#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char *argv[] = {
        "ssu_execl_test_1", "param1", "param2", (char *)0 // execv에 전달할 실행 파일 이름과 인자 목록
    };

    printf("this is the original program\n"); // 프로그램 시작을 알리는 메시지 출력
    execv("./ssu_execl_test_1", argv); // ssu_execl_test_1 프로그램을 새로운 프로세스로 실행
    printf("%s\n", "This line should never get printed\n"); // execv가 성공적으로 실행되었을 경우 이 코드는 실행되지 않음
    exit(0); // 프로그램 종료
}
