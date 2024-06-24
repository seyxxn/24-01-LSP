#include <stdio.h>
#include <stdlib.h>

int main(void) {
	char *fname = "ssu_test.txt";
	char *mode = "r";

    // fopen으로 fname으로 지정한 파일을 open함
	if (fopen(fname, mode) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}else {
		printf("Success!\nFilename: <%s>, mode <%s>\n", fname, mode);
	}

	exit(0);
}
