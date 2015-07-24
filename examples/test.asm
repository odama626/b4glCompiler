	global main
	extern printf
	extern exit
	
	section .data
	global_byte_buffer: DB 0
	global_char_format: DB "%c",0
	a:	DW 0
	section .text
main:
	mov ax,[a]
	push rax
	mov rax,0
	pop rbx
	cmp rax, rbx
	jne L1
	mov rax, 1
	jmp L0
L1:
	mov rax, 0
L0:
	cmp rax, 0
	JE L2
	mov rax,49
	mov [global_byte_buffer], al
	mov rcx, global_char_format
	mov rdx, qword[global_byte_buffer]
	call printf
L2:
	xor rcx, rcx
	call exit
