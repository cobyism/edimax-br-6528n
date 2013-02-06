
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static char *tmp_file = "/tmp/tmp.txt";


#if !(defined(CONFIG_RTL865X_SC) && defined(CONFIG_RTL865X_AC) && defined(CONFIG_RTL8186_TR) && defined(CONFIG_RTL8196C_EC) )
#include <time.h>

#if 0
#define DEBUG_PRINT(fmt, args...)	printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)	{}
#endif

struct ps_info {
	unsigned int fTime;
	unsigned int tTime;
	unsigned int date;
};

enum _WLAN_INTERFACE_ {
	WLAN_IF_ROOT,
	WLAN_IF_VXD,
	WLAN_IF_VA0,
	WLAN_IF_VA1,
	WLAN_IF_VA2,
	WLAN_IF_VA3,
	WLAN_IF_TOTAL
};

enum _ETH_INTERFACE_ {
	ETH_IF_0,
	ETH_IF_2,
	ETH_IF_3,
	ETH_IF_4,	
	ETH_IF_TOTAL
};

#if !defined(CONFIG_RTL865X_KLD)
static int pre_enable_state = -1;
#else
static int pre_enable_state[WLAN_IF_TOTAL];
static int pre_eth_enable_state[ETH_IF_TOTAL];
#endif
#endif

#if !defined(CONFIG_RTL865X_KLD)
#if defined(CONFIG_RTL8196B)
static int test_gpio(void)
{
	FILE *fp;
	int i;
	fp=fopen("/proc/rf_switch","r");
	fscanf(fp,"%d",&i);
	fclose(fp);
	return i;
}
#endif
#endif

static int get_flash_int_value(char *keyword, int *pVal)
{
	char tmpbuf[100], *ptr;
	FILE *fp;

	sprintf(tmpbuf, "flash get %s > %s", keyword, tmp_file);
	system(tmpbuf);

	fp = fopen(tmp_file, "r");
	if (fp == NULL) {
		printf("read tmp file [%s] failed!\n", tmp_file);
		return 0;
	}
	fgets(tmpbuf, 100, fp);
	fclose(fp);

	ptr = strchr(tmpbuf, '=');
	if (ptr == NULL) {
		printf("read %s value failed!\n", keyword);
		return 0;
	}

	*pVal = atoi(ptr+1);
	return 1;
}

#if !(defined(CONFIG_RTL865X_SC) && defined(CONFIG_RTL865X_AC) && defined(CONFIG_RTL8186_TR) && defined(CONFIG_RTL8196C_EC)) && !defined(CONFIG_RTL865X_KLD)
static int is_wlan0_exist(void)
{
	char tmpbuf[100], *ptr=NULL;
	FILE *fp;

	sprintf(tmpbuf, "ifconfig > %s", tmp_file);
	system(tmpbuf);

	fp = fopen(tmp_file, "r");
	if (fp == NULL)
		return 0;

	while (fgets(tmpbuf, 100, fp)) {
		ptr = strstr(tmpbuf, "wlan0");
		if (ptr) {
			if (strlen(ptr) <= 5)
				break;
			if (*(ptr+5) != '-')
				break;
		}
	}
	fclose(fp);

	return (ptr ? 1 : 0);
}
#endif

#if !defined(CONFIG_RTL865X_KLD)
static void enable_wlan(void)
{
	int wlan_disabled;
	int wlan_wds_enabled=0;
	int wlan_wds_num=0;
	int i;
	char cmdBuffer[100];
	int repeader_enabled;

	//DEBUG_PRINT("Enable wlan!\n");

	get_flash_int_value("WLAN_DISABLED", &wlan_disabled);
	get_flash_int_value("WDS_ENABLED", &wlan_wds_enabled);
	get_flash_int_value("WDS_NUM", &wlan_wds_num);
	get_flash_int_value("REPEATER_ENABLED1", &repeader_enabled);

	if (!wlan_disabled){
		system("ifconfig wlan0 up");
		if(wlan_wds_enabled==1){
				for(i=0;i<wlan_wds_num;i++){
					sprintf(cmdBuffer, "ifconfig wlan0-wds%d up", i);
					system(cmdBuffer);
				}
		}
	}

	if (repeader_enabled)
		system("ifconfig wlan0-vxd up");
}

