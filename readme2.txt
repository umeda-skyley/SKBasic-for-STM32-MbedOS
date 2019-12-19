File: readme2.txt
Karl Lunt
Revised: 9 May 12



            Notes on the internals of KLBasic

This file contains information essential for extending and maintaining
the KLBasic program.  It is intended for use by those hobbyists who wish
to add features to KLBasic or to fix bugs in KLB.

KLBasic is a target-resident Basic interpreter.  You load the executable
on a target device and run KLB.  Using a console device of some kind,
usually a serial comm port on a PC, you can create, edit, and run programs
in a variant of the Basic language.  KLB supports 32-bit integer math,
storage and retrieval of program source from the target device, and automatic
execution of a selected KLB program on power-up.

I (Karl Lunt) have decided to release the source code for my KLBasic
project to the community.  I've taken KLB quite a way from its origins
in Gordon Doughman's BASIC11, but I don't have much time any more to
devote to KLB.  So I've decided to make the source available to others,
in the hope that they would create forks of KLB, to extend and support
the project.

Please note that the core design of KLBasic was driven by Gordon's
original 68hc11 design.  I have tried hard to stay faithful to Gordon's
original design, even when that might not result in the best C code.
I've always intended to do a ground-up redesign, but I don't know
when or if I will ever get to that.  It's better for the community
to have a shot at it than leave KLBasic stuck in its current form.

Please refer to Gordon's original release document (readme.txt), which
should have been included in the same package of files as this one.



--- History

KLB was based on Gordon Doughman's original BASIC11, written in 68hc11
assembly language.  KLB itself is written in C, in an effort to make the
source code as target-independent as possible.

KLB has been ported to several different Atmel AVR devices, notably the
ATmega1284p and ATxmega128a1.



--- Future of KLBasic

I don't know if I will make any significant additions to KLBasic.  What you
see here represents several years of sporadic effort, and I'm easily
distracted.  :-)

If you decide to modify KLBasic, please rename it before you start editing.
Choose a name for your new Basic interpreter and treat your work as a
project completely separate from the original KLBasic (a fork).

I ask this because I don't want conflicts between any updates I might do
to KLBasic and work you might have done on your version.

HOWEVER, please ensure that your source files include acknowledgement
of Gordon's original BASIC11 and my KLBasic as the foundation of your code!



--- basiccore

KLBasic consists of two separate collections of files.  The first group,
called basiccore, consists of the target-independent common core routines.
This group contains the following files:

basiclb1.c
basiclb2.c
basiclb3.c
basiclb4.c
basiclb5.c
command1.c
command2.c
inits.c
leditor.c
rexpres.c
runtime1.c
runtime2.c
runtime3.c
stacks.c

defines.h
funcs.h
stacks.h

With the exception of the stacks.* files, this group of files is pretty
much Gordon's original 68hc11 assembly language source converted to C.
In fact, I have retained Gordon's original assembly source as comments within
these files, both for guidance during design and for historical reference.

The stacks.* files are my contribution, providing a generic set of stack
operators.

Note that there is no project build or make files for the basiccore files.
You do not build basiccore.  Instead, these source files are included as
source files when you build a target-specific project.



--- Target-specific project files

The second group of files is target-specific and contains two source
files, named to reflect the type of target.  For example, the project
for a generic ATmega target contains the following two source files:

targetavr.c
targetavr.h

The target-specific file collection also includes make files and project
files for building the final KLBasic executable for a target.  The
actual project files will vary, based on your build environment.  For
example, the targetavr files include an AVRStudio4 set of project files
(targetavr.aps and targetavr.aws) plus make files for each of the
three possible targets (Makefile_90can128, Makefile_128, and Makefile_1284p).

Note that you are free to use any IDE or build environment you want to
build and maintain KLBasic.  I use AVRStudio4 for building ATmega
executables, but you could just as easily use Visual Studio and avr-gcc
with some tweaking.

Please refer to the above cited Makefile_*** files for examples of how
to pull in the core source files and how to build a targeted version
of KLBasic.



--- basiccore issues

The basiccore source files must not contain any references to target-specific
features.  For example, you should not put anything in the basiccore source
files that depend on a UART or a memory device or an I/O port or device.  All
such references should appear only in the target-specific fileset.

In some rare cases, this isn't possible, and I was forced to make compile-
time allowances.  One notable instance is avr-gcc's use of 16-bit pointers.
Weird things happen if the core code assumes that a pointer is 32-bits wide.
You will find places in the core code that are bracketed with '#ifdef AVR',
to guard against such issues.

