/*
 *  command3.c      Additional commands for the SKBasic project
 *
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"

#if defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)

#include	"uart_interface.h"
#include 	"hardware.h"
#if defined(RL7023_BINDING)
#include 	"adf7023.h"
#include 	"adf15d4g.h"
#include 	"SelfLib.h"
#endif

#include 	"decentra_base.h"
#include 	"debug.h"
#include	"profile.h"
#include	"memory.h"
#include	"context.h"
#include	"routing.h"
#include 	"util.h"
#include 	"framework.h"
#include 	"selector.h"
#include 	"lookup.h"
#include 	"scheduler.h"
#include 	"misc.h"
#include 	"impl.h"
#include 	"rl7023_impl.h."

extern RuntimeContext 	runtime;

extern void _print_memory(MemoryContextPtr memory);
extern void _print_neighbor_table(InquiryContextPtr inq);
extern void _print_acl(Identity list[]);
extern void _print_neighbor_node(NodePtr node);
extern void _print_rt(RoutingTablePtr table);
extern void _print_rt_item(RoutingTableItemPtr item);
extern void _print_id(byte id[]);
extern void _print_clock(VClockPtr clock);
extern void _print_dv(LinkStatePtr dv);
extern void _print_priority_list(Identity list[]);
extern void _print_params(void);
extern void print_size(void);
extern void setup_params(void);
extern Boolean SaveParam(void);
extern Boolean LoadParam(void);
extern void Dump_Sreg(void);
extern void TT_RcAttSet( unsigned char dbm );
extern void reset_tx_stat(void);
extern UInt8 get_tx_stat(void);

void			cskinfo(void)
{
	int		i;
	skipspcs();
	if (*ibufptr == EOL)
	{
		_print("EINFO ");

		_print_hex(runtime.self.id.clue[0], 2);
		_print_hex(runtime.self.id.clue[1], 2);
		_print(" ");
		
		for( i = 0; i < MAX_NAME_LEN; i++ ){
			if( runtime.self.name[i] != 0 ){
				SK_putc(runtime.self.name[i]);
			}
		}
		_print(" ");

		_print_hex(runtime.self.port_num, 4);
		_print(" ");

		_print_hex(runtime.self.channel, 2);
		_print(" ");
		
		_print("\r\n");
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			cskreset(void)
{
	skipspcs();
	if (*ibufptr == EOL)
	{
#if defined(RL7023_BINDING)
		adf_init_15d4g(1);
#endif
		
		do_init_context(&runtime);
		
		setup_params();
		
		start_impl(&runtime);
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			csksave(void)
{
	skipspcs();
	if (*ibufptr == EOL)
	{
		Boolean ans;
		ans = SaveParam();
		
		if( ans == D_TRUE ){
			_print("OK\r\n");
		} else {
			_print("ER06\r\n");
		}
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			cskload(void)
{
	skipspcs();
	if (*ibufptr == EOL)
	{
		Boolean ans;
		ans = LoadParam();
		
		if( ans == D_TRUE ){
			_print("OK\r\n");
		} else {
			_print("ER06\r\n");
		}
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			cskrfctrl(void)
{
	skipspcs();
	if (isdigit(*ibufptr))
	{
		int		param = getdeci();
		switch(param){
			case 1:					//oAe±i?i?eoeM
#if defined(RL7023_BINDING)
				adf_rc_cmd(CMD_PHY_ON_15d4);
				adf_rc_cmd(CMD_EXIT_15d4_MODE);
				adf_reg_wr( 0x00D, 1 );
//				adf_reg_wr( 0x11A, 0x40 );
				adf_rc_cmd(CMD_PHY_TX);
				while(1){
					if(SK_getc() >= 0)	break;
				}
				adf_reg_wr( 0x00D, 0 );
				adf_rc_cmd(CMD_SYNC);
				adf_rc_cmd(CMD_PHY_ON);
				adf_rc_cmd(CMD_ENTER_15d4_MODE);
				adf_reg_wr(BB_ANTENNA_DIVERSITY_CFG, adf_reg_rd(BB_ANTENNA_DIVERSITY_CFG) | 0x01);
				adf_rc_cmd(CMD_PHY_RX_CCA_15d4);	
#endif
				break;
			case 2:					//oAe±ELEEEaEAeoeM
#if defined(RL7023_BINDING)
				adf_rc_cmd(CMD_PHY_ON_15d4);
				adf_rc_cmd(CMD_EXIT_15d4_MODE);
				adf_reg_wr( 0x00D, 3 );
//				adf_reg_wr( 0x11A, 0x40 );
				adf_rc_cmd(CMD_PHY_TX);
				while(1){
					if(SK_getc() >= 0)	break;
				}
				adf_reg_wr( 0x00D, 0 );
				adf_rc_cmd(CMD_SYNC);
				adf_rc_cmd(CMD_PHY_ON);
				adf_rc_cmd(CMD_ENTER_15d4_MODE);
				adf_reg_wr(BB_ANTENNA_DIVERSITY_CFG, adf_reg_rd(BB_ANTENNA_DIVERSITY_CFG) | 0x01);
				adf_rc_cmd(CMD_PHY_RX_CCA_15d4);	
#endif
				break;
			default:
				_print("FAIL ER06\r\n");
				break;
		}
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			cskrfreg(void)
{
	int		reg_addr = -1;
	skipspcs();
	if(*ibufptr == '$'){
		ibufptr++;
		reg_addr = (int)gethex();
	}else if(isdigit(*ibufptr)){
		reg_addr = (int)getdeci();
	}else{
		_print("FAIL ER05\r\n");
		return;
	}
	if((reg_addr >= 0) && (reg_addr < 0x400)){
		int		val = -1;
		skipspcs();
		if(*ibufptr == '$'){
			ibufptr++;
			val = (int)gethex();
		}else if(isdigit(*ibufptr)){
			val = (int)getdeci();
		}else{
			_print("ERFREG ");
			_print_hex( adf_reg_rd( (UInt16)reg_addr ), 2 );
			_print("\r\n");
			return;
		}
		if((val >= 0) && (val < 256)){
			adf_reg_wr( (UInt16)reg_addr, (UInt8)val );
		}else{
			_print("FAIL ER06\r\n");
		}
		
	}else{
		_print("FAIL ER06\r\n");
	}
	return;
}

void			cskver(void)
{
	skipspcs();
	if (*ibufptr == EOL)
	{
		_print("EVER "); _print((char*)VERSION_STR);
		
		_print("\r\nOK\r\n");
	}
	else
	{
		_print("FAIL ER05\r\n");
	}
}

void			cskpow(void)
{
	int		val = -1;
	skipspcs();
	if(*ibufptr == '$'){
		ibufptr++;
		val = (int)gethex();
	}else if(isdigit(*ibufptr)){
		val = (int)getdeci();
	}else{
		_print("FAIL ER05\r\n");
		return;
	}
	if((val >= 0) && (val <= 26)){
		TT_RcAttSet( (UInt8)val );
	}else{
		_print("FAIL ER06\r\n");
	}
	return;
}

void			csktable(void)
{
	skipspcs();
	if (isdigit(*ibufptr))
	{
		int		dump = getdeci();
		switch(dump){
			case 1:
				//_print("Neighbor:\r\n");	
				_print("ENEIGHBOR\r\n");
				_print_neighbor_table(&(runtime.inquiry_context));
				break;
			case 2:
				//_print("RT:\r\n");
				_print("ERT\r\n");
				_print_rt(&(runtime.self.routing_table));
				break;
			case 3:
				_print("EMEMORY\r\n");
				_print_memory( &(runtime.memory) );
				_print("free:");_print_hex( count_free_memory(&runtime.memory, &runtime), 2 );
				_print("\r\n");
				break;
			case 4:
				_print("EACL\r\n");
				_print_acl( runtime.self.acl_list );
				break;
			case 5:
				_print("EDV\r\n");
#if defined(RL7023_BINDING)
				_print_dv(&runtime.self.distance_vector);
#endif
				break;
			case 6:
				_print("EPRIORITY\r\n");
				_print_priority_list(runtime.router.priority_route);
				break;

			case 240:
#if defined(RL7023_BINDING)
				reset_tx_stat();
#endif
				break;
				
			case 249:
#if defined(RL7023_BINDING)
				adf_rc_cmd(CMD_SYNC);
				adf_rc_cmd(CMD_PHY_ON);
				adf_rc_cmd(CMD_ENTER_15d4_MODE);
				adf_reg_wr(BB_ANTENNA_DIVERSITY_CFG, adf_reg_rd(BB_ANTENNA_DIVERSITY_CFG) | 0x01);
				adf_rc_cmd(CMD_PHY_RX_CCA_15d4);	
#endif
				break;
	
			case 250:
#if defined(RL7023_BINDING)
				adf_rc_cmd(CMD_PHY_RX_CCA_15d4);
#endif
				break;
				
			case 251:{
#if defined(RL7023_BINDING)
				UInt8 stat;
				
				adf_status(&stat);
				_print_hex(stat, 2); _print("\r\n");
				_print_hex(get_tx_stat(), 2); _print("\r\n");
				_print_hex(9, 2); _print("\r\n");
#endif
				break;
			}
			
			case 252:{
#if defined(RL7023_BINDING)
				UInt8 rssi[2];
				rssi[0] = adf_reg_rd(BB_ANTENNA0_RSSI);
				rssi[1] = adf_reg_rd(BB_ANTENNA1_RSSI);
				_print_hex(rssi[0], 2); _print(" ");
				_print_hex(rssi[1], 2); _print("\r\n");
#endif
				break;
			}
			case 253:
				Dump_Sreg();
				break;
			case 254:
				_print("ECONTEXT\r\n");
				print_size();
				break;
			case 255:
				_print("EPARAMS\r\n");
				_print_params();
				break;
			default:
				_print("FAIL ER06\r\n");
				break;
		}
	}
	else
	{
		_print("FAIL ER05\r\n");
	}

}

#if defined(SKBASIC_CMD)
void			cexit(void)
{
	errcode = EXIT_BASIC;
}
#endif

#elif defined(SKBASIC_EMBEDDED)
void			cexit(void)
{
	errcode = EXIT_BASIC;
}
#endif
