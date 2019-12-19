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

// -------------------------------------------------
//   Compiler Options
// -------------------------------------------------
#include "compiler_options.h"


// -------------------------------------------------
//   Include
// -------------------------------------------------
#include	"skyley_type.h"
#include	"skyley_base.h"
#include	"hardware.h"


// -------------------------------------------------
//   Prototypes
// -------------------------------------------------
static void SK_IncrementEventTick(SK_UH tick);
static T_SK_MailboxList* SK_AllocLinkedList(void);
static void SK_ReleaseLinkedList(T_SK_MailboxList* list);
static void SK_RemoveFromList(T_SK_MailboxList *item);
static void SK_AddToTail(T_SK_MailboxList *item);


// -------------------------------------------------
//   変数
// -------------------------------------------------
SK_UW						gnSK_SystemTick;

static SK_UB				gaSK_CmdMemoryFlag		[ SK_CMD_MEMBLOCK_NUM									];
static SK_UB				gaSK_DataMemoryFlag		[ SK_DATA_MEMBLOCK_NUM									];
static SK_UB				gaSK_DataMemoryTag		[ SK_DATA_MEMBLOCK_NUM									];
static SK_UB				gaSK_BigDataMemoryFlag	[ SK_BIGDATA_MEMBLOCK_NUM								];

static SK_UH 				pad;

static SK_UB				gaSK_CmdMemory			[ (SK_CMD_MEMBLOCK_SIZE * SK_CMD_MEMBLOCK_NUM)			];
static SK_UB				gaSK_DataMemory			[ (SK_DATA_MEMBLOCK_SIZE * SK_DATA_MEMBLOCK_NUM)		];
static SK_UB				gaSK_BigDataMemory		[ (SK_BIGDATA_MEMBLOCK_SIZE * SK_BIGDATA_MEMBLOCK_NUM)	];

static T_SK_MailboxList		gaSK_MessageList		[ SK_MBX_NUM											];
static T_SK_MailboxList		*gpstSK_MessageStart;
static T_SK_MailboxList		*gpstSK_MessageEnd;


SK_UB SK_MemoryFlag(SK_VP item, SK_UB flag, SK_UB action) {
	SK_UW		i;

	if (item == NULL) return 0xff;
		
	i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_CmdMemory))) / SK_CMD_MEMBLOCK_SIZE);
	if ( (item >= (SK_VP)(gaSK_CmdMemory)) && (i < SK_CMD_MEMBLOCK_NUM) ) {
		if( action == 1 ){
			gaSK_CmdMemoryFlag[i] = flag; 
		}
		return gaSK_CmdMemoryFlag[i];
	} else {
		i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_DataMemory))) / SK_DATA_MEMBLOCK_SIZE);
		if ( (item >= (SK_VP)(gaSK_DataMemory)) && (i < SK_DATA_MEMBLOCK_NUM) ) {
			if( action == 1 ){
				gaSK_DataMemoryFlag[i] = flag; 
			}
			return gaSK_DataMemoryFlag[i];
		} else {
			i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_BigDataMemory))) / SK_BIGDATA_MEMBLOCK_SIZE);
			if ( (item >= (SK_VP)(gaSK_BigDataMemory)) && (i < SK_BIGDATA_MEMBLOCK_NUM) ) {
				if( action == 1 ){
					gaSK_BigDataMemoryFlag[i] = flag;
				}
				return gaSK_BigDataMemoryFlag[i];
			}
		}
	}
	
	return 0xff;
}


void SK_RemoveFromList(T_SK_MailboxList *list){
	if ( list->m_pNext != NULL ) { list->m_pNext->m_pPrev = list->m_pPrev; }
	if ( list->m_pPrev != NULL ) { list->m_pPrev->m_pNext = list->m_pNext; }
	if ( gpstSK_MessageStart == list ) { gpstSK_MessageStart = list->m_pNext; }
	if ( gpstSK_MessageEnd   == list ) { gpstSK_MessageEnd   = list->m_pPrev; }
}


void SK_AddToTail(T_SK_MailboxList *list){
	if ( gpstSK_MessageEnd != NULL ) {
		gpstSK_MessageEnd->m_pNext	= list;
		list->m_pPrev				= gpstSK_MessageEnd;
		list->m_pNext				= NULL;
		gpstSK_MessageEnd			= list;
	} else {
		list->m_pPrev				= NULL;
		list->m_pNext				= NULL;
		gpstSK_MessageStart			= list;
		gpstSK_MessageEnd			= list;
	}
}


