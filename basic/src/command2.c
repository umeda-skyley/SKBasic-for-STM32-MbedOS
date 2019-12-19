/*
 *  command2.c      Additional commands for the Basic11 project
 *
 *  These commands will have to be rewritten in C using the
 *  'hc11 source below as a pseudo-code.
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"



 

/*
 *  cllist      list program to device #1
 */
void  cllist(void)
{
}

/*
 * CLLIST:   EQU    *
 *           LDAA   #$01         ; USE DEVICE #1 FOR HARD COPY LISTING.
 *           STAA   DEVNUM
 *           JSR    CLIST        ; GO DO A STANDARD LIST COMMAND.
 *           CLR    DEVNUM
 *           RTS                 ; RETURN.
 */


/*
 *  cautost      flag current program for autorun on next reset
 *
 *  Options are:
 *
 *  auto					report state of autostart flag
 *  auto  off				remove autostart flag
 *  auto  ee				mark program in EEPROM for autostart
 *  auto  fln				mark program in flash, area n, for autostart
 *
 *  This routine calls the target-specific autostart routine
 *  targetmarkautostart() with one of the following arguments:
 *
 *  AUTOST_OFF				remove autostart flag
 *  AUTOST_EE				mark program in EEPROM for autostart
 *  AUTOST_FL0				mark program in flash area 0 for autostart
 *  AUTOST_FL1				 "      "     "   "     "  1  "     "
 *  AUTOST_FL2				 "      "     "   "     "  2  "     "
 *  
 */
void  cautost(void)
{
	U8					flag;		// holds autostart flag on read

	skipspcs();						// move to argument, if any
	if (*ibufptr == EOL)			// if checking autostart value...
	{
		flag = targetreadautostart();	// see what's going on
		switch  (flag)				// based on autostart value...
		{
			case  AUTOST_OFF:		// if no autostart...
			pl_P(PSTR("Autostart not enabled."));
			break;

			case  AUTOST_EE:		// if EEPROM...
			pl_P(PSTR("Autostart from EEPROM."));
			break;

			case  AUTOST_FL0:		// if flash 0...
			case  AUTOST_FL1:		// or flash 1...
			case  AUTOST_FL2:		// or flash 2...
			pl_P(PSTR("Autostart from flash "));
			if (flag == AUTOST_FL0)  outbyte('0');
			else if (flag == AUTOST_FL1)  outbyte('1');
			else outbyte('2');
			outbyte('.');
			break;

			default:
			pl_P(PSTR("Illegal autostart value "));
			outdeci(flag);
			outbyte('.');
			break;
		}
	}
	else if (match(AUTOST_OFF_STR))  targetmarkautostart(AUTOST_OFF);
	else if (match(AUTOST_EE_STR))   targetmarkautostart(AUTOST_EE);
	else if (match(AUTOST_FL0_STR))  targetmarkautostart(AUTOST_FL0);
	else if (match(AUTOST_FL1_STR))  targetmarkautostart(AUTOST_FL1);
	else if (match(AUTOST_FL2_STR))  targetmarkautostart(AUTOST_FL2);
	else
	{
		pl_P(PSTR("Unknown AUTOST argument.  Must be OFF, EE, FL0, FL1, or FL2."));
		return;
	}
}




/*
 * CAUTOST:  EQU    *            ; SET AUTO START MODE FOR BASIC PROGRAM.
 *           LDAA   #$55         ; GET FLAG.
 * CAUTOST1: LDX    EEStart
 *           STAA   AUTOSTF,x    ; PROGRAM IT INTO THE EEPROM
 *           JSR    DLY10MS      ; WAIT WHILE IT PROGRAMS.
 *           RTS                 ; RETURN.
 */



/*
 *  cnoauto      remove autorun flag from EEPROM
 */
//void  cnoauto(void)
//{
//	targetmarkautostart(FALSE);
//}

/*
 * CNOAUTO:  EQU    *
 *           LDAA   #$FF
 *           BRA    CAUTOST1
 */



/*
 *  autoload      modify working registers to run program in EEPROM
 */
void  autoload(void)
{
}

/*
 * AUTOLOAD: EQU       *
 *            ldx      EESTART
 *            LDD      EESTART
 *            ADDD     #SSTART
 *            STD      BASBEG 
 *            LDD      EESTART
 *            ADDD     SBASEND,x
 *            ADDD     #SSTART
 *            STD      BASEND
 * *
 *            LDD      SVAREND,x
 *            SUBD     SVARBEG,x
 *            ADDD     RAMSTART
 *            STD      VAREND
 *            LDD      RAMSTART
 *            STD      VARBEGIN
 *            XGDY
 *            LDD      EESTART
 *            ADDD     SVARBEG,X
 *            XGDX
 *            BRA      CELOAD4
 */