static void disable_wlan(void)
{
	int wlan_disabled;
	int wlan_wds_enabled=0;
	int wlan_wds_num=0;
	int i;
	char cmdBuffer[100];
	int repeader_enabled;

	//DEBUG_PRINT("Disable wlan!\n");
	get_flash_int_value("WLAN_DISABLED", &wlan_disabled);
	get_flash_int_value("WDS_ENABLED", &wlan_wds_enabled);
	get_flash_int_value("WDS_NUM", &wlan_wds_num);
	get_flash_int_value("REPEATER_ENABLED1", &repeader_enabled);

	if (!wlan_disabled) {
		system("iwpriv wlan0 set_mib keep_rsnie=1");
		system("ifconfig wlan0 down");
		if(wlan_wds_enabled==1){
				for(i=0;i<wlan_wds_num;i++){
					sprintf(cmdBuffer, "ifconfig wlan0-wds%d down", i);
					system(cmdBuffer);
				}
		}
	}

	if (repeader_enabled) {
		system("iwpriv wlan0-vxd set_mib keep_rsnie=1");
		system("ifconfig wlan0-vxd down");
	}
}

#if defined(CONFIG_RTL8196B)
// Read RF h/w switch (GPIOC3) to see if need to disable/enable wlan interface
static int poll_rf_switch(void)
{
	static int rf_enabled=2;
	static int is_wlan_enabled=-1;
	int temp, wlan_disabled;
	temp = test_gpio();

	if (rf_enabled == 2) { // first time
		rf_enabled = temp;
		if(rf_enabled ==1)
			return 1;
		if(rf_enabled ==0)
			return 2;
	}

	if (temp != rf_enabled) {
		rf_enabled = temp;

		if (!get_flash_int_value("WLAN_DISABLED", &wlan_disabled))
			return -1;

		if (rf_enabled && !wlan_disabled){
			//	enable_wlan();
			return 1; //WLAN is enabled, but we should check the time match in advance
		}
		else {
			pre_enable_state = 0;
			disable_wlan();
			return 2; //WLAN disabled
		}
	}
#if 0 ///GPIO state no change, we donot modify wlan0 interface state, let schedule to modify the state
	else {
		if (is_wlan_enabled < 0) { // first time
			is_wlan_enabled = is_wlan0_exist();
			get_flash_int_value("WLAN_DISABLED", &wlan_disabled);
			printf("is wlan_enabled==%d\n", is_wlan_enabled);
			if (!wlan_disabled && is_wlan_enabled && temp == 0)
				disable_wlan();
			if(is_wlan_enabled==0)
			return 0;	//first time check gpio status, but we check wlan by scheduld
			if(is_wlan_enabled ==1)
				return 1;
		}
		//gpio status is not changed
		if(temp ==1)
			return 1;
		else if(temp ==0)
			return 2;
	}
#endif

	// gpio status is not changed
	if (temp == 1)
		return 1;
	else if (temp == 0)
		return 2;
	else
		return 0; //for compile warning only
}
#endif

#else    // CONFIG_RTL865X_KLD

char *wlan_if_name[] = {
	"wlan0",
	"wlan0-vxd",
	"wlan0-va0",
	"wlan0-va1",
	"wlan0-va2",
	"wlan0-va3",
};

char *eth_if_name[] = {
	"eth0",
	"eth2",
	"eth3",
	"eth4",
};

static void enable_wlan(int wlan_if)
{
	char cmdBuffer[64];

	sprintf(cmdBuffer, "iwpriv %s set_mib func_off=0", wlan_if_name[wlan_if]);
	system(cmdBuffer);
}

static void disable_wlan(int wlan_if)
{
	char cmdBuffer[64];

	sprintf(cmdBuffer, "iwpriv %s set_mib func_off=1", wlan_if_name[wlan_if]);
	system(cmdBuffer);
}
#endif

#if !(defined(CONFIG_RTL865X_SC) && defined(CONFIG_RTL865X_AC) && defined(CONFIG_RTL8186_TR) && defined(CONFIG_RTL8196C_EC))
static void parse_schl(char *argv, struct ps_info *ps)
{
	int i, head, tail, value[4];
	char tmpbuf[8];

	head = 0;
	for (i=0; i<4; i++) {
		tail = head + 1;
		while (argv[tail] != ',')
			tail++;
		memset(tmpbuf, 0, sizeof(tmpbuf));
		strncpy(tmpbuf, &(argv[head]), tail-head);
		value[i] = atoi(tmpbuf);
		head = tail + 1;
	}

	ps->fTime = value[1];
	ps->tTime = value[2];
	ps->date  = value[3];
}

