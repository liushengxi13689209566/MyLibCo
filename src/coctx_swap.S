/*
* Tencent is pleased to support the open source community by making Libco available.
* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

.globl coctx_swap
#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
.type  coctx_swap, @function
#endif
coctx_swap:

/*struct coctx_t
{
#if defined(__i386__)
	void *regs[ 8 ];
#else
	void *regs[ 14 ];
#endif
	size_t ss_size;
	char *ss_sp;
	
}*/

#if defined(__i386__)
	leal 4(%esp), %eax //sp 保存栈顶指针 
	movl 4(%esp), %esp  //指向 current 协程 coctx_t 的指针 
	leal 32(%esp), %esp //parm a : &regs[7] + sizeof(void*) 指向ss_size

	pushl %eax //esp ->parm a  栈顶保存到了 coctx_t->regs[7] 的位置
    //保存各通用寄存器值
	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl -4(%eax)
    //将 coctx_swap 的返回地址给保存起来了,放在coctx_t->regs[0] 的位置

	
	movl 4(%eax), %esp //parm b -> &regs[0]


	popl %eax  //ret func addr 弹出 regs[0] 的值到 eax 寄存器
    //从 regs[1]~regs[6] 恢复各寄存器的值
	popl %ebx  
	popl %ecx
	popl %edx
	popl %edi
	popl %esi
	popl %ebp
	popl %esp //将 pending 协程上次切换出去时的栈指针恢复
                
	pushl %eax //set ret func addr 设置函数返回地址

	xorl %eax, %eax //清零%eax寄存器
	ret

#elif defined(__x86_64__)
	leaq 8(%rsp),%rax
	leaq 112(%rdi),%rsp
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx

	pushq -8(%rax) //ret func addr

	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	
	movq %rsi, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r9
	popq %r8
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rax //ret func addr
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rsp
	pushq %rax
	
	xorl %eax, %eax
	ret
#endif