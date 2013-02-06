/*
 * RI_handler.c.c - Reserved instruction handler
 *            for the IA-7546-4080 processor.
 */

/*
** Copyright 1998 sunplus, Inc.
*/


#include "RI_exception.h"
#include "RI_handler.h"

#undef TRUE
#define TRUE (0 == 0)

#undef FALSE
#define FALSE (0 != 0)

static int isJumpInstruction(char *pEXCdata);
static Branch_t emulateBranch(char *pEXCdata);
static unsigned int   *computeBranchAddress (char *pEXCdata, Branch_t type);


int emulateIA7546RI(char *pEXCdata)
{
  register ULONG rs;
  register ULONG rt;
  ULONG va;
  ULONG mem;
  ULONG newPC = 0;
  UINT inst;
  unsigned int byte;
  int status;
  int branchDelay;
  ULONG bdAddr;
  //printk("RI\n");
  branchDelay = ((0 != (CAUSE_BD & _STKOFFSET(EXC_DATA_CAUSE))) |
		 isJumpInstruction(pEXCdata));

  if (branchDelay) 
  {
    newPC = (ULONG) computeBranchAddress (pEXCdata,
					  emulateBranch(pEXCdata));
   	//prom_printf("Old-pc=%08x, newPC=%08x\n", ((UINT *)(_STKOFFSET(EXC_DATA_EPC)) + (branchDelay?1:0)), newPC);
	}

  inst = *((UINT *)(_STKOFFSET(EXC_DATA_EPC)) + (branchDelay?1:0));
  bdAddr = ((UINT *)(_STKOFFSET(EXC_DATA_EPC)) + (branchDelay?1:0));
  /* In case the emulated zero register is somehow trashed. */
  _GPR_STKOFFSET(0) = 0;

  rs = _GPR_STKOFFSET(_RS_(inst));
  rt = _GPR_STKOFFSET(_RT_(inst));

  va = rs + (ULONG)((short)_OFFSET_(inst));
  byte = va & 3;

  status = 0;

  switch (_OP_(inst)) 
  {
  case 0x22:			/* lwl */
    //SBD_DISPLAY ("RI22", 0);
#ifdef __MIPSEB__
    mem = *(ULONG *)(va - byte);
    mem = mem << byte*8;

    rt = (rt & ~(-1UL << byte*8)) | mem;
#else /*__MIPSEL__ */
    mem = *(ULONG *)(va - byte);
    mem = mem << (3 - byte)*8;

    rt = (rt & ~(-1UL << (3 - byte)*8)) | mem;
#endif

    _GPR_STKOFFSET(_RT_(inst)) = rt;
    status = 1;

    break;

  case 0x26:			/* lwr */
    //SBD_DISPLAY ("RI26", 0);

#ifdef __MIPSEB__
    mem = *(ULONG *)(va - byte);
    mem = mem >> (3-byte)*8;

    rt = (rt & ~(-1UL >> (3-byte)*8)) | mem;
#else /* __MIPSEL__ */
    mem = *(ULONG *)(va - byte);
    mem = mem >> byte*8;

    rt = (rt & ~(-1UL >> byte*8)) | mem;
#endif
    _GPR_STKOFFSET(_RT_(inst)) = rt;
    status = 1;

    break;

  case 0x2A:			/* swl */
    //SBD_DISPLAY ("RI2A", 0);

#ifdef __MIPSEB__
    mem = *(ULONG *)(va - byte);
    mem = mem & ~(-1UL >> byte*8);

    rt = (rt >> byte*8) | mem;

#else /* __MIPSEL__ */
    mem = *(ULONG *)(va - byte);
    mem = mem & ~(-1UL >> (3-byte)*8);

    rt = (rt >> (3-byte)*8) | mem;
#endif
    *(ULONG *)(va - byte) = rt;
    status = 1;

    break;

  case 0x2E:			/* swr */
    //SBD_DISPLAY ("RI2E", 0);

#ifdef __MIPSEB__
    mem = *(ULONG *)(va - byte);
    mem = mem & ~(-1UL << (3-byte)*8);

    rt = (rt << (3-byte)*8) | mem;
#else /* __MIPSEL__ */
    mem = *(ULONG *)(va - byte);
    mem = mem & ~(-1UL << byte*8);

    rt = (rt << byte*8) | mem;
#endif

    *(ULONG *)(va - byte) = rt;
    status = 1;

    break;
  default:
	{
		printk("illegal Reserved BdAddr=%x\n",bdAddr);
		printk("stack=%x\n",(unsigned long)(pEXCdata));
		printk("panic!!!\n");
		for(;;);
	}
	
  }

  if (status) 
  {
    if (branchDelay) 
    {
      _STKOFFSET(EXC_DATA_EPC) = newPC;
    }
    else 
    {
      _STKOFFSET(EXC_DATA_EPC) += 4;
    }
  }

  return (status);
}