// -------------------------------------------------
//   Initialize stack framework
// -------------------------------------------------

void SK_Initialize(void) {
	SK_UW		i;

	EVENT_LOCK();
	
	// init system time counter
	gnSK_SystemTick = 0;

	// clear message box
	gpstSK_MessageStart = NULL;
	gpstSK_MessageEnd   = NULL;
	for(i=0;i<SK_MBX_NUM;i++) {
		gaSK_MessageList[i].m_pItem = NULL;
		gaSK_MessageList[i].m_pPrev = NULL;
		gaSK_MessageList[i].m_pNext = NULL;
		gaSK_MessageList[i].m_nTimeStamp = 0;
		gaSK_MessageList[i].m_nLatency = 0;
		gaSK_MessageList[i].m_nRepeat = 0;
	}
	
	// init memory blocks
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		gaSK_CmdMemoryFlag[i] = 0;
	}
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		gaSK_DataMemoryFlag[i] = 0;
		gaSK_DataMemoryTag[i] = 0;
	}
	
	for(i=0;i<SK_BIGDATA_MEMBLOCK_NUM;i++) {
		gaSK_BigDataMemoryFlag[i] = 0;
	}

	EVENT_UNLOCK();
}


// -------------------------------------------------
//   Count up system timer
// -------------------------------------------------

void SK_IncrementTimeTick(SK_UH tick) {
	SK_IncrementEventTick(tick);
}


void SK_IncrementEventTick(SK_UH tick) {
	if( (0xFFFFFFFF - gnSK_SystemTick) <= tick ){
		gnSK_SystemTick = 0;
	} else {
		//system tick
		gnSK_SystemTick += tick;
	}
}


// -------------------------------------------------
//   Get system time counter
// -------------------------------------------------

SK_UW SK_GetLongTick(){
	return gnSK_SystemTick;
}


// -------------------------------------------------
//   Post a message to message box
// -------------------------------------------------

SK_ER SK_PostMessage(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item) {
	return SK_PostMessageL(id, resid, cmd, item, 0, 0);
}


// -------------------------------------------------
//   Message post with specified repeat times and target slot
// -------------------------------------------------

SK_ER SK_PostMessageL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item, SK_UH latency, SK_UB repeat) {
	T_SK_MailboxList *list;
	if (item == NULL) {
        return SK_E_ER;
    }

	EVENT_LOCK();

	list = SK_AllocLinkedList();
	if (list == NULL) {
		EVENT_UNLOCK();
		return SK_E_ER;
	}
	
	list->m_pItem		= item;
	list->m_nMBXID		= id;
	list->m_nResMBXID	= resid;
	list->m_nCommand	= cmd;
	list->m_nTimeStamp 	= gnSK_SystemTick;
	list->m_nLatency 	= latency;
	list->m_nRepeat		= repeat;

	if( latency != 0 ){
		SK_MemoryFlag(item, 0x03, FLAG_SET);
	}

	SK_AddToTail(list);

	EVENT_UNLOCK();

	return SK_E_OK;
}


// -------------------------------------------------
//   Retrieve a message
// -------------------------------------------------

SK_ER SK_GetMessage(SK_UB id,SK_UB *resid,SK_UB *cmd,SK_VP *item) {
	T_SK_MailboxList *list;

	EVENT_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
	
		if (list->m_nMBXID == id) {
			SK_UW diff;
			diff = (SK_UW)(gnSK_SystemTick - list->m_nTimeStamp);

			*cmd	= list->m_nCommand;
			*item	= list->m_pItem;
			*resid	= list->m_nResMBXID;
			
			if( list->m_nLatency == 0 ){
				SK_RemoveFromList(list);
				SK_ReleaseLinkedList(list);

				EVENT_UNLOCK();
				return SK_E_OK;
			} else if( diff > list->m_nLatency ){
				if( list->m_nRepeat <= 1 ){
					SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
					SK_RemoveFromList(list);
					SK_ReleaseLinkedList(list);
				} else {
					SK_RemoveFromList(list);
					SK_AddToTail(list);
					list->m_nRepeat--;
					list->m_nTimeStamp = gnSK_SystemTick;
				}

				EVENT_UNLOCK();
				return SK_E_OK;
			}
		}
		list = list->m_pNext;
	}

	*cmd	= 0;
	*item	= NULL;
	*resid	= 0;

	EVENT_UNLOCK();
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//   Retrieve a message with cmd id
// -------------------------------------------------

