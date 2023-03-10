/* $Id: fmadd.c,v 1.1 1999/08/23 18:59:36 cort Exp $
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "soft-fp.h"
#include "double.h"

int
fmadd(void *frD, void *frA, void *frB, void *frC)
{
	FP_DECL_D(R);
	FP_DECL_D(A);
	FP_DECL_D(B);
	FP_DECL_D(C);
	FP_DECL_D(T);
	int ret = 0;

#ifdef DEBUG
	printk("%s: %p %p %p %p\n", __FUNCTION__, frD, frA, frB, frC);
#endif

	__FP_UNPACK_D(A, frA);
	__FP_UNPACK_D(B, frB);
	__FP_UNPACK_D(C, frC);

#ifdef DEBUG
	printk("A: %ld %lu %lu %ld (%ld)\n", A_s, A_f1, A_f0, A_e, A_c);
	printk("B: %ld %lu %lu %ld (%ld)\n", B_s, B_f1, B_f0, B_e, B_c);
	printk("C: %ld %lu %lu %ld (%ld)\n", C_s, C_f1, C_f0, C_e, C_c);
#endif

	if ((A_c == FP_CLS_INF && C_c == FP_CLS_ZERO) ||
	    (A_c == FP_CLS_ZERO && C_c == FP_CLS_INF))
                ret |= EFLAG_VXIMZ;

	FP_MUL_D(T, A, C);

	if (T_s != B_s && T_c == FP_CLS_INF && B_c == FP_CLS_INF)
		ret |= EFLAG_VXISI;

	FP_ADD_D(R, T, B);

#ifdef DEBUG
	printk("D: %ld %lu %lu %ld (%ld)\n", R_s, R_f1, R_f0, R_e, R_c);
#endif

	return (ret | __FP_PACK_D(frD, R));
}
