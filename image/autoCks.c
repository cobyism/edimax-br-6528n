#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	char inFile[80]={0};
	unsigned short sum=0;
	unsigned char tmpBuf[100];
	FILE *fp;

	if ( argc != 2 ) {
		printf("ERROR: Input argument's number is wrong!");
		exit(1);
	}
	else
		sscanf(argv[1], "%s", inFile);
	
	
	if ((fp = fopen(inFile, "r")) == NULL) {
		printf("ERROR: Can't open file %s\n!", inFile);
		exit(1);
	}
	while (fread(tmpBuf, 1, 1, fp) != 0) {
		sum += tmpBuf[0];
	}
	printf("%s      checksum=%04X\n", inFile, sum);
	exit(1);
		
	
}
