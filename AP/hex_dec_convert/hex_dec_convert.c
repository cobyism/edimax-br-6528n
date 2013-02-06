/**************************************************************************
* Auth: Kyle
* Date: 2009/02/25
 ***************************************************************************
 */

#include <stdio.h>             
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void usage()
{
	printf("hex_dec_covert [hex2dec or dec2hex] value\n");
	exit(0);
}
int main(int argc, char *argv[])
{
	  if(argc <= 2){
		  usage();
	   return 0;
	  }
	  if (!strcmp(argv[1], "hex2dec")){
		 printf("%d\n",strtol(argv[2],NULL,16));
	  }
	  else if(!strcmp(argv[1], "dec2hex")){
		  printf("%x\n",strtol(argv[2],NULL,10));
	  }
	  else{
		 usage();
	  	return 0;
	  }
}

