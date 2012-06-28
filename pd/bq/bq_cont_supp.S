// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

.file "bq_cont_supp.S"

#if defined(__i386__)

#define FUNCTION(fun) \
	.text; \
	.align 2; \
	.p2align 4,,15; \
.globl fun; \
	.type fun, @function; \
fun: \
	.cfi_startproc

#define END(fun) \
	.cfi_endproc; \
	.size fun, .-fun

/*
unsigned long pd::state_t::save();
 */

FUNCTION(_ZN2pd7state_t4saveEv)
	movl 4(%esp), %eax  /* __eax = this; */
	leal 4(%esp), %ecx
	movl %ecx, (%eax)   /* stack_ptr = __esp + 1; */
	movl %ebp, 4(%eax)  /* frame_ptr = __ebp; */
	movl (%esp), %ecx
	movl %ecx, 8(%eax)  /* pc = *__rsp; */
	movl %ebx, 12(%eax) /* misc[0] = __ebx; */
	movl %esi, 16(%eax) /* misc[1] = __esi; */
	movl %edi, 20(%eax) /* misc[2] = __edi; */
	xorl %eax, %eax     /* return (0); */
	ret
END(_ZN2pd7state_t4saveEv)

/*
void __noreturn pd::state_t::restore(int unsigned long x);
 */

FUNCTION(_ZN2pd7state_t7restoreEm)
	movl 4(%esp), %edx  /* __edx = this; */
	movl 8(%esp), %eax  /* __eax = x; */
	movl 12(%edx), %ebx /* __ebx = misc[0]; */
	movl 16(%edx), %esi /* __esi = misc[1]; */
	movl 20(%edx), %edi /* __edi = misc[2]; */

	movl 4(%edx), %ebp  /* __ebp = frame_ptr; */
	movl (%edx), %esp   /* __esp = stack_ptr; */
	movl 8(%edx), %edx  /* __rdx = pc; */
	jmpl *%edx
END(_ZN2pd7state_t7restoreEm)

/*
extern void __hidden __noreturn pd::bq_cont_proc(void (*fun)(void *), void *arg);
 */

.hidden _ZN2pd12bq_cont_procEPFvPvES0_

/*
void __hidden __noreturn pd::bq_cont_run(void (*fun)(void *), void *arg, void *stack);
 */

FUNCTION(_ZN2pd11bq_cont_runEPFvPvES0_S0_)
	.cfi_endproc
	movl %esp, %ebp
	movl 12(%ebp), %eax
	movl %eax, %esp
	movl 8(%ebp), %eax
	pushl %eax
	movl 4(%ebp), %eax
	pushl %eax
	xorl %ebp, %ebp
	pushl %ebp
	jmp _ZN2pd12bq_cont_procEPFvPvES0_ /* pd::bq_cont_proc(fun, arg); */
	.cfi_startproc
END(_ZN2pd11bq_cont_runEPFvPvES0_S0_)

#elif defined(__x86_64__)

#define FUNCTION(fun) \
	.text; \
	.align 2; \
	.p2align 4,,15; \
.globl fun; \
	.type fun, @function; \
fun: \
	.cfi_startproc

#define END(fun) \
	.cfi_endproc; \
	.size fun, .-fun

/*
unsigned long pd::state_t::save();
 */

FUNCTION(_ZN2pd7state_t4saveEv)
	leaq 8(%rsp), %rax
	movq %rax, (%rdi)   /* stack_ptr = __rsp + 1; */
	movq %rbp, 8(%rdi)  /* frame_ptr = __rbp; */
	movq (%rsp), %rax
	movq %rax, 16(%rdi) /* pc = *__rsp; */
	movq %rbx, 24(%rdi) /* misc[0] = __rbx; */
	movq %r12, 32(%rdi) /* misc[1] = __r12; */
	movq %r13, 40(%rdi) /* misc[2] = __r13; */
	movq %r14, 48(%rdi) /* misc[3] = __r14; */
	movq %r15, 56(%rdi) /* misc[4] = __r15; */
	xorq %rax, %rax     /* return (0); */
	ret
END(_ZN2pd7state_t4saveEv)

/*
void __noreturn pd::state_t::restore(int unsigned long x);
 */

FUNCTION(_ZN2pd7state_t7restoreEm)
	movq 24(%rdi), %rbx /* __rbx = misc[0]; */
	movq 32(%rdi), %r12 /* __r12 = misc[1]; */
	movq 40(%rdi), %r13 /* __r13 = misc[2]; */
	movq 48(%rdi), %r14 /* __r14 = misc[3]; */
	movq 56(%rdi), %r15 /* __r15 = misc[4]; */

	movq 8(%rdi), %rbp  /* __rbp = frame_ptr; */
	movq (%rdi), %rsp   /* __rsp = stack_ptr; */
	movq 16(%rdi), %rdx /* __rdx = pc; */
	movq %rsi, %rax     /* __rax = x; */
	jmpq *%rdx
END(_ZN2pd7state_t7restoreEm)

/*
extern void __hidden __noreturn pd::bq_cont_proc(void (*fun)(void *), void *arg);
 */

.hidden _ZN2pd12bq_cont_procEPFvPvES0_

/*
void __hidden __noreturn pd::bq_cont_run(void (*fun)(void *), void *arg, void *stack);
 */

FUNCTION(_ZN2pd11bq_cont_runEPFvPvES0_S0_)
	.cfi_endproc
	movq %rdx, %rsp /* __rsp = stack; */
	xorq %rbp, %rbp /* __rbp = 0; */
	pushq %rbp
	jmp _ZN2pd12bq_cont_procEPFvPvES0_ /* pd::bq_cont_proc(fun, arg); */
	.cfi_startproc
END(_ZN2pd11bq_cont_runEPFvPvES0_S0_)

#else
#error Not implemented yet
#endif
