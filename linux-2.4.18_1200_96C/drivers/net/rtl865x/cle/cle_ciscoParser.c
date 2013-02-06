/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Cisco CLI Command parser
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
* $Id: cle_ciscoParser.c,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_ciscoParser.c,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.13  2007/06/01 09:14:57  michaelhuang
* -: removed the rtl867x compiler flag.
*
* Revision 1.12  2007/01/09 05:13:59  michaelhuang
* *: fixed some compile flag error
*
* Revision 1.11  2007/01/09 02:49:52  michaelhuang
* *: Prepare implement test module for RTL8672
*
* Revision 1.10  2006/12/04 08:34:21  alva_zhang
* replace all cle_printf  with rtlglue_printf
*
* Revision 1.9  2006/10/19 03:42:19  hyking_liu
* *: Modify for new user map module related with naptFlow
*
* Revision 1.8  2006/07/13 16:01:16  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.7  2006/05/03 01:46:33  yjlou
* *: little fixed for splint check
*
* Revision 1.6  2005/06/10 05:32:22  yjlou
* +: Porting CLE Shell to Linux user space for model test.
*    See RTL865X_MODEL_USER compile flags.
*
* Revision 1.5  2004/12/21 13:16:26  yjlou
* *: fixed a bug of CLE: CLE should always match longest pattern.
*
* Revision 1.4  2004/11/10 05:15:10  cfliu
* +: add new cle_cmdParser() to support cmd parsing without using cleshell
*
* Revision 1.3  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.2  2004/03/19 13:12:42  cfliu
* no message
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.41  2003/08/15 07:05:35  hiwu
* add __linux__ compiler flag
*
* Revision 1.40  2003/08/11 08:35:26  hiwu
* remove #include <util/mathutils.h>
*
* Revision 1.39  2003/08/06 03:33:28  hiwu
* remove unused include files
*
* Revision 1.38  2003/07/21 06:33:21  cfliu
* fix compile warnings
*
* Revision 1.37  2003/05/19 10:07:22  cfliu
* no message
*
* Revision 1.36  2003/04/30 15:36:44  cfliu
* rename macros. macro moved to types.h
*
* Revision 1.35  2003/03/26 10:45:48  cfliu
* Bug fix when retrieving token value from self-repeating nodes.
*
* Revision 1.34  2003/03/24 06:22:46  cfliu
* Fix partial match problem. tokens must be exactly matched to be accepted.
*
* Revision 1.33  2003/03/21 08:05:58  cfliu
* Bug fixes...
*
* Revision 1.32  2003/03/18 14:31:19  cfliu
* Fix greenhill compile warnings and move run.files in rtl8650 platform compile ok.
*
* Revision 1.31  2003/03/14 13:31:08  cfliu
* Fix word  alignment problem in ciscoParser module
*
* Revision 1.30  2003/03/14 10:18:31  cfliu
* Fix word align problems
*
* Revision 1.29  2003/03/12 10:43:27  cfliu
* Add CISCOCMD_ACCEPT_SYMBOL,_STRING...
*
* Revision 1.28  2003/03/07 07:22:44  cfliu
* Bug fix for self repeat nodes.
*
* Revision 1.27  2003/02/17 06:20:07  cfliu
* Fix a typo
*
* Revision 1.26  2003/02/17 06:19:20  cfliu
* Fix a typo
*
* Revision 1.25  2003/02/17 05:53:23  cfliu
* Fix a buffer overflow problem. Added an assertion to make sure command string never overflows _thisCmdformatCopy again.
*
* Revision 1.24  2003/02/11 04:15:00  cfliu
* Fix a bug when calling check function pointer of self repeating tokens.
*
* Revision 1.23  2003/02/07 09:58:00  cfliu
* Allow digit numbers as command keywords.
*
* Revision 1.22  2003/01/21 06:01:18  cfliu
* * Remove redundant header file inclusion...still need closer look on them..
*
* Revision 1.21  2003/01/02 04:45:45  cfliu
* Implement stateful group feature.
* Tokens are pushed into/poped from user's dirStack[] when entering/exiting a stateful group.
* When user's execFunc() calls cle_getNextCmdToken(), tokens saved in dirStack[] are retrieved first
* and then ciscoCmdGetNextToken() returns later tokens.
*
* Remove all unused code.
*
* Revision 1.20  2002/12/26 14:07:09  cfliu
* Rewrite dynamic command changing code...
*
* Revision 1.19  2002/12/25 07:30:29  cfliu
* Fix ciscoCmdGetNextToken()...
*
* Revision 1.18  2002/12/24 04:29:36  cfliu
* Add self repeat support. Each keyword or parameter token can be prefixed with
* a "(m~n)" to tell parser that the indicated token could only be accepted between m to n occurences (inclusive)
* ex: (1~3)hello, (1~4)%param, (1~)%counter, etc....
*
* Revision 1.17  2002/12/20 03:32:27  cfliu
* Add support for option list in _processUserCommand()
*
* Revision 1.16  2002/12/18 03:45:05  cfliu
* Add <cr> token for user to query end of command position
* Stop printing command synopsis
*
* Revision 1.15  2002/12/17 07:14:55  cfliu
* no message
*
*
* Revision 1.13  2002/12/10 12:04:20  cfliu
* Fix GreenHill compile warning
*
* Revision 1.12  2002/12/10 11:11:57  jzchen
* Fix a bug when looking for customized type.
* Print command token count.
*
* Revision 1.11  2002/12/10 10:48:13  cfliu
* Change ciscoCmdGetNextToken prototype
*
* Revision 1.10  2002/12/09 10:18:20  cfliu
* Add "?" online help support
* Accepts partial command doing best match for new Cisco-like commands
*
* Revision 1.9  2002/12/06 11:03:28  cfliu
* cle_ciscoParser.h is local header file now.
* Rename cle_userCmd_t -> cle_exec_t
* Change function prototype to fit with our CLE parser.
* Change numerate function prototype.
*
* Revision 1.8  2002/12/05 08:13:24  cfliu
* Add check for user terminal type.
* Use formatted output to support web  clients.
* Some bug fixes.
*
* Revision 1.7  2002/12/01 12:44:32  cfliu
* ciscoCmdPrintSynopsis prints more help info...
*
* Revision 1.6  2002/11/29 07:09:53  idayang
* include/export/mgmt -> include/export/management.
*
* Revision 1.5  2002/11/26 03:54:35  cfliu
* Add ciscoCmdParserShowHelp, ciscoCmdChecker.
* Comment out cle_ciscoParserCLI()
*
* Revision 1.4  2002/11/14 13:39:21  cfliu
* A major revision.
* Decouple CLE engine functions with test module exec functions and caller of CLE engine
* Support dynamic command dispatching.
* Separate system shared parameter check function with module's private parameter check function
*
* Revision 1.3  2002/11/13 03:42:44  cfliu
* Redundant code cleanup and bug fix. This is a stable version now.
*
* Revision 1.2  2002/11/12 11:14:47  cfliu
* Add command format help string.
*
* Revision 1.1  2002/11/12 04:57:17  cfliu
* Initial version. Cisco Command Line Parser.
* This is a preliminary release.
*
*/