But in general, the core routines must not make assumptions as to features
on the target.  For example, the core routines are supposed to process the
LOAD and SAVE commands, which clearly requires knowledge of the target
hardware.  In these cases, the core routines simply call target-specific
routines named cload() and csave() to do the actual work.



--- Target-specific issues

The target-specific source files must provide a wide range of target-
specific functions and support.  The target***.c file is where you put
ALL functions that depend on features of the target.

You should review the contents of the included targetavr.c file for
examples of the functions normally included in a target source file.
For reference, here are some of the routines found in targetavr.c that
are invoked by the core:

main()                 top-level entry point following reset
targetinitvars()       sets up program RAM and pointers
getconst()             searches table of constants in flash
targetgetportaddr()    returns address of a port
targetcheckautorun()   determines if the target runs a program on reset
targetmarkautostart()  tags a stored program for autostart
targetwriteeeprom**()  writes 8, 16, or 32-bit value to on-target EEPROM
targetreadeeprom**()   reads 8, 16, or 32-bit value from on-target EEPROM
csave()                saves current program to target storage
cload()                loads program from target storage
targetgets()           gets string (with editing) from console
nl()                   outputs newline to console
pl()                   outputs null-terminated string to console
outbyte_xlate()        output a character, with C-translation, to console
outbyte()              output a character to console
inbyte()               get a character from the console
getioport()            search table of ports for requested port by name
targetwriteport()      modify contents of a port
targetreadport()       read contents of a port
targetgetportname()    return string containing port name
targetgetflashprogramaddr()   return address of a flash file
targetcopyfromflash()  copy N bytes from a flash file to RAM
iodevinit()            initialize target I/O devices, including console

Note that the names given above are expected by the core routines
and must appear as shown.  For example, if one of the core routines
needs to read a null-terminated string from the console, it always
calls targetgets().

Note also that some targets will implement the underlying I/O for the
above routines differently.  For example, the ATxmega128a1 version of
KLBasic uses an external serial flash device for storing program files,
while the ATmega1284p version uses blocks of on-chip flash memory for
that function.  In both cases, flash files are written by calling
csave(), but the files are stored in different memories and those memories
is accessed in different ways.



--- Design of the interpreter

KLBasic consists of a top-level loop, found in funcion basic_main() in
file basiclb1.c.  This loop is:

	while(1)						// do forever
	{
		if (immid)					// if we are in immediate mode...
		{
			outrdy();				// send the ready message
		}

		immid = 0;					// not in immediate mode (yet)
		errcode = 0;				// clear error status
		runflag = 0;				// clear the run mode flag
		outprmpt();					// output prompt
		getline();					// getline from console
		skipspcs();					// ignore leading spaces in input buffer
		initstack8(&opstack, oparray, OPSLEN);	// always clear opcode stack
		initstack32(&numstack, numarray, NUMSLEN); 	// always clear numeric stack

		if (chckcmds() == 0)		// if line was not a known command...
		{
			fence = varend;			// save current end of variables
			parse();				// translate/execute line
			if (errcode)			// if an error occured somewhere...
			{
			 	rpterr();			// report the error
			 	varend = fence;		// restore the end of variables
			}
		}
	}
 
The function getline() reads a string from the console and saves it
in a global buffer, then sets up global pointers used by later routines
to process that buffer.

The function chckcmds() tries to execute the contents of the string
as if it were a command, such as RUN or LOAD or LIST.  If the string is
a legal command, chckcmds() executes the command and returns a non-zero
value.

If the string does not contain a command, the parse() function selects
the appropriate action based on the content of the string buffer.  A string
containing only a line number will cause the source line of the current
program with that line number to be deleted.  A string containing a
statement for immediate execution, such as 'myvar = 4', will be
converted to tokens (using xlate()), then executed (using runline()).
A string containing a line number followed by a statement will be converted
to tokens (using xlate()), then saved in the current source file (using
storlin()).

Translation of a string to tokens is done by the xlate() function.  This
function steps across the string held in global buffer inbuff[], using
the global pointer ibufptr to access chars in the string.  xlate() converts
keywords it finds within the string to tokens, saving those tokens in the
global buffer tknbuf[], using the global pointer tbufptr to access cells
within the buffer.

After a string has been tokenized in tknbuf[], the function runline() is
called to execute the tokens.  runline() uses tbufptr to step through the
tknbuf[] array.  For each token found, runline() invokes the associated
run-time function, which in turn will advance the tbufptr pointer as
needed.

This design separates the translation phase from the run-time phase.



--- Design of the run-time engine