SK_ER SK_GetMessageByCmd(SK_UB id,SK_UB *resid,SK_UB cmd,SK_VP *item) {
	T_SK_MailboxList *list;

	EVENT_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
		if ((list->m_nMBXID == id) && (list->m_nCommand == cmd)) {
			SK_UW diff;
			diff = (SK_UW)(gnSK_SystemTick - list->m_nTimeStamp);
		
			*item	= list->m_pItem;
			*resid	= list->m_nResMBXID;
			
			if( list->m_nLatency == 0 ){
				SK_RemoveFromList(list);
				SK_ReleaseLinkedList(list);

				EVENT_UNLOCK();
				return SK_E_OK;
			} else if( diff > list->m_nLatency ){
				if( list->m_nRepeat <= 1 ){
					SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
					SK_RemoveFromList(list);
					SK_ReleaseLinkedList(list);
				} else {
					SK_RemoveFromList(list);
					SK_AddToTail(list);
					list->m_nRepeat--;
					list->m_nTimeStamp = gnSK_SystemTick;
				}

				EVENT_UNLOCK();
				return SK_E_OK;
			}
		}
		list = list->m_pNext;
	}

	*item	= NULL;
	
	EVENT_UNLOCK();
			
	return SK_E_TMOUT;
}


// -------------------------------------------------
//   Free linked list for memory block
// -------------------------------------------------

T_SK_MailboxList* SK_AllocLinkedList() {
	SK_UW		i;

	for(i=0;i<SK_MBX_NUM;i++) {
		if (gaSK_MessageList[i].m_pItem == NULL) {
			return &gaSK_MessageList[i];
		}
	}
	return NULL;
}


void SK_ReleaseLinkedList(T_SK_MailboxList* list) {
	list->m_pItem = NULL;
	list->m_nLatency = 0;
	list->m_nRepeat = 0;
}


// -------------------------------------------------
//   Allocate small memory block
// -------------------------------------------------


SK_ER SK_AllocBigDataMemory(SK_VP *item) {
	SK_UW		i;

	EVENT_LOCK();

	for(i=0;i<SK_BIGDATA_MEMBLOCK_NUM;i++) {
		if (gaSK_BigDataMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_BigDataMemory[(SK_UW)SK_BIGDATA_MEMBLOCK_SIZE*(SK_UW)i]);
			gaSK_BigDataMemoryFlag[i] = 1;

			EVENT_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;

	EVENT_UNLOCK();

	return SK_E_TMOUT;
}

SK_ER SK_AllocCommandMemory(SK_VP *item) {
	SK_UW		i;

	EVENT_LOCK();
	
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		if (gaSK_CmdMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_CmdMemory[(SK_UW)SK_CMD_MEMBLOCK_SIZE*(SK_UW)i]);
			gaSK_CmdMemoryFlag[i] = 1;
			
			EVENT_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;
	
	EVENT_UNLOCK();
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//    Allocate large memory block
// -------------------------------------------------

SK_ER SK_AllocDataMemory(SK_VP *item) {
	SK_UW		i;

	EVENT_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_DataMemory[(SK_UW)SK_DATA_MEMBLOCK_SIZE*(SK_UW)i]);
			gaSK_DataMemoryFlag[i] = 1;	
			gaSK_DataMemoryTag[i] = 255;
			
			EVENT_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;
	
	EVENT_UNLOCK();
	
	return SK_E_TMOUT;
}


SK_ER SK_AllocDataMemoryWith(SK_VP *item, SK_UB tag) {
	SK_UW i;

	EVENT_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_DataMemory[(SK_UW)SK_DATA_MEMBLOCK_SIZE*(SK_UW)i]);
			gaSK_DataMemoryFlag[i] = 1;	
			gaSK_DataMemoryTag[i] = tag;
			
			EVENT_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;

	EVENT_UNLOCK();
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//   Free meomry block (large, small, big)
// -------------------------------------------------

SK_ER SK_FreeMemory(SK_VP item) {
	SK_UW i;
	
	EVENT_LOCK();

	if (item == NULL) {
		EVENT_UNLOCK();
		return SK_E_ER;
	}
	
	i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_CmdMemory))) / SK_CMD_MEMBLOCK_SIZE);
	if ( (item >= (SK_VP)(gaSK_CmdMemory)) && (i < SK_CMD_MEMBLOCK_NUM) ) {
		if( gaSK_CmdMemoryFlag[i] <= 2 ){
			gaSK_CmdMemoryFlag[i] = 0;
			EVENT_UNLOCK();
			return SK_E_OK;
		}
	} else {
		i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_DataMemory))) / SK_DATA_MEMBLOCK_SIZE);
		if ( (item >= (SK_VP)(gaSK_DataMemory)) && (i < SK_DATA_MEMBLOCK_NUM) ) {
			if( gaSK_DataMemoryFlag[i] <= 2 ){
				gaSK_DataMemoryFlag[i] = 0;
				gaSK_DataMemoryTag[i] = 0; //20130521 add
				EVENT_UNLOCK();
				return SK_E_OK;
			}
		} else {
			i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_BigDataMemory))) / SK_BIGDATA_MEMBLOCK_SIZE);
			if ( (item >= (SK_VP)(gaSK_BigDataMemory)) && (i < SK_BIGDATA_MEMBLOCK_NUM) ) {
				if( gaSK_BigDataMemoryFlag[i] <= 2 ){
					gaSK_BigDataMemoryFlag[i] = 0;
					EVENT_UNLOCK();
					return SK_E_OK;
				}
			}
		}
	}
	
	EVENT_UNLOCK();
	
	return SK_E_ER;	
}