#include "../rtl865x/rtl_types.h"
#include "rtl_cle.h"
#include "cle_ciscoParser.h"
#include "cle_utility.h"
#include "cle_define.h"
#include "rtl_utils.h"

#if defined(RTL865X_MODEL_USER)
#include <ctype.h>
#include "rtl_glue.h"
#include "cle_userglue.h"
#endif


/*********************************************************************************
	Section 1:	Internal structures and variables
**********************************************************************************/
//#define  DEBUG_CISCO_CMDPARSER
#define MAX_NODE_STACK_DEPTH	128
#define MAX_LEVELOPTIONS		32
//#define MAX_TOKENS				78	//For debuging purpose
//#define MAX_NODES				78		//For debugging purpose
#define MAX_TOKENS				256		//No more than 256!!
#define MAX_NODES				256		//No more than 256!!
#define MAX_CMD_TEMPBUFFER	3072
#define MAX_CMD_BUFFER		1024
#define DEFAULT_MAX_REPEAT		255

#define reent_getCleUserId()	0


struct node {
	uint16 node:1 , keyword:1, param:1, reqStart:1, reqEnd:1, optionStart:1,   xor:1, endCommand:1, selfRepeat:1;  /*optionEnd:1, reqEnd:1,*/
	int16 tokenSize;	  // = size  returned by ciscoCmdGetNextToken
	uint8 	lastAccepted, nextAccepted, reqBeginNode;
	
	uint32 repeat:8, min_repeat:8, max_repeat:8, checked:8; //for self repeating nodes
	void *token; //= nextToken returned by ciscoCmdGetNextToken. may be converted by check function if TOKEN_CONVERTED is set.
	cle_paramType_t *paramType;		//Check and convert the token if token is a format specifier.
	int8 *helpString;	//Help string of this token, valid for keyword or param token only
} ;

struct candidateCmd {
	 uint32 list[MAX_NODES];	//temp buffer to store candidate nodes in command graph
	 uint32 count;
};

 struct stack{	
	int32 top;
	uint32 items[MAX_NODE_STACK_DEPTH];
};

 struct cmdToken {
 	int8 *cmd;
 	int8 *help;
 };

static struct {
		uint32 start;
		uint32 end;
} _optionList [MAX_LEVELOPTIONS];

//_lastNodeStack:
//_linkNodeStack:
//_optionListStack:
//_optionCountStack:
//
static struct stack _lastNodeStack, _linkNodeStack, _optionListStack,_optionCountStack;
static struct candidateCmd _candidate;
static struct node _nodes[MAX_NODES];		//Data structure for each command token
static struct cmdToken _tokenPos[MAX_NODES];			//Points to each token of command format string
static uint8 _linkGraph[MAX_NODES][MAX_NODES+1];	//The command graph
static uint8 _tempBuffer[MAX_CMD_TEMPBUFFER]; //Dynamic memory for parameter check functions
volatile static int8 _thisCmdformatCopy[MAX_CMD_TEMPBUFFER];
static uint32  _tempBufferUsed;				
static uint32 _nonRepeatingTempBufferStart;
static uint32 _nodeUsed;	//Number of nodes array used.
static int8 *_terminateNode = CISCOCMD_ACCEPT_SYMBOL;
static int8 *_terminateNodeHelpString = CISCOCMD_ACCEPT_STRING;
cle_exec_t *_nextNewCmd;
cle_exec_t *_thisCommand;

#ifdef DEBUG_CISCO_CMDPARSER
#define _linkGraphEdge( x , y )		_linkGraph[x][y]=1; rtlglue_printf("%d -> %d \t", x, y)
#else
#define _linkGraphEdge( x , y )		_linkGraph[x][y]=1
#endif
/*********************************************************************************
	Section 2:	Internal function prototype
**********************************************************************************/

#ifdef DEBUG_CISCO_CMDPARSER
	static void _printAllStacks(int8 *token);
	static void _printLinkGraph(uint32 lineNumber);
#endif


/*********************************************************************************
	Section 4:	Exported public global variables
**********************************************************************************/
/*********************************************************************************
	Section 5:	Stack operation utility functions
**********************************************************************************/
#define _isStackEmpty(stack)	(((stack)->top<=-1)? 1 : 0)

 static __inline__ int32 _popStack(struct stack *ps){
	if (_isStackEmpty(ps))
		return -1;
	assert(ps->top>=0);
	return(ps->items[ps->top--]);
}

 static __inline__ int32 _pushStack(struct stack *ps,int32 x){
	if (ps->top==MAX_NODE_STACK_DEPTH-1)
		return -1;
	ps->items[++(ps->top)]=x;
	return 0;
}

 static __inline__ int32 _peekStackTop(struct stack *ps){
	if (!_isStackEmpty(ps))
		return(ps->items[ps->top]);
	return -1;
}

 static __inline__ int32 _pokeStackTop(struct stack *ps, int32 x){
	if (ps->top==MAX_NODE_STACK_DEPTH-1)
		return -1;
	ps->items[ps->top]=x;
	return 0;
}


/*********************************************************************************
	Section 7:	Internal function implementation
**********************************************************************************/

//If a token is a parameter type specifier, lookup paramTypes table to see if is there.
static cle_paramType_t *_lookupTokenType(int8 *str){
	int32 index, total;
	cle_paramType_t *thisCmdParamType =_thisCommand->cmdParamType;
	total = _thisCommand->cmdParamTypeNum;
	//Lookup command's private paramType array first.
	if(thisCmdParamType){
		for(index=0; index< total; index++){
			if(strcmp(thisCmdParamType[index].formatString, str)==0)
				return &thisCmdParamType[index];
		}
	}
	//Not found, now lookup system paramType array.
	for(index=0; index< sysParamTypesNum; index++){
		if(!strcmp(systemParamTypes[index].formatString, str))
			return &systemParamTypes[index];
	}
	return (cle_paramType_t *)NULL;
}

