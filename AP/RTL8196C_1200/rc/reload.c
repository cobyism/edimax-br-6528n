#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if 0
#define DEBUG_PRINT(fmt, args...)	printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)	{}
#endif

int main(int argc, char *argv[])
{
	FILE *fp;
	int i, rf_switch_stat;

	fp = fopen("/proc/rf_switch", "r");
	fscanf(fp, "%d", &rf_switch_stat);
	fclose(fp);

#if defined(_PCI_THREEWAY_SWITCH_) && defined(_BR6428GNL_)
	int last_status=-1;
#endif

	while(1)
	{
#if defined(_PCI_THREEWAY_SWITCH_) && defined(_BR6428GNL_)
		fp = fopen("/proc/pci_threeway_switch", "r");
		fscanf(fp, "%d", &i);
		fclose(fp);

		if( last_status != -1 && last_status != i )
		{
			if ( ( fp = fopen("/tmp/initing", "r") ) == NULL )
			{
				system("> /tmp/initing");
				last_status=i;
				system("init.sh gw all");
				system("rm -rf /tmp/initing");
			}
			else
				fclose(fp);
		}
		else
			last_status=i;
#endif

		fp = fopen("/proc/load_default", "r");
		fscanf(fp, "%d", &i);
		if (i == 1) {
			printf("Going to Reload Default\n");
			system("flash default"); // EDX BSP, RexHua
			system("reboot");
		}
		fclose(fp);

		fp = fopen("/proc/wps_but", "r");
		fscanf(fp, "%d", &i);
		if (i == 1) {
			system("killall wps.sh 2> /dev/null");
			system("/bin/wps.sh 2 0 1 &");
			system("echo 0 > /proc/wps_but ");
		}
		fclose(fp);
		


		// Detect RF_SWITCH
		fp = fopen("/proc/rf_switch", "r");
		fscanf(fp, "%d", &i);
		if (i != rf_switch_stat){
		#if defined(_RADIO_ON_OFF_) 
		//radio on & radio off do not run init.sh
		fopen("/proc/rf_switch", "r");
		fscanf(fp, "%d", &rf_switch_stat);
		fclose(fp);
		if(i == 0){
		system("./bin/radio_on_off.sh up");	
		}else{
		system("./bin/radio_on_off.sh down");	
		}
		/*
			fopen("/proc/rf_switch", "r");
			fscanf(fp, "%d", &rf_switch_stat);
			fclose(fp);
			system("init_radio_on_off.sh gw all");
		*/
		#else	
			system("reboot.sh");
		#endif
		}
		fclose(fp);




		#ifdef _ESD_DETECT_
		fp = fopen("/proc/esd_detect", "r");
		if (fp != NULL)
		{
			fscanf(fp, "%d", &i);
			if (i == 1) {
				printf("ESD DETECT!\n");
				system("reboot.sh");
			}
			fclose(fp);
		}
		#endif
		sleep(2);
	}
}
