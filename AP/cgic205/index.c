/* Change this if the SERVER_NAME environment variable does not report
	the true name of your web server. */
#if 1
#define SERVER_NAME cgiServerName
#endif
#if 0
#define SERVER_NAME "www.boutell.com"
#endif

/* You may need to change this, particularly under Windows;
	it is a reasonable guess as to an acceptable place to
	store a saved environment in order to test that feature. 
	If that feature is not important to you, you needn't
	concern yourself with this. */

#ifdef WIN32
#define SAVED_ENVIRONMENT "c:\\cgicsave.env"
#else
#define SAVED_ENVIRONMENT "/tmp/cgicsave.env"
#endif /* WIN32 */

#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

void ShowForm();
void Process();
int finished = 0;
char strURL[50];
/*
	login web for NetIndex 
	written by vance 2008.12.11
*/
int cgiMain() {
	/* Send the content type, letting the browser know this is HTML */
	cgiHeaderContentType("text/html");
	/* If a submit button has already been clicked, act on the submission of the form. */
	fprintf(cgiOut, "<html><head>\n");
	fflush(cgiOut);
    	fprintf(cgiOut, "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n");
	fflush(cgiOut);
    	fprintf(cgiOut, "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n");
	fflush(cgiOut);
    	fprintf(cgiOut, "<meta http-equiv=\"Expires\" content=\"-1\">\n");
	fflush(cgiOut);
    	fprintf(cgiOut, "<link rel=\"stylesheet\" href=\"/file/set.css\"> \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"/file/javascript.js\"></script> \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"/file/multilanguage.var\"></script> \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<title></title> \n");
	fflush(cgiOut);
   	fprintf(cgiOut, "<script> \n");
	fflush(cgiOut);
   	fprintf(cgiOut, "function copyto() \n");
	fflush(cgiOut);
	fprintf(cgiOut, "{document.login.url.value=location.href;} \n");
	fflush(cgiOut);
      fprintf(cgiOut, "function saveChanges() \n");
	fflush(cgiOut);
      fprintf(cgiOut, "{if ( includeSpace(document.login.pass.value)){ \n");
	fflush(cgiOut);
	fprintf(cgiOut, "alert(showText(PasswordNoSpace)); \n");
	fflush(cgiOut);
      fprintf(cgiOut, "document.login.pass.focus(); \n");
	fflush(cgiOut);
      fprintf(cgiOut, "return false;} \n");
	fflush(cgiOut);
      fprintf(cgiOut, "  return true;} \n");
	fflush(cgiOut);
	fprintf(cgiOut, "</script></head> \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<body class=\"mainbg\"> \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<blockquote> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<br><a class=\"titlecolor\">Login</a><br><br> \n");
	fflush(cgiOut);
	if (cgiFormSubmitClicked("save") == cgiFormSuccess)
	{
		Process();
	}
	else
	{
		/* Now show the form */
		ShowForm();
		/* Finish up the page */
	}
	if(finished == 1)
	{
		//redirect web
		fprintf(cgiOut, "<script>window.location=\"%s\"</script>",strURL);
		fflush(cgiOut);
		//cgiHeaderLocation(strURL);
	}
	fprintf(cgiOut, "</blockquote></body></html> \n");
	fflush(cgiOut);
	return 0;
}
	
void Process(){
	char strName[20],strPass[20];
	char command[50];
	if(cgiFormString("name",strName,20) != cgiFormSuccess)
	{
		fprintf(cgiOut, "name error!!<br/>");
		fflush(cgiOut);
	}
	if(cgiFormString("pass",strPass,20) != cgiFormSuccess)
	{
		fprintf(cgiOut, "pass error!!<br/>");
		fflush(cgiOut);
	}
	if(cgiFormString("url",strURL,50) != cgiFormSuccess)
	{
		fprintf(cgiOut, "url error!!<br/>");
		fflush(cgiOut);
	}
	//fprintf(cgiOut,"name=%s,pass=%s,url=%s,IP=%s</br>\n",strName,strPass,strURL,cgiRemoteAddr);
	//fflush(cgiOut);
	//get user name and password in flash
	FILE *fptr;
	char ch;
	char buf[2][10];
	fptr = fopen("/var/login","r");

	if(fptr != NULL)	
	{
		int i=0,j=0;
		 while(!feof(fptr))
		 {
		   ch=getc(fptr);
			if(ch != '\n')
			{
				buf[i][j]=ch;
				j++;
			}
			else 
			{
				buf[i][j]='\0';
				i++;
				j=0;
			}
		}
	}
	else
		printf("no file.\n");
		
	fclose(fptr);	

	if ( strName[0] || strPass[0])//name and password can't be empty
	{
		if ( !strcmp(strName,buf[0]) && !strcmp(strPass,buf[1]) ) {
			fprintf(cgiOut, "login successfully!!\n");
			fflush(cgiOut);
			//add mac to iptables for passing through 3g network
			strcpy(command,"/bin/scriptlib_util.sh setPassthroughLanToWan ");
			strcat(command,cgiRemoteAddr);
			//fprintf(cgiOut, "%s\n",command);
			//fprintf(cgiOut, "%s\n",command);
			//fflush(cgiOut);
			system(command);
			finished=1;
		}
		else{
			fprintf(cgiOut, "Login fail\n");
			fflush(cgiOut);
		}
	}
	else
	{
		fprintf(cgiOut, "Login fail\n");
		fflush(cgiOut);
	}
}

void ShowForm()
{
	fprintf(cgiOut, "<form  \n");
	fflush(cgiOut);
	fprintf(cgiOut, "action=\"");
	fflush(cgiOut);
	cgiValueEscape(cgiScriptName);
	fflush(cgiOut);
	fprintf(cgiOut, "\" method=POST name=\"login\"> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<table border=\"0\" cellspacing=\"1\" cellpadding=\"0\" width=\"520\"> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<tr><td class=\"table1\" width=\"35%%\"><script>dw(UserName)</script>&nbsp;:&nbsp;</td> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<td class=\"table2\" width=\"65%%\">&nbsp;<input type=\"input\" name=\"name\" size=\"20\" maxlength=\"30\" value=\"\" class=\"text\"></td></tr> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<tr><td class=\"table1\"><script>dw(Password)</script>&nbsp;:&nbsp;</td> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<td class=\"table2\">&nbsp;<input type=\"password\" name=\"pass\" size=\"20\" maxlength=\"30\" class=\"text\"></td></tr> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "</table><br><table width=\"520\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\"> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<tr><td align=\"right\"><script> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "document.write(\'<input type=submit value=\"'+showText(APPLY)+'\" name=\"save\" class=\"button\" onClick=\"return saveChanges()\">&nbsp;&nbsp;\'); \n");
	fflush(cgiOut);
      fprintf(cgiOut, "document.write(\'<input type=button value=\"'+showText(CANCEL)+'\" class=\"button\" onClick=\"document.login.reset();\">\'); \n");
	fflush(cgiOut);
      fprintf(cgiOut, "</script> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "<input type=hidden value=\"\" name=\"url\"> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "</td></tr></table>   \n");
	fflush(cgiOut);
	fprintf(cgiOut, "<script>copyto();</script> \n");
	fflush(cgiOut);
      fprintf(cgiOut, "</form> \n");
	fflush(cgiOut);
}