//Split the command format string into separate tokens for further processing
//Each token can be either 1) a keyword or a parameter (begins with a %) alone, or 
// 2) a keyword, parameter WITH help string, surrounded by a pair of '...'
// Ex: A keyword with help string:    keyword'This is help string'
//	   A parameter with help string	%d'A numeric number'
static int32 _splitCmdFormatTokens(void){
	int8 *cmdFormatStr;
	int8 *cmds, *cmd, *help, *savedPtr, *savedPtr2;
	int32  index=0;

	assert(_thisCommand->cmdUsage!=NULL);
	//Copy _thisCommand->cmdUsage to a writable buffer.
	assert(strlen(_thisCommand->cmdUsage)<MAX_CMD_TEMPBUFFER);
	strncpy((int8 *)_thisCmdformatCopy, (const char *)_thisCommand->cmdUsage,MAX_CMD_TEMPBUFFER);
	cmdFormatStr = (int8 *)_thisCmdformatCopy;


	memset(&_tokenPos, 0 , sizeof(_tokenPos));
	//Parse cmdFormatStr and delimit all tokens.
	cmds=strtok_r(cmdFormatStr, "'", &savedPtr);
	while(cmds){
		cmd=strtok_r(cmds, " ", &savedPtr2);
		while(cmd){
			_tokenPos[index].cmd=cmd;
			cmd=strtok_r(NULL, " ", &savedPtr2);
			if(!cmd)
				_tokenPos[index].help=NULL;
			index++;
  		};

		help=strtok_r(NULL, "'", &savedPtr);
		if(help){
			_tokenPos[--index].help = help;
			index++;
		}
		cmds=strtok_r(NULL, "'", &savedPtr);
	};
	_tokenPos[index++].cmd=_terminateNode;
	assert(index>0);

#ifdef DEBUG_CISCO_CMDPARSER
{	int32 i;
	for(i=0; i < index; i++)
		rtlglue_printf("\n%d. Cmd: %s  Help:'%s'", i+1,_tokenPos[i].cmd, _tokenPos[i].help?_tokenPos[i].help:"(null)");
}
#endif

	return index;
}	


 static __inline__  void _createKeywordNode(struct node *thisNode, int8 *token, int8 *helpString, uint8 min_repeat, uint8 max_repeat ){
	thisNode->keyword=thisNode->node = 1;
	thisNode->token=token;
	thisNode->helpString  = helpString;
	if(min_repeat||max_repeat){
		thisNode->selfRepeat = 1;
		thisNode->min_repeat=(min_repeat==0)?1:min_repeat;
		thisNode->max_repeat=max_repeat;
	}
	thisNode->tokenSize = strlen(token);
}

 static __inline__  int32 _createParamNode(struct node *thisNode, int8 *token, int8 *helpString, uint8 min_repeat, uint8 max_repeat ){
	uint32 size;
	thisNode->paramType = _lookupTokenType(token);
	assert(thisNode->paramType!=NULL);
	assert(IS4BYTEALIGNED((memaddr)_tempBuffer+_tempBufferUsed));
	thisNode->token = (void *) _tempBuffer+_tempBufferUsed;
	thisNode->param=thisNode->node = 1;	
	thisNode->helpString  = helpString;
	thisNode->tokenSize = 0;
	if(min_repeat||max_repeat){
		thisNode->selfRepeat = 1;
		thisNode->min_repeat=(min_repeat==0)?1:min_repeat;
		thisNode->max_repeat=max_repeat;
	}
	size=thisNode->paramType->maxSize;
	if(!IS4BYTEALIGNED(size)) //not word aligned
		return ROUNDUP4(size);
	 return size;
}
	
 static __inline__  void _createReqStartNode(struct node *thisNode, int8 *token, uint8 min_repeat, uint8 max_repeat ){
	thisNode->reqStart=thisNode->node=1;
	thisNode->token=token;
	thisNode->tokenSize = strlen(token);
	if(min_repeat||max_repeat){
		thisNode->selfRepeat = 1;
		thisNode->min_repeat=(min_repeat==0)?1:min_repeat;
		thisNode->max_repeat=max_repeat;
	}
}

 static __inline__  void _createReqEndNode(struct node *thisNode, int8 *token){
	thisNode->node = thisNode->reqEnd=1;
	thisNode->token=token;
	thisNode->tokenSize = strlen(token);
}

 static __inline__  void _createOptionStartNode(struct node *thisNode, int8 *token, uint8 min_repeat, uint8 max_repeat ){
	thisNode->optionStart=1;
	thisNode->token=token;
	thisNode->tokenSize = strlen(token);
	if(min_repeat||max_repeat){
		thisNode->selfRepeat = 1;
		thisNode->min_repeat=min_repeat;
		thisNode->max_repeat=max_repeat;
	}
}

 static __inline__  void _createOptionEndNode(struct node *thisNode, int8 *token){
	thisNode->token=token;
	thisNode->tokenSize = strlen(token);
}

 static __inline__  void _createXORDummyNode(struct node *thisNode, int8 *token){
	thisNode->xor=1;
}

 static __inline__  void _createTerminateNode(struct node *thisNode){
	thisNode->endCommand=thisNode->node=1;
	thisNode->token=_terminateNode;
	thisNode->tokenSize = strlen(_terminateNode);
	thisNode->helpString = _terminateNodeHelpString;
}

 static __inline__  void _createOptionFakeNode(struct node *thisNode){
	thisNode->node=1;
}

//Create edges from all last level of options to this node, endNodeIndex
static void _linkAllOptions(uint32 endNodeIndex){
	uint32 nodeIndex, index;
	
	_popStack(&_optionCountStack);//discard
	index = 0;	
	//pop all options from _optionListStack
	while((nodeIndex = _popStack(&_optionListStack))>0 && !_nodes[nodeIndex].node){
		_optionList[index].end = nodeIndex;
		nodeIndex = _popStack(&_optionListStack);
		_optionList[index].start = nodeIndex;
		assert(nodeIndex&&!_nodes[nodeIndex].node);
		index++;
	}
	if(_nodes[nodeIndex].node) //reached the start of option list
		_linkGraphEdge(nodeIndex, endNodeIndex);

	// and mark all possible permutation edges in graph.
	if(index){
		int32 x, y;
		for(x = 0; x < index; x++){
			int32 start, end;
			start = _optionList[x].end; 
			for(y = 0; y < index; y++){
				end = _optionList[y].start;
				if(x!=y)
					_linkGraphEdge(start, end);
			}				
		}
		for(x = 0; x < index; x++)
			_linkGraphEdge(_optionList[x].end, endNodeIndex);			
	}
}