SK_ER SK_FreeMemoryTag(SK_UB tag) {
	SK_ER err;
	
	EVENT_LOCK();
	
	err = SK_FreeMemoryTagNoLock(tag);

	EVENT_UNLOCK();
	
	return err;
}


SK_ER SK_FreeMemoryTagNoLock(SK_UB tag) {
	SK_UW i;

	for( i = 0; i < SK_DATA_MEMBLOCK_NUM; i++ ){
		//20130521 mod
		if( gaSK_DataMemoryTag[i] == tag && gaSK_DataMemoryFlag[i] != 0 ){
			gaSK_DataMemoryTag[i] = 0;
			gaSK_DataMemoryFlag[i] = 0; //20130521 add

			return SK_E_OK;
		}
	}

	return SK_E_ER;
}


// -------------------------------------------------
//   Count free memory block space
// -------------------------------------------------

SK_UB SK_CountFreeBigMemory(void){
	SK_UH i, cnt;
	cnt = 0;

	EVENT_LOCK();

	for(i=0;i<SK_BIGDATA_MEMBLOCK_NUM;i++) {
		if (gaSK_BigDataMemoryFlag[i]==0) {
			cnt++;
		}
	}

	EVENT_UNLOCK();

	return cnt;
}


SK_UB SK_CountFreeDataMemory(void){
	SK_UW i, cnt;
	cnt = 0;
	
	EVENT_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			cnt++;
		}
	}
	
	EVENT_UNLOCK();
	
	return cnt;	
}


SK_UB SK_CountFreeMemoryTag(SK_UB tag){
	SK_UW i, cnt;
	cnt = 0;
	
	EVENT_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryTag[i]==tag) {
			cnt++;
		}
	}
	
	EVENT_UNLOCK();
	
	return cnt;	
}


SK_UB SK_CountFreeCommandMemory(void){
	SK_UW i, cnt;
	cnt = 0;
	
	EVENT_LOCK();
	
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		if (gaSK_CmdMemoryFlag[i]==0) {
			cnt++;
		}
	}
	
	EVENT_UNLOCK();
	
	return cnt;	
}


// -------------------------------------------------
//   Stop delayed execution of a post message
// -------------------------------------------------

SK_ER SK_StopMessage(SK_VP item){
	T_SK_MailboxList *list;

	EVENT_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
		if (list->m_pItem == item) {
			SK_MemoryFlag(item, 0, FLAG_SET); //2nd bit and 1st bit should be clear

			SK_RemoveFromList(list);
			SK_ReleaseLinkedList(list);
			
			EVENT_UNLOCK();
			return SK_E_OK;
			
		}
		list = list->m_pNext;
	}
	
	EVENT_UNLOCK();
	
	return SK_E_TMOUT;
}


