#include <stdio.h>
#include <dirent.h>
#include <string.h>

char working_dir[200];

char *clean_javascript(char *input)
{
	while( isspace(*input) ) input++;
	while( isspace(*(input+strlen(input)-1)) && *(input+strlen(input)-1)!='\n' ) *(input+strlen(input)-1)='\0';
	if( *input=='#' && *(input+1)!='!' ) *input='\0';
	return input;
}


char *clean_html(char *input)
{
	while( isspace(*input) ) input++;
	while( isspace(*(input+strlen(input)-1)) && *(input+strlen(input)-1)!='\n' ) *(input+strlen(input)-1)='\0';
	//if( *input=='#' && *(input+1)!='!' ) *input='\0';
	return input;
}


void clean(char *path, char *extension, char *type)
{
	char pathname[200], filename[200];
	char tempstr[1000];
	FILE *file, *file_temp;
	DIR *dir;
	struct dirent *ptr;

	sprintf(pathname, "%s/%s", working_dir, path);

	if( (dir=opendir(pathname)) != NULL )
	{
		while( (ptr = readdir(dir))!=NULL )
		{
			sprintf(filename,"%s/%s",pathname, ptr->d_name);
			sprintf(tempstr, ".%s", extension);
			if( strlen(ptr->d_name)>3 && !strcmp( ptr->d_name+strlen(ptr->d_name)-strlen(tempstr), tempstr ) && (file=fopen(filename, "r")) != NULL && (file_temp=fopen("/etc/clean_tmp", "w")) != NULL )
			{
				while( fgets(tempstr, sizeof(tempstr), file)>0 )
				{
					if(!strcmp( type, "shellscript"))
						fputs(clean_javascript(tempstr), file_temp);
					else if(!strcmp( type, "html"))
						fputs(clean_html(tempstr), file_temp);
				}
				fclose(file_temp);
				fclose(file);

				if(  (file=fopen(filename, "w"))!=NULL  &&  (file_temp=fopen("/etc/clean_tmp", "r")) != NULL  )
				{
					while( fgets(tempstr, sizeof(tempstr), file_temp)>0 )
					{
						fputs(tempstr, file);
					}
					fclose(file_temp);
					fclose(file);
				}
			}
		}
		closedir(dir);
	}
}

int main(int argc, char *argv[])
{
	char full_name[200];
	int i;

	if(argc<2)
	{
		printf("Usage: %s <Romfs DIR>\n", argv[0]);
		return 0;
	}

	sprintf(working_dir, "%s", argv[1]);

	while( working_dir[strlen(working_dir)-1] == '/' ) working_dir[strlen(working_dir)-1]='\0';

	clean("bin", "sh", "shellscript");
	clean("web", "asp", "html");

	return 0;
}
