#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // 프로그램이 강제 종료될 것임을 알리는 메시지를 출력
    printf("abort terminate this program\n");
    // 프로그램을 즉시 강제 종료. 이후의 코드는 실행되지 않음
    abort();
    // 이 줄은 절대 실행되지 않음
    printf("this line is never reached\n");
    // 정상적인 프로그램 종료. 그러나 이 코드는 실행되지 않음
    exit(0);
}