//Create edges from all  "nodes" in _linkNodeStack to this node, endNodeIndex
static void _linkWithPreviousNodes(uint32 nodeIndex){

	while(!_isStackEmpty(&_linkNodeStack)){
		int32 start, stackItem;
		start = _popStack(&_linkNodeStack);
		_linkGraphEdge(start, nodeIndex);
		stackItem = _peekStackTop(&_linkNodeStack);
		if(stackItem>=0 &&( !_nodes[stackItem].node || _nodes[stackItem].reqStart))
			break;
		
	}
	if(_nodes[nodeIndex].selfRepeat){
		_linkGraphEdge(nodeIndex, nodeIndex);
	}
}

//Same as _linkWithPreviousNodes, create edges from all  nodes in _linkNodeStack to 
//this node, endNodeIndex. This is used when } is encountered.
static void _linkWithPreviousReqNodes(uint32 nodeIndex, uint32 reqBegin){
	while(!_isStackEmpty(&_linkNodeStack)){
		int32 start;
		start = _popStack(&_linkNodeStack);
		if(start<0)
			return;
		if(_nodes[start].xor) //discard | , get next
			continue;
		if(_nodes[start].reqStart)//discard { and stop
			break;
		_nodes[start].reqBeginNode = reqBegin; // _nodes[start] links to }, regBegin is the nodeIndex for corresponding { token.
		_linkGraphEdge(start, nodeIndex);
	}
}


static void _initializeLinkGraphRowColumn(int32 nodeIndex){
	uint32 index;
	memset(&_nodes[nodeIndex], 0, sizeof(struct node));
	for(index=0; index<MAX_NODES; index++)
		_linkGraph[index][nodeIndex]=0;
	for(index=0; index<MAX_NODES+1; index++)
		_linkGraph[nodeIndex][index]=0;
}

static int32 _createCommandGraph(void){
	uint32 nodeIndex,tokenIndex ;
	uint32 memNeeded;

	//Mark all stacks as empty.
	_lastNodeStack.top= _linkNodeStack.top= _optionListStack.top= _optionCountStack.top = -1;	

	assert(_nodeUsed>0);

	//Create first dummy start node, node 0 is the first dummy node
	_initializeLinkGraphRowColumn(0);
	_createOptionFakeNode(&_nodes[0]);
	_pushStack(&_linkNodeStack,0);

	memNeeded=0; 	//dynamic memory required.	
	//Retrieve each command token and create a command graph.
	for(nodeIndex=1,tokenIndex=0; nodeIndex< _nodeUsed+1; nodeIndex++, tokenIndex++){
		int8 *token, *helpString, ch;
		struct node *thisNode;
		uint8 min_repeat=0, max_repeat=0;

		token = _tokenPos[tokenIndex].cmd;
		helpString = _tokenPos[tokenIndex].help;
		ch = *token;
		thisNode = &_nodes[nodeIndex];

#ifdef DEBUG_CISCO_CMDPARSER
		_printAllStacks(token);
#endif		
		_initializeLinkGraphRowColumn(nodeIndex);

		//End of command. the terminating node
		if(ch==_terminateNode[0]){
			int32 stackItem;
			_createTerminateNode(thisNode);
			_linkWithPreviousNodes(nodeIndex);
			stackItem = _peekStackTop(&_optionCountStack);
			if(stackItem==0)
				_linkAllOptions(nodeIndex);
			_pushStack(&_linkNodeStack,nodeIndex);
			continue;
		}

		//Is a repeating specifier
		if(ch=='('){
			int8 *realToken, *saved, *repeater, *min, *max, *c;
			c= token+1;
			repeater = strtok_r(c, ")", &realToken);
			assert(repeater!=NULL);
			min = strtok_r(repeater, "~", &saved);
			min_repeat = (min)? AsciiToInt(&min): 0;
			max =strtok_r(NULL, "~", &saved);
			max_repeat = (max)?  AsciiToInt(&max): DEFAULT_MAX_REPEAT;
			token=realToken;
			ch=token[0];
			_nodes[nodeIndex].selfRepeat = 1;
		}

		//Is a parameter
		if(ch=='%'){
			int32 stackItem;
			_tempBufferUsed+=_createParamNode(thisNode, token, helpString, min_repeat, max_repeat);
			assert(IS4BYTEALIGNED(_tempBufferUsed));
			if(memNeeded > MAX_CMD_TEMPBUFFER)
				return -1;
			_linkWithPreviousNodes(nodeIndex);
			stackItem = _peekStackTop(&_optionCountStack);
			if(stackItem==0)
				_linkAllOptions(nodeIndex);
			_pushStack(&_linkNodeStack,nodeIndex);
			continue;
		}

		//Is a keyword
		if(isalpha(ch)||isdigit(ch)){
			int32 stackItem;
			_createKeywordNode(thisNode, token, helpString, min_repeat, max_repeat);
			_linkWithPreviousNodes(nodeIndex);
			stackItem = _peekStackTop(&_optionCountStack);
			if(stackItem==0)
				_linkAllOptions(nodeIndex);
			_pushStack(&_linkNodeStack,nodeIndex);
			continue;
		}
		

		switch (ch){
				int32 stackItem, stackItem2;
			case '|': //this is a dummy node which doesn't links with anybody.
				_createXORDummyNode(thisNode, token);

				//stackItem2 = _peekStackTop(&_linkNodeStack);
				//if(_nodes[stackItem2].reqEnd){
				//	_popStack(&_linkNodeStack);
				//}

				stackItem=_peekStackTop(&_lastNodeStack);
				_pushStack(&_linkNodeStack,nodeIndex);	
				_pushStack(&_linkNodeStack,stackItem);	
				break;
			case '{':	
				_createReqStartNode(thisNode, token, min_repeat, max_repeat);	
				_linkWithPreviousNodes(nodeIndex);				
				stackItem = _peekStackTop(&_optionCountStack);
				if(stackItem==0)
					_linkAllOptions(nodeIndex);
				_pushStack(&_lastNodeStack,nodeIndex);
				//put { twice
				_pushStack(&_linkNodeStack,nodeIndex);
				_pushStack(&_linkNodeStack,nodeIndex);

				break;
			case '}':
				_createReqEndNode(thisNode, token);	
				_linkWithPreviousReqNodes(nodeIndex, _popStack(&_lastNodeStack));
				stackItem = _peekStackTop(&_optionCountStack);
#ifdef DEBUG_CISCO_CMDPARSER				
				rtlglue_printf("\nLinking options...\n");
#endif
				if(stackItem==0)
					_linkAllOptions(nodeIndex);
				_pushStack(&_linkNodeStack,nodeIndex);				
				break;

			case '[':

				stackItem = _peekStackTop(&_optionCountStack);
				if(stackItem==0){ 
					//yet another new option in same option level
					_createOptionStartNode(thisNode, token, min_repeat, max_repeat);
					stackItem = _peekStackTop(&_linkNodeStack);
					_linkWithPreviousNodes(nodeIndex); // link with previous option's ]
					stackItem = _peekStackTop(&_lastNodeStack); 
					_linkGraphEdge(stackItem, nodeIndex); //link with the token before fist option in this level
					_pokeStackTop(&_optionCountStack,1);
					_pushStack(&_optionListStack,nodeIndex);
					_pushStack(&_linkNodeStack,nodeIndex);
					_pushStack(&_lastNodeStack, stackItem);
					continue;
				}else if(stackItem==1){
					//a new option level with preceding node.
					stackItem2 = _peekStackTop(&_linkNodeStack);
					if(_nodes[stackItem2].node)
						goto newOptionLevel;
				}
				
				stackItem = _peekStackTop(&_optionListStack);
				if(stackItem>=0 && _nodes[stackItem].optionStart){
					//create a dummy node between two successive [ tokens.
					_createOptionFakeNode(thisNode);
					_linkWithPreviousNodes(nodeIndex);
					_pushStack(&_linkNodeStack,nodeIndex);
					
					//create the actual node for [ now...
					nodeIndex++;
					_nodeUsed++;
					thisNode = &_nodes[nodeIndex];
					_initializeLinkGraphRowColumn(nodeIndex);
				}
newOptionLevel:
				//this is the first option in this new level
				_createOptionStartNode(thisNode, token, min_repeat, max_repeat);
				stackItem = _peekStackTop(&_linkNodeStack);
				_linkWithPreviousNodes(nodeIndex);

				_pushStack(&_lastNodeStack,stackItem);
				_pushStack(&_optionListStack,stackItem); //the option list terminating node
				_pushStack(&_optionListStack,nodeIndex); //this option.
				_pushStack(&_optionCountStack,1);
				_pushStack(&_linkNodeStack,nodeIndex);	
				break;
				
			case ']':
				_createOptionEndNode(thisNode, token);
				_linkWithPreviousNodes(nodeIndex);				
				stackItem = _peekStackTop(&_optionCountStack);
				if(stackItem==0)
					_linkAllOptions(nodeIndex);
				stackItem2 = _peekStackTop(&_optionCountStack);
				if(stackItem2>=0){
					_pokeStackTop(&_optionCountStack,--stackItem2);
					_pushStack(&_optionListStack,nodeIndex);	
				}
				_pushStack(&_linkNodeStack,nodeIndex);
				_popStack(&_lastNodeStack);
				break;
		}
	}

#ifdef DEBUG_CISCO_CMDPARSER					
	_printLinkGraph(__LINE__);					
#endif
	return nodeIndex;
}




