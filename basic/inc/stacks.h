/*
 *  stacks.h      include file for stack support
 */

#ifndef  STACKS_H
#define  STACKS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef  struct  stack32_t
{
	unsigned char	max;			// maximum number of items on stack
	unsigned char	index;			// current stack pointer
	U32				*stk;			// pointer to stack array
}  STACK32;


typedef  struct  stack8_t
{
	unsigned char	max;			// maximum number of items on stack
	unsigned char	index;			// current stack pointer
	U8				*stk;			// pointer to stack array
}  STACK8;




/*
 *  Functions for handling stacks
 */

void				initstack32(STACK32  *stk, U32  *array, U8  max);
void				initstack8(STACK8  *stk, U8  *array, U8  max);
void				push32(STACK32  *stk, U32  val, unsigned char  err);
void				push8(STACK8  *stk, U8  val, unsigned char  err);
U32					pull32(STACK32  *stk, unsigned char  err);
U8					pull8(STACK8  *stk, unsigned char  err);

#ifdef __cplusplus
}
#endif

#endif