#define ECO_SUNDAY_MASK		0x00000001
#define ECO_MONDAY_MASK		0x00000002
#define ECO_TUESDAY_MASK	0x00000004
#define ECO_WEDNESDAY_MASK	0x00000008
#define ECO_THURSDAY_MASK	0x00000010
#define ECO_FRIDAY_MASK		0x00000020
#define ECO_SATURDAY_MASK	0x00000040

static void dump_ps(struct ps_info *ps)
{
	char tmpbuf[200];

	sprintf(tmpbuf, "date (%x): ", ps->date);
	if (ps->date & ECO_SUNDAY_MASK)
		strcat(tmpbuf, "Sunday ");
	if (ps->date & ECO_MONDAY_MASK)
		strcat(tmpbuf, "Monday ");
	if (ps->date & ECO_TUESDAY_MASK)
		strcat(tmpbuf, "Tuesday ");
	if (ps->date & ECO_WEDNESDAY_MASK)
		strcat(tmpbuf, "Wednesday ");
	if (ps->date & ECO_THURSDAY_MASK)
		strcat(tmpbuf, "Thursday ");
	if (ps->date & ECO_FRIDAY_MASK)
		strcat(tmpbuf, "Friday ");
	if (ps->date & ECO_SATURDAY_MASK)
		strcat(tmpbuf, "Saturday ");
	strcat(tmpbuf, "\n");
	DEBUG_PRINT(tmpbuf);

	DEBUG_PRINT("From (%x): %dh %dm\n", ps->fTime, (ps->fTime / 60), (ps->fTime % 60));
	DEBUG_PRINT("To (%x): %dh %dm\n",  ps->tTime, (ps->tTime /60), (ps->tTime % 60));
}

#ifdef CONFIG_RTL865X_KLD
static void enable_eth(int eth_if)
{
	char cmdBuffer[64];

	sprintf(cmdBuffer, "ifconfig %s 0.0.0.0", eth_if_name[eth_if]);
	system(cmdBuffer);
}

static void disable_eth(int eth_if)
{
	char cmdBuffer[64];

	sprintf(cmdBuffer, "ifconfig %s down", eth_if_name[eth_if]);
	system(cmdBuffer);
}

static void check_time_and_control_eth(struct ps_info *ps, int eth_if, int force_on)
{
	int hit_date=0, hit_time=0;
	unsigned int start, end, current;
	time_t tm;
	struct tm tm_time;

	if (force_on) {
		hit_date = 1;		
		hit_time = 1;			
	}	

	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));

	DEBUG_PRINT(" tm_wday=%d, tm_hour=%d, tm_min=%d\n",
		tm_time.tm_wday, tm_time.tm_hour, tm_time.tm_min);

	switch(tm_time.tm_wday) {
		case 0:
			if (ps->date & ECO_SUNDAY_MASK)
				hit_date = 1;
			break;
		case 1:
			if (ps->date & ECO_MONDAY_MASK)
				hit_date = 1;
			break;
		case 2:
			if (ps->date & ECO_TUESDAY_MASK)
				hit_date = 1;
			break;
		case 3:
			if (ps->date & ECO_WEDNESDAY_MASK)
				hit_date = 1;
			break;
		case 4:
			if (ps->date & ECO_THURSDAY_MASK)
				hit_date = 1;
			break;
		case 5:
			if (ps->date & ECO_FRIDAY_MASK)
				hit_date = 1;
			break;
		case 6:
			if (ps->date & ECO_SATURDAY_MASK)
				hit_date = 1;
			break;
	}

	start = ps->fTime;
	end   = ps->tTime;
	current = tm_time.tm_hour * 60 + tm_time.tm_min;
	DEBUG_PRINT("start=%d, end=%d, current=%d\n", start, end, current);

	if (end >= start) {
		if ((current >= start) && (current < end))
			hit_time = 1;
	}
	else {
		if ((current >= start) || (current < end))
			hit_time = 1;
	}

	DEBUG_PRINT("[%s] pre_enable_state=%d, hit_date=%d, hit_time=%d\n",
		eth_if_name[eth_if], pre_eth_enable_state[eth_if], hit_date, hit_time);

	if (!pre_eth_enable_state[eth_if] && hit_date && hit_time) {
		enable_eth(eth_if);
		pre_eth_enable_state[eth_if] = 1;
	}
	else if (pre_eth_enable_state[eth_if] && (!hit_date || !hit_time)) {
		disable_eth(eth_if);
		pre_eth_enable_state[eth_if] = 0;
	}
}
#endif // CONFIG_RTL865X_KLD