//Get next possible candidate nodes recursively.
static int32 _getCandidateNodes(uint32 startNode){
	int32 end, accepted;
	struct node *thisNode;
	for(end = 0; end <= _nodeUsed; end++){
		thisNode = &_nodes[end];
		_nodes[end].checked=0;
		if(thisNode->keyword||thisNode->param||thisNode->endCommand){
			if(_linkGraph[startNode][end]){
				accepted = 0;
				//Check accepted tokens, don't select them again.
				if(end!=startNode){
					do{
						if (accepted==end)
							break;
						accepted=_nodes[accepted].nextAccepted;
					}while(accepted);
				}else if(!_nodes[startNode].selfRepeat ||
					_nodes[startNode].repeat >= _nodes[startNode].max_repeat){
					continue;
				}
				
				if(!accepted){
					int32 count;
					count = _candidate.count-1;
					//Check candidate list. Don't select same token twice.
					while(count>=0){
						if (_candidate.list[count]==end)
							break;
						count--;
					}
					if(count<0){
						_candidate.list[_candidate.count]=end;
						_candidate.count++;
						if(thisNode->selfRepeat&&thisNode->repeat<thisNode->min_repeat)
							return 0;
					}
				}
			}
		}else if(_linkGraph[startNode][end]){
			if(end==startNode)
				end++;
			_getCandidateNodes(end);
		}
	}
	return 0;
}


static int32 _isLastNode(uint32 startNode, uint32 terminateNode){
	int32 end, accepted;
	for(end = _nodeUsed; end >=0 ; end--){
		if(_linkGraph[startNode][end]){
			if (end==terminateNode)
				return 1;
			if (_nodes[end].keyword||_nodes[end].param)
				return 0;
			accepted = 0;
			do{
				if (accepted==end)
					break;
				accepted=_nodes[accepted].nextAccepted;
			}while(accepted);
			
			return _isLastNode(end, terminateNode);
		}
	}
	return 0;
}



static int32 _pathExist(uint32 from, uint32 to){
	int32 end;
	if (from==to)
		return 1;
	for(end = 0; end <= _nodeUsed; end++){
		if(_linkGraph[from][end]&&_pathExist(end, to))
			return 1;
	}
	return 0;

}


