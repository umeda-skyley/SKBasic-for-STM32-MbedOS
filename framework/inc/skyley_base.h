/**
    Copyright (c) 2019 Skyley Networks, Inc. All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the Institute nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#ifndef __skyley_base_h__
#define __skyley_base_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include	"compiler_options.h"
#include	"skyley_type.h"

// -------------------------------------------------
//   定数
// -------------------------------------------------

#define		SK_MBX_NUM					(16L)		// total number of message box slots
#define		SK_CMD_MEMBLOCK_SIZE		(40L)		// Block size for command memory pool
#define		SK_CMD_MEMBLOCK_NUM			(512L)		// Total number of command memory pool block
#define		SK_DATA_MEMBLOCK_SIZE		(4096L)		// Block size for data memory pool
#define		SK_DATA_MEMBLOCK_NUM		(2L)		// Total number of data memory pool block
#define 	SK_BIGDATA_MEMBLOCK_SIZE	(8192L)		//
#define 	SK_BIGDATA_MEMBLOCK_NUM		(2L)

// Layer ID
#define		SK_LAYER_BASE				0
#define		SK_LAYER_APL				250

// timeout
#define 	SK_MSG_TIMEOUT				5000		// Internal timeout


// -------------------------------------------------
//   コマンド番号
// -------------------------------------------------

enum	{
	SK_DATA_REQUEST_CMD,
	SK_DATA_CONFIRM_CMD,
	SK_DATA_INDICATION_CMD,
};


// -------------------------------------------------
//   typedef
// -------------------------------------------------

struct SK_MailboxList {
	struct SK_MailboxList*		m_pNext;			// Pointer to next message
	struct SK_MailboxList*		m_pPrev;			// Pointet to previous message
	SK_UB						m_nMBXID;			// ID of message box
	SK_UB						m_nCommand;			// Command ID
	SK_UB						m_nResMBXID;		// ID of receiver message box of response
	SK_VP						m_pItem;			// Pointer to contents
	
	SK_UW						m_nTimeStamp;		// Last time stamp
	SK_UH						m_nLatency;			// Delay time for continuous execution
	SK_UB						m_nRepeat;			// Number of repeat for continuous execution
};
typedef struct SK_MailboxList T_SK_MailboxList;


#ifndef SK_STATESTART
// -------------------------------------------------
//   Macros for state machine procedure
// -------------------------------------------------
// 1.Describe SK_STATESTART(***) in global part. *** is a label, ex. "MAC", "NWK"
// 2.Define SK_STATEADD(***,Number) at beginning of a function
// 3.Define SK_STATEEND(***) at end of a function
// 4.Then macro can be SK_SLEEP(time,***,Number) for instance, here *** is a name of label defined in part 1

	
#define SK_STATESTART(label)									\
	static SK_UB 	gnSK_State_##label		= 0;				\
	static SK_UW 	gnSK_Time_##label		= 0


#define SK_STATEADD(label,num)			if (gnSK_State_##label == num) { goto StateJump_##label##num; }


#define SK_STATEEND(label)				StateEnd_##label:


#define SK_SLEEP(time,label,num)								\
{																\
StateJump_##label##num:											\
	if (gnSK_State_##label == num) {							\
		if ((SK_UW)((SK_UW)gnSK_SystemTick - (SK_UW)gnSK_Time_##label)<time) {	\
			goto StateEnd_##label;								\
		} else {												\
			gnSK_State_##label = 0;								\
		}														\
	} else {													\
		gnSK_Time_##label = gnSK_SystemTick;					\
		gnSK_State_##label = num;								\
		goto StateEnd_##label;									\
	}															\
}


#define SK_WAITFOR(time,flg,label,num)							\
{																\
StateJump_##label##num:											\
	if (gnSK_State_##label == num) {							\
		if ( !(flg) && ((SK_UW)((SK_UW)gnSK_SystemTick - (SK_UW)gnSK_Time_##label)<time) ) {	\
			goto StateEnd_##label;								\
		} else {												\
			gnSK_State_##label = 0;								\
		}														\
	} else {													\
		gnSK_Time_##label = gnSK_SystemTick;					\
		gnSK_State_##label = num;								\
		goto StateEnd_##label;									\
	}															\
}

#define SK_INITSTATE(label)						\
{												\
	gnSK_State_##label		= 0;				\
	gnSK_Time_##label		= 0;				\
}		

#define SK_RETURN_STATE(label)									\
{																\
	if(gnSK_State_##label == 0){								\
		return TRUE;											\
	} else { 													\
		return FALSE;											\
	}															\
}
#endif


#define SK_GETSTATE(label) 			(gnSK_State_##label)
#define SK_SETSTATE(X, label) 		{ gnSK_State_##label = X; }
#define SK_PUSHSTATE(X, Y, label) 	{ gnSK_State_##label = X; gnSK_State_Next_##label = Y; }
#define SK_NEXTSTATE(label) 		{ gnSK_State_##label = gnSK_State_Next_##label; }


// -------------------------------------------------
//   Endian abstraction
// -------------------------------------------------

#define MAC_SET_WORD(ptr, wrd)			{ ((SK_UB *)(ptr))[0] = (SK_UB)((SK_UH)(wrd) & 0xFF); ((SK_UB *)(ptr))[1] = (SK_UB)(((SK_UH)(wrd) >> 8) & 0xFF); };
#define MAC_SET_LONG(ptr, dwrd)			{ ((SK_UB *)(ptr))[0] = (SK_UB)((SK_UW)(dwrd) & 0xFF); ((SK_UB *)(ptr))[1] = (SK_UB)(((SK_UW)(dwrd) / 0x100L) & 0xFF); ((SK_UB *)(ptr))[2] = (SK_UB)(((SK_UW)(dwrd) / 0x10000L) & 0xFF); ((SK_UB *)(ptr))[3] = (SK_UB)(((SK_UW)(dwrd) / 0x1000000L) & 0xFF); };
#define MAC_GET_WORD(ptr)				((SK_UH)(((SK_UB *)(ptr))[0]) + (SK_UH)(((SK_UB *)(ptr))[1] * 0x100))
#define MAC_GET_LONG(ptr)				((SK_UW)((SK_UW)(((SK_UB *)(ptr))[0])) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[1]) * 0x100) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[2]) * 0x10000L) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[3]) * 0x1000000L))


// -------------------------------------------------
//   Global variables
// -------------------------------------------------
// System timer
extern SK_UW	gnSK_SystemTick;


// -------------------------------------------------
//   Utility functions
// -------------------------------------------------

void SK_Initialize(void);
void SK_IncrementTimeTick(SK_UH tick);
SK_UW SK_GetLongTick(void);


// -------------------------------------------------
//   Message boxes
// -------------------------------------------------

SK_ER SK_PostMessage(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item);
SK_ER SK_PostMessageL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item, SK_UH latency, SK_UB repeat);
SK_ER SK_GetMessage(SK_UB id,SK_UB *resid,SK_UB *cmd,SK_VP *item);
SK_ER SK_GetMessageByCmd(SK_UB id,SK_UB *resid,SK_UB cmd,SK_VP *item);
SK_ER SK_StopMessage(SK_VP item);


// -------------------------------------------------
//   Maitain memory block
// -------------------------------------------------

SK_ER SK_AllocCommandMemory(SK_VP *item);
SK_ER SK_AllocDataMemory(SK_VP *item);
SK_ER SK_AllocDataMemoryWith(SK_VP *item, SK_UB tag);
SK_ER SK_AllocBigDataMemory(SK_VP *item);
SK_ER SK_FreeMemory(SK_VP item);
SK_ER SK_FreeMemoryL(SK_VP item, SK_BOOL force);
SK_ER SK_FreeMemoryTag(SK_UB tag);
SK_ER SK_FreeMemoryTagNoLock(SK_UB tag);


// -------------------------------------------------
//   Count number of free memory area
// -------------------------------------------------

SK_UB SK_CountFreeBigMemory(void);
SK_UB SK_CountFreeDataMemory(void);
SK_UB SK_CountFreeCommandMemory(void);
SK_UB SK_CountFreeMemoryTag(SK_UB tag);


// -------------------------------------------------
//   Set/Get condition flag of memory pool
// -------------------------------------------------
#define FLAG_GET 0
#define FLAG_SET 1

SK_UB SK_MemoryFlag(SK_VP item, SK_UB flag, SK_UB action);


// -------------------------------------------------
//   Stack init and main loop
// -------------------------------------------------

void SK_Base_Init(void);
void SK_Base_Main(void);


#ifdef __cplusplus
}
#endif

#endif
