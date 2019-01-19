/*************************************************************************
	> File Name: liu_coctx.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月16日 星期三 16时56分49秒
 ************************************************************************/

#ifndef _COCTX_H
#define _COCTX_H

#include <stdlib.h>
typedef void *(*coctx_pfn_t)(void *s, void *s2);
struct coctx_param_t
{
	const void *s1;
	const void *s2;
};
struct Coctx_t
{
	void *regs[14]; //14个寄存器

	size_t ss_size;
	char *ss_sp;
};

int Coctx_init(Coctx_t *ctx);
int Coctx_make(Coctx_t *ctx, coctx_pfn_t pfn, const void *s, const void *s1);
#endif