/*
 *  cmd_free      display on console number of free bytes remaining
 *
 *  (Also return to calling routine??)
 */
void  bsc_cmd_free(void)
{
	nl();
	pl_P(PSTR("Program memory: "));
	outdeci(basmend - basend);			// show bytes free in non-volatile memory
	pl_P(PSTR(" bytes free"));
	nl();
	pl_P(PSTR("Variable memory: "));
	outdeci(varmend - varend);
	pl_P(PSTR(" bytes free"));
	nl();
	pl_P(PSTR("Dynamic memory pool (arrays): "));
	outdeci(dynmemend - strastg);
	pl_P(PSTR(" bytes free"));
	nl();
}

/*
 * CFREE:              EQU       *
 *           JSR       NL2
 *           LDD       VARMEND
 *           SUBD      STRASTG
 *           JSR       OUTDECI
 *           JSR       NL
 *           RTS
 */


/*
 *  cdump      display contents of all variables(??)
 */
void  cdump(void)
{
}

/*
 * CDUMP:              EQU       *
 * *         JSR       NL2                 ; PRINT TWO BLANK LINES.
 * *         CLR       DNAME+2             ; ZERO THE LAST BYTE OF THE VARIABLE NAME 'ARRAY'
 * *         LDX       VARBEGIN            ; POINT TO THE START OF THE VARIABLE TABLE.
 * *CDUMP2  LDAA       0,X                 ; GET AN ENTRY. IS IT THE END OF THE TABLE?
 * *        BNE        CDUMP3              ; YES. WE'RE DONE.
 *           RTS
 * *CDUMP3   LDAA      1,X                 ; NO. GET THE FIRST CHARACTER OF THE NAME.
 * *         STAA      DNAME
 * *         LDAA      2,X
 * *         STAA      DNAME+1
 * *         LDX       #DNAME
 * *         JSR       PL
 * *         LDAA      0,X                 ; GET THE VARIABLE TOKEN.
 * *         CMPA      #IVARTOK            ; IS IT AN INTEGER?
 * *         BEQ       CDUMP9              ; YES. DUMP ITS VALUE.
 * ;         CMPA      #IAVARTOK ; NO. IS IT AN INTEGER ARRAY?
 * ;         BNE       CDUMP99             ; NO.
 * *         LDD       3,X                 ; YES. GET THE POINTER TO THE ARRAY STORAGE. HAS IT BEEN DIMENSIONED?
 * *         BNE       CDUMP5              ; YES. GO PRINT ALL THE VALUES.
 * *         LDX       #UNDIM
 * *         JSR       PL
 * *CDUMP6   LDAB      #5
 * *         ABX
 * *         BRA       CDUMP2
 * *CDUMP5   PSHX                          ; SAVE THE POINTER TO THE VARIABLE TABLE.
 * *         XGDX                          ; POINT TO THE ARRAY STORAGE AREA.
 * *         LDD       0,X                 ; GET THE MAXIMUM SUBSCRIPT.
 * *         STD       SUBMAX
 * *         CLRA
 * *         CLRB
 * *         STD       SUBCNT
 * *CDUMP77  LDAA      #'('
 * *         JSR       OUTBYTE
 * *         LDD       SUBCNT
 * *         JSR       OUTDECI
 * *         LDX       #CPEQ
 * *         JSR       PL
 * *         INX
 * *         INX
 * *         LDD       0,X
 * *         JSR       OUTDECI
 * *         JSR       NL
 * *         LDD       SUBCNT
 * *         ADDD      #1
 * *         CMPD      SUBMAX
 * *         BHI       CDUMP88
 * *         STD       SUBCNT
 * *         LDX       #DNAME
 * *         JSR       PL
 * *         BRA       CDUMP77
 * *CDUMP88  PULX
 * *         BRA       CDUMP6
 * *CDUMP9   LDAA      #'='
 * *         JSR       OUTBYTE
 * *         LDD       3,X
 * *         JSR       OUTDECI
 * *         JSR       NL
 * *         BRA       CDUMP6
 * *
 * *
 * *UNDIM    FCB       '=[?]',0
 * *CPEQ     FCB       ')=',0
 */
 