//	Return	*nodeIndex  		*token		Meaning
//	-1		0				0				B)Null cmd string. Expects first token
//	-n		lastNodeIndex		token[n]			C)n-1 tokens accepted, token[n] not found.
//	-(n+1)	lastNodeIndex		0				D)All token parsed. More needed
//	+n		firstNodeIndex		-				E1)Command accepted. 
//	+n		firstNodeIndex		token[n+1]		E2)Command accepted. Excess arguments present
//   n = accepted in this function. 
static int32 _processUserCommand( int32 totalNodes, int32 argc, int8 **argv, int32 *nodeIndex, int8 **token){
	int8 *thisToken;   //, *savedPtr;
	uint32 index,  lastNodeIndex=0, thisNodeIndex=0, firstAcceptedNodeIndex=0 ;
	int32 accepted=0, count=0;
	struct node *thisNode=NULL;

	memset(&_candidate, 0, sizeof(_candidate));

	if(argc==0){
		if(!_isLastNode(lastNodeIndex, totalNodes-1)){
			//Case B: A Null command string. 
			*token = NULL;
			*nodeIndex = 0; //Nothing accepted
			return -1; //Expects first token 
		}
		goto success;
	}

	//Parse str and delimit all tokens.
	//thisToken=strtok_r(cmdStr, " ", &savedPtr);
	thisToken = argv[0];
	while(count<argc && thisToken){

		//Lookup the link graph and find all candidate nodes. (Appeared nodes already excluded)
		_candidate.count = 0;
		_getCandidateNodes(lastNodeIndex);

		//Check if this token matches with any candidate.
		for(index=0; index<_candidate.count; index++){
			int32 tokenSize;
			thisNode= &_nodes[_candidate.list[index]];
			thisNodeIndex = _candidate.list[index];
			if (thisNode->keyword){
				//if(strncmp(thisToken, (int8 *)thisNode->token, thisNode->tokenSize)==0){
				if(strcmp(thisToken, (int8 *)thisNode->token)==0){
					if(thisNode->selfRepeat && thisNode->repeat>0){
						int8 *buffer;
						assert(IS4BYTEALIGNED((memaddr)_tempBuffer+_tempBufferUsed));
						buffer  =  (int8 *)_tempBuffer+_tempBufferUsed;
						*((uint32 *)buffer)=0; 
						_tempBufferUsed+=sizeof(uint32);
						buffer  =  (int8 *)_tempBuffer+_tempBufferUsed; //save string pointer here
						*((memaddr *)buffer)=(memaddr)thisNode->token;
						_tempBufferUsed+=sizeof(int8 *);						
					}
					break; //accepted
				}
			}else if(thisNode->param){
				_nextNewCmd=NULL;
				if(thisNode->selfRepeat && thisNode->repeat>0){
					uint8 *buffer;  int8 *size;
					assert(IS4BYTEALIGNED((memaddr)_tempBuffer+_tempBufferUsed));	
					size = (int8 *)_tempBuffer+_tempBufferUsed;
					*((uint32 *)size)=sizeof(uint32); //save default tokenSize here
					_tempBufferUsed+=sizeof(uint32);

					buffer  =  _tempBuffer+_tempBufferUsed; //save converted data here
					*((memaddr *)buffer)=(uint32)buffer;
					if(thisNode->max_repeat){
						int32 enumCount;
						if(thisNode->paramType->enumFunc){ //Must have a enumerate function if (1~) range is used for this parameter type
							enumCount = thisNode->paramType->enumFunc(0, NULL);
							thisNode->max_repeat = min((int32)thisNode->max_repeat, enumCount);
						}
						assert(thisNode->max_repeat>0);
					}
					tokenSize=thisNode->paramType->checkFunc((void **)&buffer, thisToken, thisNode->paramType->maxSize, &_nextNewCmd);

					if(tokenSize>0){
						*((uint32 *)size)=tokenSize; //update token size
						_tempBufferUsed+= (1+((tokenSize-1)>>2))<<2; // 4 byte alignment
					}else if(tokenSize==0){ //It's a string...
						*((uint32 *)size)=0; //update token size
						_tempBufferUsed+=sizeof(int8 *);						
					}else
						_tempBufferUsed-=sizeof(uint32);
						assert(IS4BYTEALIGNED(_tempBufferUsed));	
				}else	
					tokenSize=thisNode->paramType->checkFunc(&thisNode->token, thisToken, thisNode->paramType->maxSize, &_nextNewCmd);
				if(tokenSize>=0){
					assert(tokenSize<= thisNode->paramType->maxSize);
					thisNode->tokenSize = tokenSize;
					break;//accepted
				}
			}else if(_candidate.count==1){
				//No more optional candidate tokens. Terminate node is the only choice.
				//This is an excess token...
				assert(thisNode->endCommand);
				*token = thisToken;
				goto success;
			}
		}
		
		if(index==_candidate.count){//Case C: No match. Unaccepted token
			*token= thisToken;
			*nodeIndex = lastNodeIndex;
			return -(accepted+1);
		}

		if(_nextNewCmd){
			*nodeIndex = firstAcceptedNodeIndex;
			//get next user's token if any
			if(count+1<argc){
				*token= argv[count+1];
			}else
				*token=NULL;
			//clePrintf("Next cmd: %s  agument:'%s'\n", _nextNewCmd->cmdStr,  (int8 *)*token);
			//If *token=0: A new command should be parsed, but no more tokens.
			//If *token!=0:A new command should be parsed. *token is the first token for new command
			return accepted+1;
		}

		//this token accepted.
		if (lastNodeIndex==0)
			firstAcceptedNodeIndex = thisNodeIndex;

		if(thisNode->selfRepeat){
			if(!thisNode->repeat){
				_nodes[lastNodeIndex].nextAccepted = thisNodeIndex;
				thisNode->lastAccepted = lastNodeIndex;
			}				
			thisNode->repeat++;
			
			if (thisNode->repeat >= thisNode->max_repeat)
				_linkGraph[thisNodeIndex][thisNodeIndex]=0;
		}else{
			assert(lastNodeIndex!=thisNodeIndex);
			_nodes[lastNodeIndex].nextAccepted = thisNodeIndex;
			thisNode->lastAccepted = lastNodeIndex;
		}

		lastNodeIndex = thisNodeIndex;
		accepted++;

		//get next user's token
		if(count+1<argc)
			thisToken= argv[++count];
		else
			thisToken=NULL;
	}

	if(!_isLastNode(lastNodeIndex, totalNodes-1)){
		//Case D: All tokens consumed. Expects more...
		_candidate.count = 0;
		_getCandidateNodes(lastNodeIndex);
		assert(_candidate.count);
		*token= NULL;
		*nodeIndex = lastNodeIndex;
		return -accepted;
	}
	
	assert(firstAcceptedNodeIndex);
success:
	*nodeIndex = firstAcceptedNodeIndex;
	return accepted;
}



/*********************************************************************************
	Section 8:	Exported API implementation
**********************************************************************************/

void ciscoCmdParserGetCandidate(void *lastTokenNode){
	struct node *lastNode = (struct node *)lastTokenNode;
	assert(lastTokenNode!=NULL);
	_candidate.count=0;
	_getCandidateNodes( lastNode - &_nodes[0]);
}

