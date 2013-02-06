#include <stdio.h>
#include <stdlib.h>

void swapHL(unsigned char *buf, unsigned long size)
{
	unsigned long i;
	unsigned char temp;

	size = size - (size % 2);
	
	for(i=0; i < size; i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}
}


int main(int argc,char *argv[])
{
	FILE *inFp,*outFp;
	unsigned long rdSize = 0;
	unsigned char buf[1024];

	if(argc != 3)
	{
		printf("\nusage: swapHL inFile outFile \n\n");
		exit(1);
	}
	
 	/*===============open file===========*/
    if((inFp = fopen(argv[1],"rb")) == NULL)
	{
		printf("Open source file %s error\n",argv[1]);
		exit(1);
	}

  	//*===========write file============*//
	if((outFp = fopen(argv[2], "wb")) == NULL)
	{
		printf("Open write file %s error\n",argv[2]);
		exit(1);
	}


	while((rdSize = fread(buf, 1, 1024, inFp)) != 0)
	{
		swapHL(buf, rdSize);
		fwrite(buf, 1, rdSize, outFp);
	}
	
	fclose(inFp);
	fclose(outFp);
}
