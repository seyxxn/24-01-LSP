#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("this is the original program\n"); // 프로그램 시작을 알리는 메시지 출력
    execl("./ssu_execl_test_1", "ssu_execl_test_1", "param1", "param2", "param3", (char *)0);
    // ssu_execl_test_1 프로그램을 실행하고자 함. 실행 파일 이름과 전달할 인자들을 지정
    printf("%s\n", "this line should never get printed\n"); // execl이 성공적으로 실행되면 이 코드는 실행되지 않음
    exit(0); // 프로그램 종료
}