static void check_time_and_control_RF(struct ps_info *ps, int wlan_if, int force_on)
{
	int hit_date=0, hit_time=0;
	unsigned int start, end, current;
	time_t tm;
	struct tm tm_time;

	if (force_on) {
		hit_date = 1;		
		hit_time = 1;			
	}	

	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));

	DEBUG_PRINT(" tm_wday=%d, tm_hour=%d, tm_min=%d\n",
		tm_time.tm_wday, tm_time.tm_hour, tm_time.tm_min);

	switch(tm_time.tm_wday) {
		case 0:
			if (ps->date & ECO_SUNDAY_MASK)
				hit_date = 1;
			break;
		case 1:
			if (ps->date & ECO_MONDAY_MASK)
				hit_date = 1;
			break;
		case 2:
			if (ps->date & ECO_TUESDAY_MASK)
				hit_date = 1;
			break;
		case 3:
			if (ps->date & ECO_WEDNESDAY_MASK)
				hit_date = 1;
			break;
		case 4:
			if (ps->date & ECO_THURSDAY_MASK)
				hit_date = 1;
			break;
		case 5:
			if (ps->date & ECO_FRIDAY_MASK)
				hit_date = 1;
			break;
		case 6:
			if (ps->date & ECO_SATURDAY_MASK)
				hit_date = 1;
			break;
	}

	start = ps->fTime;
	end   = ps->tTime;
	current = tm_time.tm_hour * 60 + tm_time.tm_min;
	DEBUG_PRINT("start=%d, end=%d, current=%d\n", start, end, current);

	if (end >= start) {
		if ((current >= start) && (current < end))
			hit_time = 1;
	}
	else {
		if ((current >= start) || (current < end))
			hit_time = 1;
	}

#if !defined(CONFIG_RTL865X_KLD)
	DEBUG_PRINT("pre_enable_state=%d, hit_date=%d, hit_time=%d\n", pre_enable_state, hit_date, hit_time);

	if (pre_enable_state < 0) { // first time
		if (hit_date && hit_time) {
			pre_enable_state = 1;
			if (!is_wlan0_exist())
				enable_wlan();
		}
		else {
			disable_wlan();
			pre_enable_state = 0;
		}
	}
	else {
		if (!pre_enable_state && hit_date && hit_time) {
			enable_wlan();
			pre_enable_state = 1;
		}
		else if (pre_enable_state && (!hit_date || !hit_time)) {
			disable_wlan();
			pre_enable_state = 0;
		}
	}
#else
	DEBUG_PRINT("[%s] pre_enable_state=%d, hit_date=%d, hit_time=%d\n",
		wlan_if_name[wlan_if], pre_enable_state[wlan_if], hit_date, hit_time);

	if (!pre_enable_state[wlan_if] && hit_date && hit_time) {
		enable_wlan(wlan_if);
		pre_enable_state[wlan_if] = 1;
	}
	else if (pre_enable_state[wlan_if] && (!hit_date || !hit_time)) {
		disable_wlan(wlan_if);
		pre_enable_state[wlan_if] = 0;
	}
#endif
}
#endif

