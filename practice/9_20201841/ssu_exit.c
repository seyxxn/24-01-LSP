#include <stdio.h>
#include <stdlib.h>

int main(void){
	printf("Good afternoon");
	exit(0);
	// exit() 함수를 호출하여 강제 종료 -> 표준 입출력 처리를 모두 마치고 _exit()를 부르기 때문에
	// 출력 O
}