int32 ciscoCmdParserGetCandidateTokenList(struct enumList *list, int32 totalEntries){
	int32 index, count;
	struct node *thisNode;

	count=0;
	for(	index = 0; count <totalEntries && index < _candidate.count; index ++, count++){
		thisNode =  &_nodes[_candidate.list[index]];
		if(thisNode->keyword || thisNode->endCommand){
			list[count].tokenName = (int8 *)thisNode->token;
			list[count].helpString= (int8 *)thisNode->helpString?thisNode->helpString: "";
		}else{
			int32 tokens;
			if(thisNode->paramType->enumFunc){
				if((tokens = thisNode->paramType->enumFunc(totalEntries-count, &list[count] ))>0){
					if(thisNode->selfRepeat){
						thisNode->max_repeat = min(tokens, (int32)thisNode->max_repeat);
						if(thisNode->repeat< thisNode->max_repeat)
							count+= tokens-1;
					}else
						count+= tokens-1;
					continue;				
				}else
					count-=1;
			}else if (thisNode->param&&thisNode->paramType->formatString){
				list[count].tokenName = (int8 *)thisNode->paramType->formatString;
				list[count].helpString= (int8 *)thisNode->helpString ?thisNode->helpString: "";
			}
		}
	}
	return count;
}


//If ciscoCmdParser() returns -1, pass the savedData returned by ciscoCmdParser() to print help message
int32 ciscoCmdParserShowHelp(struct enumList * list, int32 totalEntries){
	int32 index;
	uint32 uiStyle = cle_getUiStyle(reent_getCleUserId());
	assert(list && totalEntries>0);

	if(uiStyle & CLE_UI_STYLE_TERMINAL){
		for(	index = 0; index < totalEntries; index ++)
			if(list[index].tokenName)
				rtlglue_printf(CISCOCMD_HELPMSG_FORMAT, list[index].tokenName, list[index].helpString?list[index].helpString: "");
		rtlglue_printf("\n");
	}else{
		cle_writeOutputHeader(reent_getCleUserId(),CLE_RNBUF_TABLE,0,1);
		for(	index = 0; index < totalEntries; index ++)
			if(list[index].tokenName)
				cle_addItemToOutput(reent_getCleUserId(),CLE_RNBUF_DISPLAYSTR, list[index].tokenName, strlen(list[index].tokenName));
		cle_setOutputLen(reent_getCleUserId());
	}
	return SUCCESS;
}


//This function is called by cle_parser module, not by user's module
//Returns -1 if no more tokens.
//nextToken returns next token, may be a keyword or a converted data specific to previous related keyword
//savedData is value-result parameter.
int32 ciscoCmdGetNextToken(int8 **nextToken, int32 *size, int8 **savedData){
	struct node *node;
	assert(nextToken && size);
	assert(savedData!=NULL);

	if(!*savedData)
		return FAILED;
	node = (struct node *) *savedData;
	if(!node->selfRepeat){
		if(node->keyword)
			*size=0;
		else
			*size = node->tokenSize;
		*nextToken = node->token;
	}else if(node->checked==0){
		*size = node->tokenSize;
		*nextToken = node->token;
		assert(IS4BYTEALIGNED((memaddr)nextToken));
	}else if(node->checked<node->repeat){
		assert(IS4BYTEALIGNED((memaddr)_tempBuffer+_nonRepeatingTempBufferStart));
		*size = *((int32 *) ((memaddr)_tempBuffer+_nonRepeatingTempBufferStart));
		_nonRepeatingTempBufferStart+=sizeof(uint32 *);
		if(!*size){//it's a string
			*nextToken =*((int8 **)(_tempBuffer+_nonRepeatingTempBufferStart));
			_nonRepeatingTempBufferStart+=sizeof(uint32);			
		}else{//it's a converted data
			*nextToken = (int8 *)_tempBuffer+_nonRepeatingTempBufferStart;
			_nonRepeatingTempBufferStart+=ROUNDUP4(*size);
			assert(IS4BYTEALIGNED((memaddr)nextToken));
		}
	}
	
	if(node->selfRepeat){
		node->checked++;
		if(node->checked < node->repeat)
			*savedData = (int8 *) node;
		else 
			goto normal;
	}else{
normal:
		if(node->nextAccepted==0)
			*savedData = NULL;
		else
			*savedData = (int8 *)&_nodes[node->nextAccepted];
	}	
	return SUCCESS;
}



//Free all temp buffer used by this command.
int32 ciscoCmdParserFlushTmpBuffer(){
	if(_tempBufferUsed>0){
		assert(IS4BYTEALIGNED(_tempBufferUsed));
		_tempBufferUsed=_nonRepeatingTempBufferStart=0;
	}
	return 0;
}



//Free all temp buffer used by this command.
int32 ciscoCmdParserFree(){
	_candidate.count = _nodeUsed = _tempBufferUsed=_nonRepeatingTempBufferStart=0;
	_nextNewCmd = NULL;
	return 0;

}

//	Return	*saved  		*token		Meaning
//	0	0				0			B)Accepted without any arguments
//	-1	&_candidate		0			B1)Null cmd string. Expects first token
//	-n	&_candidate		lastToken	C)lastErrToken not found.
//	-n	&_candidate		0			D)All token parsed. More needed.
//	+n	&_nodes[x]		-			E)Command accepted. First token at _nodes[x]
//	+n	&_nodes[x]		lastToken	E2)Command accepted. First token at _nodes[x], Excess tokens at lastToken
//   n = accepted in this function.

int32 ciscoCmdParser( int32 argc, int8 **argv, cle_exec_t **thisCmd, int8 **savedData, int8 **token){
	int32 accepted;
	int32 totalNodes;
	int32 nodeIndex=0;
	int8 *lastToken=NULL;

	assert( thisCmd && *thisCmd && savedData);
	memset(&_nodes, 0, sizeof(_nodes));
	_thisCommand = *thisCmd;

	_nodeUsed=_splitCmdFormatTokens();
	assert(_nodeUsed>0);
	memset(&_linkGraph, 0, sizeof(_linkGraph));
	totalNodes=_createCommandGraph();
	assert(totalNodes >0 && totalNodes <MAX_TOKENS );
	assert(IS4BYTEALIGNED(_tempBufferUsed));
	_nonRepeatingTempBufferStart = _tempBufferUsed;
	accepted = _processUserCommand(totalNodes, argc, argv, &nodeIndex, &lastToken);

	if(accepted >=0){
		if(accepted) //Cmmand accepted with some tokens accepted
			*savedData = (int8 *)&_nodes[nodeIndex];
		else //command accepted without any tokens
			*savedData=NULL;
		*token = lastToken; //if non-NULL, we have excess tokens...
		if(_nextNewCmd)
			*thisCmd = _nextNewCmd;
	}else{
		*savedData =(void *)&_candidate;
		_candidate.count = 0;
		_getCandidateNodes(nodeIndex);
		if(!nodeIndex&&!lastToken){
			//Null command string or All tokens parsed. but More expected
			*token = NULL;
		}else
			*token = lastToken;
	}
	return accepted;
}