When KLBasic processes the RUN command or when KLBasic loads and runs a
file from an autostart reboot, the run-time engine is used to process a
KLBasic program.  At the start of execution, the entire program exists as
a set of tokens stored in a RAM buffer named prgram[].  The run-time
engine accesses tokens within this buffer using the global pointer
tbufptr.

The function _crun() in command1.c provides the run-time engine functionality.
This routine uses the token at the current position in the pgrram[] buffer
to select a run-time function from an array of structures named RTFuncs[].
Each function that is called by _crun() must perform the expected actions
and must ensure that the global variables, notably tbufptr, are updated
appropriately.

It is important to maintain the distinction between translate-time and
run-time functions.  If you add new capabilities to KLBasic, be sure to
follow the above time line.  You must provide functions that perform the
translation of characters to tokens and functions that perform the
run-time behavior.  Do not merge these functions!



--- Function naming conventions

Functions generally fall into one of three categories; command, translation,
or run-time.  Functions that execute a command, such as LIST, start with
the letter 'c' followed by the command name.  For example, the function
for executing the command LIST is clist().

Functions that translate a keyword string into a token start with the
letter 'x' followed by the keyword.  For example, the function for
translating a PRINT statement into a set of tokens is xprint().

Functions that provide the run-time behavior for a keyword start with the
letter 'r' followed by the keyword.  For example, the function for executing
the WHILE keyword token is rwhile().



--- Available memory

KLBasic is designed to store a tokenized program in RAM and to execute that
program from RAM.  This can be a problem with some of the smaller MCUs, as
they generally don't have enough RAM to support large programs.  For AVRs,
about the smallest useful MCU is the ATmega1284p, with 16 KB of RAM.  Although
there are versions of KLB for other AVRs, such as the ATmega128, the reduced
amount of RAM limits program size to less than 2 KB.

The ATxmega128a1 version of KLB is designed for use with the Xplained-128a1
board, which includes several megs of RAM, making it suitable for very large
programs.



--- Numbers

KLB uses 32-bit integers, though there are parts of Gordon's code that show
he considered handling floating-point numbers as well.

The run-time engine uses the numeric stack numstack for holding values
and performing math calculations.  numstack is a structure that acts
as a stack object and is accessed using routines found in stack.c.

In general, all run-time math operations use numstack.  For example,
addition pulls the top value off of numstack, adds the new top value on
numstack to it, then overwrites the top value on numstack with
the sum.  For details on the math operations and how they use numstack,
refer to the various run-time routines in files named runtime*.c.



--- Timers

KLB provides a set of down-counting timers, which provide millisecond-
resolution timing.  The target-specific source must provide support for
these timers, generally as a periodic timer interrupt.  Note that the
accuracy of these timers depends on hardware characteristics and they
are not presumed to be highly accurate.  For example, an MCU's timer
chain may not allow precise one-msec interrupts for a given crystal.
However, these timers should provide intervals that are reasonably
close to one millisecond.



--- The console

The target hardware must provide some kind of interactive console device.
This is usually a UART hooked to a serial port on a host PC.  However,
target implementations are free to use other consoles.  For example, my
ATxmega128a1 version of KLBasic includes support for the Gameduino VGA
adapter as a console output device.  You could easily create a target
with support for the Gameduino and a P/S2 keyboard, yielding a standalone
system that no longer needs a host PC for anything.



--- Non-volatile memory

KLB uses some form of non-volatile memory for program storage and retrieval.
For the ATmega variants, this is on-chip flash.  For the ATxmega128a1, this
memory is in a serial flash device wired to the target board, an Atmel
XMEGA128A1-Xplained board.  An obvious upgrade to KLBasic is support for
an SD card with some kind of file system, such as Chan's FatFS.

If you make such additions, remember that these upgrades must be done solely
to the target-specific code, NOT to the core files!



--- Adding new functionality

Gordon's original design used an 8-bit token for each keyword.  This allows
only 256 total tokens and could easily prove to be a bottleneck if you try
to add extensive functionality.  To avoid a complete redesign of the token
structure, you can use a subtoken concept, similar to how KLBasic functions
are handled.

KLB functions use a two-token scheme.  All functions use the same first
token (FUNCTFLG) followed by a specific function subtoken (for example,
ABSTOK).

Thus, if you intend to do a lot of expansion, you might select an available
token as your specific expansion token, then implement a full range of
256 subtokens.



--- Interrupts

Gordon's original design supported the ON <event> construct, which allowed
the use of external interrupts.  I never checked to see if my implementation
broke the design that Gordon had used.  In theory, you should be able to
add interrupt support by setting a flag within an ISR, then checking the
state of that flag in the _crun() routine.  If the flag is set, save the
current context (basically, tbufptr and maybe one or two other variables),
load the target line number for the interrupt, then resume processing.
Eventually, control will hit a RETURN from interrupt, in which case you restore
the context and continue.

