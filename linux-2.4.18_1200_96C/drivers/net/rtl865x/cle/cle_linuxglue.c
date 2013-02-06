//This file is Linux dependent
#ifndef __linux__
#error "This file must be compiled under Linux"
#endif
#include "../rtl865x/types.h"

//#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>

#include "cle_struct.h"
#define CLEBUF		1024
#define CMDBUFLEN	128
#define CLE_MORE		"more"
#define CLE_OUT		"out"

typedef int (*cle_printfun_t)(int, int);

cle_printfun_t  __cle_callback_p;



static char *cle_buf;
static char outbuf[CLEBUF];
static char prompt[CMDBUFLEN >> 2];
static int cle_start;
static int cle_end;


#if 0
static int cle_flushout(char *page, char *tail)
{
	int size;
	char *sc;

	/*
	  * copy the content of cle_buf to the /proc/rtcle file buffer
	  */
	for(sc=cle_buf; *sc != '\0'; ++sc)
		; /* do nothing */
	size = sc - cle_buf;
	memcpy(page, cle_buf, size);

	/* append tail */
	for(sc=(page+size+1); *tail != '\0'; *sc=*tail, sc++, tail++)
		; /* do nothing */
	
	return size;
}
#endif
static int cle_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{
	int size=0;

	cle_buf = page;

	if (*outbuf != 0 || *prompt != 0) {
		size = sprintf(page, "%s\t%s", prompt, outbuf);
		*outbuf = 0;
		*prompt = 0;
	}
	
	cle_buf = NULL;
	return size;
}


static int cle_write(struct file *file,  char *ubuf, 
		unsigned long count, void *data)
{
	unsigned int outLen;

	/* DO special thing */
	switch (ubuf[count-2]) {

	case '\n': /* Get Prompt Only */
		break;
		
	case '\t': /* Tab */
		outLen = CMDBUFLEN;
		ubuf[count-1] = 0;
		cle_setTabMode(0, 1);
		cle_parser(0, ubuf, outbuf, &outLen);
		cle_setTabMode(0, 0);
		break;
		
	case '?':
		outLen = CMDBUFLEN;
		ubuf[count-1] = '\0';
		cle_parser(0, ubuf, outbuf, &outLen);
		break;
		
	default: /* parsing input */

		ubuf[count-1] = 0;
		outLen = CMDBUFLEN;
		cle_parser(0, ubuf, outbuf, &outLen);
		*outbuf = 0;
		
	}

	/* Get UI prompt */
	cle_getPrompt(0, prompt, (CMDBUFLEN >> 2));
	
	return 1;
}






#define CLE_FS_ATTR		(S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP|S_IROTH|S_IWOTH)
#define CLE_INIT_SUCCESS	0
#if CONFIG_RTL865X_MODULE_ROMEDRV
void __init rtlcle_init (void)
#else
static void __init rtlcle_init (void)
#endif
{
	struct proc_dir_entry *cle;

	/* Initialize CLE Module */
	if (cle_init(1, 64, 64) != CLE_INIT_SUCCESS) {
		rtlglue_printf("rtlcle: cle module initial fail !\n");
		return;
	}
		
	/* Initialize CLE Root Directory */
	if(CLE_userIdError==cle_initUser("865x", CLE_UI_STYLE_STANDARD_IO, CLE_MODE_PRIVILEGE|CLE_MODE_SUPERENG, 4, cle_newCmdRoot)){
		rtlglue_printf("rtlcle: cle user init failed !\n");
		return;
	}
	/* Create a proc file */
	if ((cle=create_proc_entry("rtcle", CLE_FS_ATTR, &proc_root)) == NULL) {
		rtlglue_printf("rtlcle: cannot create file /proc/rtcle !\n");
		return;
	}
	cle->read_proc  = cle_read;
	cle->write_proc = cle_write;

	//*cle_buf = '\0';
	cle_buf = NULL;
	__cle_callback_p = NULL;
	/* if cle_start > cle_end, no information */
	cle_start = 1;  cle_end = 0;
	
}
#if CONFIG_RTL865X_MODULE_ROMEDRV
static void __exit rtlcle_exit(void)
#else
void __exit rtlcle_exit(void)
#endif
{
	remove_proc_entry("rtcle", NULL);
}

#if CONFIG_RTL865X_MODULE_ROMEDRV
#else
module_init(rtlcle_init);
module_exit(rtlcle_exit);

#endif