int main(int argc, char *argv[])
{
	FILE *fp;
	int gpio_state=-1;
	int last_gpio_state=-1;
	int i;
#if !(defined(CONFIG_RTL865X_SC) && defined(CONFIG_RTL865X_AC) && defined(CONFIG_RTL8186_TR) && defined(CONFIG_RTL8196C_EC))
	struct ps_info ps;
	static int wlan_schl=0, ntp_enable=0, ntp_success=0;
#if defined(CONFIG_RTL865X_KLD)
	struct ps_info ps_va0, ps_eth;
	static int wlan_va0_schl=0, eth_port=0;
	pre_enable_state[0] = 1;
	for (i=1; i<WLAN_IF_TOTAL; i++)
		pre_enable_state[i] = 0;
#endif

	memset(&ps, '\0', sizeof(ps));
	if (argc > 2) {
		for (i=0; i<argc; i++) {
			if (!strcmp(argv[i], "-e")) {
				parse_schl(argv[i+1], &ps);
				dump_ps(&ps);
				wlan_schl = 1;
			}
#if defined(CONFIG_RTL865X_KLD)
			if (!strcmp(argv[i], "-f")) {
				parse_schl(argv[i+1], &ps_va0);
				dump_ps(&ps_va0);
				wlan_va0_schl = 1;
			}
			if (!strcmp(argv[i], "-g")) {
				parse_schl(argv[i+1], &ps_eth);
				dump_ps(&ps_eth);			
			}
			if (!strcmp(argv[i], "-p")) {
				sscanf(argv[i+1], "%d", &eth_port);	
			}			
#endif
		}
	}

#if !defined(CONFIG_RTL865X_AP)
	get_flash_int_value("NTP_ENABLED", &ntp_enable);
#endif
#endif

	while(1)
	{
		fp = fopen("/proc/load_default", "r");
		fscanf(fp, "%d", &i);
		if (i == 1) {
			printf("Going to Reload Default\n");
			system("flash reset");
			system("reboot");
		}
		fclose(fp);

#if !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B) && !defined(CONFIG_RTL8196C_EC)
#if !defined(CONFIG_RTL865X_AP)
		if (ntp_enable) {
			if (!ntp_success) {
				fp = fopen("/etc/TZ", "r");
				if (fp) {
					fclose(fp);
					ntp_success = 1;
				}
			}
		}
#endif
		// enable wlan scheduling
		if (wlan_schl && (!ntp_enable || ntp_success))
			check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
#if defined(CONFIG_RTL865X_KLD)
		if (wlan_va0_schl && (!ntp_enable || ntp_success))
			check_time_and_control_RF(&ps_va0, WLAN_IF_VA0, 0);
		else if (wlan_va0_schl && ntp_enable && !ntp_success)
			check_time_and_control_RF(&ps, WLAN_IF_VA0, 1);

		if (eth_port &&  (!ntp_enable || ntp_success)) {
			if (eth_port & 1)			
				check_time_and_control_eth(&ps_eth, ETH_IF_0, 0);
			if (eth_port & 2)			
				check_time_and_control_eth(&ps_eth, ETH_IF_2, 0);
			if (eth_port & 4)			
				check_time_and_control_eth(&ps_eth, ETH_IF_3, 0);			
			if (eth_port & 8)			
				check_time_and_control_eth(&ps_eth, ETH_IF_4, 0);
		}		
		else if (eth_port && ntp_enable && !ntp_success) {
			if (eth_port & 1)			
				check_time_and_control_eth(&ps_eth, ETH_IF_0, 1);
			if (eth_port & 2)			
				check_time_and_control_eth(&ps_eth, ETH_IF_2, 1);
			if (eth_port & 4)			
				check_time_and_control_eth(&ps_eth, ETH_IF_3, 1);			
			if (eth_port & 8)			
				check_time_and_control_eth(&ps_eth, ETH_IF_4, 1);		
		}		
#endif
#endif

#if defined(CONFIG_RTL8196B)
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
		/* rtl8196C or rtl8198 do not need to check rf pin status*/
		gpio_state = 1;
		last_gpio_state =gpio_state; 
#else
		gpio_state = poll_rf_switch();
#endif		

		//printf("the gpio state =%d\n", gpio_state);
		if (last_gpio_state != gpio_state) {
			if (last_gpio_state == -1) {    // This is first state, we should check in advance for schedule, if the gpio is enabled
				if (gpio_state == 1) {
#if !defined(CONFIG_RTL865X_AP)
					if (ntp_enable) {
						if (!ntp_success) {
							fp = fopen("/etc/TZ", "r");
							if (fp) {
								fclose(fp);
								ntp_success = 1;
							}
						}
					}
#endif
					// enable wlan scheduling
					if (wlan_schl && (!ntp_enable || ntp_success))
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					else if (wlan_schl && ntp_enable && !ntp_success)
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
				}
			}
			else {    // It is not first state, we get the result from gpio state
				if (gpio_state == 1) {    // gpio state is available for time schedule match
#if !defined(CONFIG_RTL865X_AP)
					if (ntp_enable) {
						if (!ntp_success) {
							fp = fopen("/etc/TZ", "r");
							if (fp) {
								fclose(fp);
								ntp_success = 1;
							}
						}
					}
#endif
					// enable wlan scheduling
					if (wlan_schl && (!ntp_enable || ntp_success))
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					else if (wlan_schl && ntp_enable && !ntp_success)
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					else
						enable_wlan();
				}
			}
			last_gpio_state = gpio_state;
		}
		else {
			if (gpio_state == 1) {
#if !defined(CONFIG_RTL865X_AP)
				if (ntp_enable) {
					if (!ntp_success) {
						fp = fopen("/etc/TZ", "r");
						if (fp) {
							fclose(fp);
							ntp_success = 1;
						}
					}
				}
#endif
				// enable wlan scheduling
				if (wlan_schl && (!ntp_enable || ntp_success))
					check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
				else if (wlan_schl && ntp_enable && !ntp_success)
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
			}
		}
#endif
		sleep(3);
	}
}

