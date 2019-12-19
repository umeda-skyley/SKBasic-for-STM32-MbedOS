/*
 *  stacks.c      stack operators for Basic11 project
 */


#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"
#include  "stacks.h"


void  push32(STACK32  *stk, U32  val, unsigned char  err)
{
	if (stk->index == stk->max)			// if stack is already full...
	{
		errcode = err;					// tell the world
		rpterr();						// tell the user
		return;
	}
	stk->stk[stk->index] = val;		// ok so far, save the value
	stk->index++;						// bump the index
}



U32  pull32(STACK32  *stk, unsigned char  err)
{
	if (stk->index == 0)				// if the stack is empty...
	{	
		errcode = err;					// tell the world
		rpterr();						// tell the user
		return  (0);					// have to return something
	}
	stk->index--;						// back up one
	return  (stk->stk[stk->index]);		// and return the value
}



void  initstack32(STACK32  *stk, U32  *array, U8  max)
{
	stk->index = 0;						// clear the index
	stk->max = max;						// set the upper limit
	stk->stk = array;					// set the pointer to the stack
}



void  push8(STACK8  *stk, U8  val, unsigned char  err)
{
	if (stk->index == stk->max)			// if stack is already full...
	{
		errcode = err;					// tell the world
		rpterr();						// tell the user
		return;
	}
	stk->stk[stk->index] = val;		// ok so far, save the value
	stk->index++;						// bump the index
}



U8  pull8(STACK8  *stk, unsigned char  err)
{
	if (stk->index == 0)				// if the stack is empty...
	{	
		errcode = err;					// tell the world
		rpterr();						// tell the user
		return  (0);					// have to return something
	}
	stk->index--;						// back up one
	return  (stk->stk[stk->index]);		// and return the value
}



void  initstack8(STACK8  *stk, U8  *array, U8  max)
{
	stk->index = 0;						// clear the index
	stk->max = max;						// set the upper limit
	stk->stk = array;					// set the pointer to the stack
}
