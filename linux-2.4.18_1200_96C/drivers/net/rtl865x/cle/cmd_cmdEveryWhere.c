
#include "../rtl865x/rtl_types.h"
#include "rtl_cle.h" //cle_grp_t rootGrp[] is externed at cle.h

#include "cmd_cmdEveryWhere.h"



int32 cmdDoScript(uint32 channel, int32 argc, int8 ** argv)
{
	//cle_script(channel,argv[1]);
	return CLE_EXERN_OK_NOSCRIPT;
}

cle_exec_t cmdEveryWhere[] = { 
#if 0
	{
		"script",			//cmdStr
		"run command script",	//cmdDesc
		"script argument1",			//cmdUsage
		cmdDoScript,			//execution function
		0,				//access level
		1,				//minimum argument name + parameter
		NULL//displayFun
	},
#endif
};