static int isJumpInstruction(char *pEXCdata)
{
  UINT *pc;
  UINT inst;

  pc = (UINT *)(_STKOFFSET(EXC_DATA_EPC));
  inst = *pc;

  switch (_OP_(inst)) 
  {
  case 0x00:			/* Special */
    if ((_OPS_(inst) == 0x08)	/* jr */
	| (_OPS_(inst) == 0x09))    /* jalr */
	{ 
      return TRUE;
    }
    return FALSE;
      
  case 0x02:			/* j */
  case 0x03:			/* jal */
    return TRUE;

  default:
    return FALSE;
  }
}


static Branch_t emulateBranch(char *pEXCdata)
{
  register ULONG rs;
  register ULONG rt;
  UINT *pc;
  UINT inst;
  Branch_t branchStatus;

  pc = (UINT *)(_STKOFFSET(EXC_DATA_EPC));

  inst = *pc;

  rs = _GPR_STKOFFSET(_RS_(inst));
  rt = _GPR_STKOFFSET(_RT_(inst));

  branchStatus = BRANCH_T_NONE;
  switch (_OP_(inst)) 
  {
  case 0x00:			/* Special */
    if (_OPS_(inst) == 0x08)  /* jr */
    {	
      branchStatus = BRANCH_T_REGISTER;
    }
    if (_OPS_(inst) == 0x09)  /* jalr */
    {	
      _GPR_STKOFFSET(_RD_(inst)) = (ULONG) (pc + 2);
      branchStatus = BRANCH_T_REGISTER;
    }
    break;

  case 0x03:			/* jal */
    _STKOFFSET(EXC_DATA_RA) = (ULONG) (pc + 2);
  case 0x02:			/* j */
    branchStatus = BRANCH_T_TARGET;
    break;

  case 0x04:			/* beq */
    if (rs == rt) branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x05:			/* bne */
    if (rs != rt) branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x06:			/* blez */
    if ((signed long) rs <= (signed long) 0)
      branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x07:			/* bgtz */
    if ((signed long) rs > (signed long) 0)
      branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x01:			/* regimm */
    switch(_RT_(inst)) 
    {
    case 0x10:			/* bltzal */
      _STKOFFSET(EXC_DATA_RA) = (ULONG) (pc + 2);
    case 0x00:			/* bltz */
      if ((signed long) rs < (signed long) 0)
	branchStatus = BRANCH_T_OFFSET;
      break;

    case 0x11:			/* bgezal */
      _STKOFFSET(EXC_DATA_RA) = (ULONG) (pc + 2);
    case 0x01:			/* bgez */
      if ((signed long) rs >= (signed long) 0)
	branchStatus = BRANCH_T_OFFSET;
      break;
    }
    break;

    /* Todo: bcxf and bcxt */

  case 0x10:			/* cop0 */
    branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x11:			/* cop1 */
    branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x12:			/* cop2 */
    branchStatus = BRANCH_T_OFFSET;
    break;

  case 0x13:			/* cop3 */
    branchStatus = BRANCH_T_OFFSET;
    break;
  }

  return branchStatus;
}


static UINT *computeBranchAddress (char *pEXCdata, Branch_t type)
{
  register ULONG rs;
  UINT *pc;
  UINT inst;
  signed int offset;

  pc = (UINT *)(_STKOFFSET(EXC_DATA_EPC));

  inst = *pc;

  switch (type) 
  {
  case BRANCH_T_OFFSET:
    offset = (signed short) (inst & 0xffff);
    return (pc + 1 + offset);

  case BRANCH_T_TARGET:
    offset = inst & 0x3ffffff;
    return (UINT *)(((ULONG)(pc + 1) & 0xf0000000) | (offset << 2));

  case BRANCH_T_REGISTER:
    rs = _GPR_STKOFFSET(_RS_(inst));
    return (UINT *)rs;

  case BRANCH_T_NONE:
    return (pc + 2);

  default:
    return (0);
  }
}
