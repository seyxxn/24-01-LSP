#include <stdio.h>
#include <stdlib.h>

int main(void){
	printf("Good afternoon?");
	_exit(0);
	// _exit() 함수를 호출하여 강제종료 -> 이때 표준 입출력이 다 날라감
	// 출력 X
}