I don't know if my design is clean enough to support this.  If it is, that
would be a terrific feature to add to your Basic.



--- CALL() function

The Basic CALL() function is the traditional means of allowing a Basic
program to invoke an assembly language (or C) program.  This can be done
easily in the 68hc11, as it is a Von Neumann machine and you can simply
jump into the middle of RAM to execute code.  This does not work in Harvard
machines and explains why KLBasic does not support CALL() in the AVR
implementations.

If you decide to implement a CALL() function, you will need to extend
the core routines to include the ccall(), xcall(), and rcall() functions.
It would probably be best to make these conditional at compile-time
(using #ifdef VON_NEUMANN or suchlike) so they don't get built for
Harvard machines.



--- AVR program memory

The AVR implementations store a number of string constants in program memory,
so as not to take up valuable RAM.  Dealing with strings stored in AVR
program memory uses the ***_P() function variants, such as strlen_P().
Other MCUs, notably Von Neumann machines such as the 68000, do not need or
use such variants.  You can either use a #define to map the ***_P() version
of these routines to their ANSI counterparts, or you can create stub
routines named ***_P() that simply call the ANSI counterparts.

Similarly, you will find data constants that are assigned to PROGMEM in the
AVR implementations.  This construct causes the compiler/linker to store
the associated data in on-chip flash.  Porting KLB to other devices will
likely not require the PROGMEM construct.



--- Non-volatile system values

The AVR implementations can check target non-volatile memory for
two values following reset.  One value is the frequency of the target
MCU in Hertz, which sets the clock frequency and is used for configuring
system elements such as the down-counting timers and the console
baud rate.  The other value is the console baud rate.

In the AVR implementations, these values are read from on-chip EEPROM.
For other implementations, you will need to find some small section of
NV memory for saving these values.

This arrangement allows KLBasic users to customize their target device
without having to recompile and reinstall KLB.  Done properly, you can
even use a KLBasic program or statements from the console to change
these values.  When you later reboot, the target will use the new values.

Note that in its current form, you only need 12 bytes of NV storage for
the system values.

Note also that this is a target-dependent feature; it is not in the
core and is not a KLBasic core feature.  However, it is very handy, as
you can distribute a version of KLBasic with default settings and any
reasonably capable user can generate the tiny EEPROM hex file needed
to customize KLB for their use.



--- Ideas for the future

There is so much more to do on KLBasic.  Here, in no particular order, is
a list of what I would like to add if I ever hit the lottery...

* Write a user's manual!  This is one of those tasks that I kept putting
off and it never got done...

* Add a command for listing the keywords that can be accepted by KLB,
similar to Forth's WORDS word.  I think a variant of LIST, such as
LIST KEYWORDS or something, would be a big help.  It doesn't need to
provide usage (though it could) but at least list the acceptable
keywords and commands.

* Function for resetting the target from the console or from within a 
program.  This would be easy to do, probably little more than turning
on the watchdog, then letting it time out.

* Support for strings, including a way to declare (dimension) them and
functions for the common string operations.  I would probably do this
by requiring the user to declare the maximum size of a string using a
DIM statement.  The goal would be to design a system that did not rely
on garbage collection, to minimize performance hits.

* Support for floating point.

* Support for SD card.

* Support for FAT file system (such as Chan's FatFS).  This would
include modifying the LOAD and SAVE commands to accept file names.

* Support for the PRINT# and INPUT# constructs.  The basis for this
is already in Gordon's code, but I did not carry it over into my version.

* Target-independent full-screen editor.  This is a big one, as it
would require redesigning large parts of KLBasic's core and standardizing
on a common target.  But it would remove the need for line numbers
in programs.

* Support for SUBROUTINE and FUNCTION statements.  I've given this
some thought but not yet come up with an idea I like.  At the very least,
add the ability to assign a line number to a variable, which would
allow constructs such as:

   50 readadc = 100
   60 gosub readadc

Even something this simple would ease maintanence of programs and
help reduce bugs.

* Add a RENUMBER function of some kind, to ease editing chores when you
have to move around blocks of code within your program.

* Perform general code cleanup.  The current code is pretty messy and
certainly isn't typical of my designs.



--- Conclusion

I'm sure I've left out a lot of important information.  If you hit a wall
and need to contact me, do a Google for Karl Lunt; you'll find me.  Send me
an email and I will try to help you past the hurdle.

Good luck,,,



