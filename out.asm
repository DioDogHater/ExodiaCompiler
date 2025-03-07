global _start
_start:
	mov rax, 0
	push rax
	mov rax, 5
	push rax
	mov rax, 5
	push rax
	pop rax
	pop rbx
	imul rbx
	push rax
	mov rax, 1
	push rax
	pop rax
	pop rbx
	add rax, rbx
	push rax
	mov rax, 22
	push rax
	pop rbx
	pop rax
	sub rax, rbx
	push rax
	pop rax
	mov [rsp+0], rax
	mov rax, 2
	push rax
	mov rax, 2
	push rax
	pop rax
	pop rbx
	add rax, rbx
	push rax
	mov rax, 4
	push rax
	pop rax
	pop rbx
	imul rbx
	push rax
	push QWORD [rsp+0]
	push QWORD [rsp+16]
	pop rbx
	pop rax
	sub rax, rbx
	push rax
	mov rax, 60
	pop rdi
	syscall
	mov rax, 60
	mov rdi, 0
	syscall
