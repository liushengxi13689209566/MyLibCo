/*************************************************************************
	> File Name: test1.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月17日 星期一 17时52分57秒
 ************************************************************************/

#include <iostream>
#include <ucontext.h>

using namespace std;
static char g_stack[2048];

#if (0)

/*Userlevel context.*/

typedef struct ucontext_t
{
	unsigned long int uc_flags;
	struct ucontext_t *uc_link; //uc_link指向当前上下文退出时将恢复的上下文
	stack_t uc_stack;
	mcontext_t uc_mcontext; //uc_mcontext存储执行 状态，包括所有寄存器和CPU 标志，指令指针和堆栈指针
	sigset_t uc_sigmask;
	struct _libc_fpstate __fpregs_mem;
} ucontext_t;

/*Structure describing a signal stack.*/
typedef struct
{
	void *ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;

#endif

static ucontext_t ctx,
	ctx_main;

void func()
{
	cout << "enter func......" << endl;

	swapcontext(&ctx, &ctx_main); //切换到 ctx_main 上下文，保存当前上下文到 ctx　

	cout << "func resume from yield " << endl;
}

int main(void)
{
	getcontext(&ctx);
	ctx.uc_stack.ss_sp = g_stack;
	ctx.uc_stack.ss_size = sizeof(g_stack);
	ctx.uc_link = &ctx_main;

	makecontext(&ctx, func, 0); //修改上下文指向 func 函数
	cout << "main " << endl;

	swapcontext(&ctx_main, &ctx); //切换到 ctx 上下文，保存当前上下文到 ctx_main　
	cout << "back to main " << endl;

	swapcontext(&ctx_main, &ctx);
	cout << "back to main  again " << endl;
	return 0;
}