int8 *cle_argv[MAX_NODES];
int32 cle_cmdParser( int8 *cmdString, cle_exec_t *CmdFmt, int8 *delimiter){
	int argc=0, count;
	int8 *saved=NULL, *token=NULL;
	cmdString = strtok(cmdString,delimiter);
	if(!cmdString)
		return 0;
	cle_argv[0]= cmdString;
	argc=1;
	while(cmdString){
		cmdString=strtok(NULL, delimiter);
		cle_argv[argc]=cmdString;
		argc++;
	}
	count=ciscoCmdParser(argc, cle_argv, &CmdFmt,&saved, &token);
	if(count>0)
		CmdFmt->execFun(0, count, &saved);
	ciscoCmdParserFree();
	return count;
}

void ciscoCmdParserGetAllCandidates(){
	_candidate.count = 0;
	_getCandidateNodes(0);
}

static int8 _thisCmdSynopsisBuffer[MAX_CMD_TEMPBUFFER];
void ciscoCmdPrintSynopsis(int8 *help,int8 *cmdUsage){
	int8 *cmdFormatStr;
	int8 *cmds, *cmd, *savedPtr, *savedPtr2;
	uint32 uiStyle, count;
	assert(cmdUsage!=NULL);

	uiStyle = cle_getUiStyle(reent_getCleUserId());
	if(ISCLEARED(uiStyle ,CLE_UI_STYLE_TERMINAL))
		return;
	//Copy cmdUsage to a writable buffer.
	strncpy((int8 *)_thisCmdSynopsisBuffer, (const char *)cmdUsage,MAX_CMD_TEMPBUFFER);
	cmdFormatStr = (int8 *)_thisCmdSynopsisBuffer;
	if(help)
		rtlglue_printf("%s\n", help);
	//clePrintf("Synopsis:\n");
	//Parse cmdFormatStr and delimit all tokens.
	count=0;
	cmds=strtok_r(cmdFormatStr, "'", &savedPtr);
	while(cmds){
		cmd=strtok_r(cmds, " ", &savedPtr2);
		while(cmd){
			int32 index, len = strlen(cmd);
			for(index=0; index< len;index++)
				if(cmd[index]=='\t')
					cmd[index]=' ';
			//clePrintf("%s ", cmd);
			count++;
			cmd=strtok_r(NULL, " ", &savedPtr2);
  		};

		strtok_r(NULL, "'", &savedPtr);
		cmds=strtok_r(NULL, "'", &savedPtr);
	};
 
	rtlglue_printf("(Total tokens:%d)\n",count);
}


#ifdef DEBUG_CISCO_CMDPARSER
static void _printAllStacks(int8 *token){
	uint32 index;

	rtlglue_printf("\nCurrent Token: %s\n", token? token: "(NULL)");
	rtlglue_printf("LastNodeStack:");
	if(_lastNodeStack.top>=0)
		for(index=0; index <= _lastNodeStack.top; index++)
			rtlglue_printf("%c'%d'  ", (_nodes[_lastNodeStack.items[index]].token&&_nodes[_lastNodeStack.items[index]].tokenSize) ? *((int8 *)_nodes[_lastNodeStack.items[index]].token):'?', _lastNodeStack.items[index]);

	rtlglue_printf("\nLinkNodeStack:");
	if(_linkNodeStack.top>=0)
		for(index=0; index <= _linkNodeStack.top; index++)
			rtlglue_printf("%c'%d'  ", (_nodes[_linkNodeStack.items[index]].token&&_nodes[_linkNodeStack.items[index]].tokenSize) ? *((int8 *)_nodes[_linkNodeStack.items[index]].token): '?',_linkNodeStack.items[index] );

	rtlglue_printf("\noptionListStack:");
	if(_optionListStack.top>=0)
		for(index=0; index <= _optionListStack.top; index++)
			rtlglue_printf("%c'%d'  ",(_nodes[_optionListStack.items[index]].token&&_nodes[_optionListStack.items[index]].tokenSize) ? *((int8 *)_nodes[_optionListStack.items[index]].token):'?',_optionListStack.items[index]);

	rtlglue_printf("\noptionCountStack:");
	if(_optionCountStack.top>=0)
		for(index=0; index <= _optionCountStack.top; index++)
			rtlglue_printf("%2d  ", _optionCountStack.items[index]);

	rtlglue_printf("\n");
}

static void _printLinkGraph(uint32 lineNumber){
	uint32 index;

	rtlglue_printf("Link Graph (Line %d)\nX",lineNumber );
	for(index=0; index<MAX_NODES; index++){
		if(_nodes[index].param)
			rtlglue_printf("%c",	*_nodes[index].paramType->formatString);
		else if(_nodes[index].keyword)	
			rtlglue_printf("%c",*((int8 *)_nodes[index].token));
		else
			rtlglue_printf("%c",	(index &&index<_nodeUsed+1&&_nodes[index].token&&_nodes[index].tokenSize)?*((int8 *)_nodes[index].token):'?');
	}
	rtlglue_printf("\n");
	for(index=0; index<_nodeUsed+1; index++){
		int32 index2;
		if(_nodes[index].param)
			rtlglue_printf("%c",	*_nodes[index].paramType->formatString);
		else if(_nodes[index].keyword)	
			rtlglue_printf("%c",*((int8 *)_nodes[index].token));
		else
			rtlglue_printf("%c",	(index &&index<_nodeUsed+1&&_nodes[index].token&&_nodes[index].tokenSize)?*((int8 *)_nodes[index].token):'?');

		for(index2=0; index2<MAX_NODES; index2++){
			rtlglue_printf("%1d", _linkGraph[index][index2]);
		}
		rtlglue_printf("\n");
	}	
}
#endif




