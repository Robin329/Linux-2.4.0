#ifndef _M68K_DIV64_H
#define _M68K_DIV64_H

/* n = n / base; return rem; */

#if 1
#define do_div(n, base) ({					\
	union {							\
		unsigned long n32[2];				\
		unsigned long long n64;				\
	} __n;							\
	unsigned long __rem, __upper;				\
								\
	__n.n64 = (n);						\
	if ((__upper = __n.n32[0])) {				\
		asm ("divul.l %2,%1:%0"				\
			: "=d" (__n.n32[0]), "=d" (__upper)	\
			: "d" (base), "0" (__n.n32[0]));	\
	}							\
	asm ("divu.l %2,%1:%0"					\
		: "=d" (__n.n32[1]), "=d" (__rem)		\
		: "d" (base), "1" (__upper), "0" (__n.n32[1]));	\
	(n) = __n.n64;						\
	__rem;							\
})
#else
#define do_div(n,base) ({					\
	int __res;						\
	__res = ((unsigned long) n) % (unsigned) base;		\
	n = ((unsigned long) n) / (unsigned) base;		\
	__res;							\
})
#endif

#endif /* _M68K_DIV64_H */
