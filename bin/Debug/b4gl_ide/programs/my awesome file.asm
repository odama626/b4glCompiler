	global _start
	
	section .data
	global_byte_buffer: DB 0
	poop:	DW 48
	section .text
_start:
	mov rax,[poop]
	mov [global_byte_buffer], al
	mov rax, 1
	mov rdi, 1
	mov rsi, global_byte_buffer
	mov rdx, 1
	syscall
	MOV rax,60  ;send exit command
	xor rdi, rdi
	syscall
