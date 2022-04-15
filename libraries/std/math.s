.global	math.abs
.global	math.longDiv
.global	math.longExp
.global	math.exp
.global	pub_Random_init
.global	pub_Random_nextInt


.text

pub_Random_nextInt:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	pushq	%r14
	movq	-8(%rbp), %r14
	pushq	%rdi
	pushq	%rdx
	movl	$910230123, %edx
	pushq	%r14
	movq	-8(%rbp), %r14
	movl	0(%r14), %r15d
	popq	%r14
	movl	%r15d, %edi
	add	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, 0(%r14)
	popq	%r14
	pushq	%r14
	movq	-8(%rbp), %r14
	movl	0(%r14), %r15d
	popq	%r14
	movl	%r15d, %ebx
	movl	%ebx, -16(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	$0, %edx
	movl	-16(%rbp), %r15d
	movl	%r15d, %edi
	cmpl	%edx, %edi
	setl	%al
	popq	%rdx
	popq	%rdi
	movb	%al, %al
	cmpb	$0, %al
	je	.Lexp0
	pushq	%rdi
	pushq	%rdx
	movl	$-1, %edx
	movl	-16(%rbp), %r15d
	movl	%r15d, %edi
	imul	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -16(%rbp)
.Lexp0:
	pushq	%rdi
	pushq	%rdx
	movl	-16(%rbp), %r15d
	movl	%r15d, %eax
	movl	-12(%rbp), %r15d
	movl	%r15d, %ecx
	cltd
	idiv	%ecx
	movl	%edx, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %eax
	leave
	ret
pub_Random_init:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	pushq	%rdx
	movq	$0, %rdi
	call	DateTime.Now
	popq	%rdx
	movq	%rax, %rbx
	movq	%rbx, -16(%rbp)
	pushq	%r14
	movq	-8(%rbp), %r14
	pushq	%rdx
	lea	-16(%rbp), %rax
	pushq	%rdi
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	pub_DateTime_getSeconds
	popq	%rdi
	popq	%rdx
	movl	%eax, %ebx
	movl	%ebx, 0(%r14)
	popq	%r14
	pushq	%rdx
	movq	-16(%rbp), %r15
	pushq	%rdi
	movq	%r15, %rax
	movq	%rax, %rdi
	call	free
	popq	%rdi
	popq	%rdx
	movl	$0, %ebx
	movl	%ebx, -20(%rbp)
	jmp	.Lexp2
.Lexp1:
	pushq	%rdx
	lea	-8(%rbp), %rax
	pushq	%rdi
	movq	(%rax), %rax
	movq	%rax, %rdi
	pushq	%rsi
	movl	$100, %eax
	movl	%eax, %esi
	call	pub_Random_nextInt
	popq	%rsi
	popq	%rdi
	popq	%rdx
	add	$1, -20(%rbp)
.Lexp2:
	pushq	%rdi
	pushq	%rdx
	pushq	%rdx
	lea	-8(%rbp), %rax
	pushq	%rdi
	movq	(%rax), %rax
	movq	%rax, %rdi
	pushq	%rsi
	movl	$10, %eax
	movl	%eax, %esi
	call	pub_Random_nextInt
	popq	%rsi
	popq	%rdi
	popq	%rdx
	movl	%eax, %edx
	movl	-20(%rbp), %r15d
	movl	%r15d, %edi
	cmpl	%edx, %edi
	setl	%al
	popq	%rdx
	popq	%rdi
	movb	%al, %al
	cmpb	$1, %al
	je	.Lexp1
	popq	%rbx
	movq	-8(%rbp), %rax
	leave
	ret
math.exp:
exp:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	$0, %ebx
	movl	%ebx, -12(%rbp)
	movl	$1, %ebx
	movl	%ebx, -16(%rbp)
	jmp	.Lexp4
.Lexp3:
	pushq	%rdi
	pushq	%rdx
	movl	-4(%rbp), %r15d
	movl	%r15d, %edx
	movl	-16(%rbp), %r15d
	movl	%r15d, %edi
	imul	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -16(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	$1, %edx
	movl	-12(%rbp), %r15d
	movl	%r15d, %edi
	add	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -12(%rbp)
.Lexp4:
	pushq	%rdi
	pushq	%rdx
	movl	-8(%rbp), %r15d
	movl	%r15d, %edx
	movl	-12(%rbp), %r15d
	movl	%r15d, %edi
	cmpl	%edx, %edi
	setl	%al
	popq	%rdx
	popq	%rdi
	movb	%al, %al
	cmpb	$1, %al
	je	.Lexp3
	movl	-16(%rbp), %r15d
	movl	%r15d, %eax
	leave
	ret
math.longExp:
longExp:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	$0, %ebx
	movl	%ebx, -16(%rbp)
	movq	$1, %rbx
	movq	%rbx, -24(%rbp)
	jmp	.LlongExp6
.LlongExp5:
	pushq	%rdi
	pushq	%rdx
	movq	-8(%rbp), %r15
	movq	%r15, %rdx
	movq	-24(%rbp), %r15
	movq	%r15, %rdi
	imul	%rdx, %rdi
	movq	%rdi, %rax
	popq	%rdx
	popq	%rdi
	movq	%rax, %rbx
	movq	%rbx, -24(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	$1, %edx
	movl	-16(%rbp), %r15d
	movl	%r15d, %edi
	add	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -16(%rbp)
.LlongExp6:
	pushq	%rdi
	pushq	%rdx
	movl	-12(%rbp), %r15d
	movl	%r15d, %edx
	movl	-16(%rbp), %r15d
	movl	%r15d, %edi
	cmpl	%edx, %edi
	setl	%al
	popq	%rdx
	popq	%rdi
	movb	%al, %al
	cmpb	$1, %al
	je	.LlongExp5
	movq	-24(%rbp), %r15
	movq	%r15, %rax
	leave
	ret
math.longDiv:
longDiv:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	-4(%rbp), %r15d
	movl	%r15d, %eax
	movl	-8(%rbp), %r15d
	movl	%r15d, %ecx
	cltd
	idiv	%ecx
	movl	%edx, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -12(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	$10, %edx
	movl	-12(%rbp), %r15d
	movl	%r15d, %edi
	imul	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -12(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	-12(%rbp), %r15d
	movl	%r15d, %eax
	movl	-8(%rbp), %r15d
	movl	%r15d, %ecx
	cltd
	idiv	%ecx
	popq	%rdx
	popq	%rdi
	movl	%eax, %ebx
	movl	%ebx, -12(%rbp)
	movl	-12(%rbp), %r15d
	movl	%r15d, %eax
	leave
	ret
math.abs:
abs:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rbx
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	pushq	%rdi
	pushq	%rdx
	movl	$0, %edx
	movl	-4(%rbp), %r15d
	movl	%r15d, %edi
	cmpl	%edx, %edi
	setl	%al
	popq	%rdx
	popq	%rdi
	movb	%al, %al
	cmpb	$0, %al
	je	.Labs7
	pushq	%rdi
	pushq	%rdx
	movl	$-1, %edx
	movl	-4(%rbp), %r15d
	movl	%r15d, %edi
	imul	%edx, %edi
	movl	%edi, %eax
	popq	%rdx
	popq	%rdi
	movl	%eax, %eax
	leave
	ret
.Labs7:
	movl	-4(%rbp), %r15d
	movl	%r15d, %eax
	leave
	ret


.data



.bss

