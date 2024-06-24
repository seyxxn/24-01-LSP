#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(void){
	char buf[BUFFER_SIZE];
	char *fname = "ssu_test.txt";
	int count;
	int fd1, fd2;
    
    // open 함수를 이용하여 fd1에 파일 디스크립터 반환받음
	if ((fd1 = open(fname, O_RDONLY, 0644)) < 0) { 
		fprintf(stderr, "open error for %s\n", fname); 
		exit(1); 
	}
	fd2 = dup(fd1); // fd1의 파일 디스크립터를 복사하여 fd2에 저장
	count = read(fd1, buf, 12);
	// read 함수를 이용하여 fd1을 buf로 12byte 읽어와서 읽은 바이트 수를 count 변수에 저장
	buf[count] = 0;
	// 읽은 문자들을 가지고 있는 buf 배열의 count 인덱스에 해당하는 문자(맨 뒤)에 0을 추가하여 문자열의 끝을 표현
	printf("fd1's printf : %s\n", buf); // 읽어온 buf를 확인하기 위해 출력
	lseek(fd1, 1, SEEK_CUR);
	// fd1의 파일 커서 위치를 현재 위치한 커서에서 1칸 이동 (read한 문자열을 가리키고 있다가 1칸 오른쪽으로 이동)
	count = read(fd2, buf, 12);
	// read 함수를 이용하여 fd2를 buf로 12byte 읽어와서 읽은 바이트 수를 count 변수에 저장
	buf[count] = 0;
	// 읽은 문자들을 가지고 있는 buf 배열의 count 인덱스에 해당하는 문자(맨 뒤)에 0을 추가하여 문자열의 끝을 표현
	printf("fd2's printf : %s\n", buf); // 읽어온 buf를 확인하기 위해 출력
	// 출력 결과 fd2에서 읽은 데이터는 fd1에서 읽은 데이터의 뒷 부분이 나옴
	// 같은 프로세스에서 파일 디스크립터의 번호가 다를 뿐, dup()를 호출하면 원본이 되는 파일 디스크립터와 오프셋을 공유함을 알 수 있음
	exit(0);
}
