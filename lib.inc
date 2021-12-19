section .data

section .text


; Принимает код возврата и завершает текущий процесс
exit:
    xor rax, rax,
	mov rax,  60
	xor rdi, rdi
	syscall
    ret

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:

    xor rax, rax
	
	.loop:
	cmp byte [rax + rdi],0
	
	je .end
	
	;inc pointer
	inc rax
	
	jmp .loop
	
	.end:
	;mov pointer,0
    ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:

    xor rax, rax
	
    push rdi
    call string_length
    pop rsi
	
    mov rdx, rax
    mov rdi, 1
    mov rax, 1
    syscall
    
  .end:
    ret

; Принимает код символа и выводит его в stdout
print_char:
    xor rax, rax
	
	push rdi
	
	mov rax,1
	mov rdi,1
	
	mov rsi,rsp
	
	mov rdx,1
	syscall
	pop rdi
	
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    xor rax, rax
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате					
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
	xor rax,rax
	
    mov rax,rdi
	mov r10,0
	mov r9,10
	mov rdx,0
	
	.loop:
	
		div r9
		push rdx
		inc r10
		mov rdx,0
		
		cmp rax,0
		je .nul
	
	jmp .loop
	
	.nul:
	
		cmp r10,0
		je .end
		
		pop rdi
		add rdi,48
		call print_char
		
		dec r10
		
		jmp .nul
	
	.end:
		ret

; Выводит знаковое 8-байтовое число в десятичном формате
print_int:
    xor rax, rax
	
	cmp rdi,0
	jge print_uint
	
	neg rdi
	mov r9,rdi
	mov rdi,45
	call print_char
	mov rdi,r9
	
	jmp print_uint

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
	; rdi - first poiner
	; rsi - second poiner
	
	.loop:
	
		mov r9b,byte[rsi]
		mov r10b,byte[rdi]
		cmp r9b,r10b
		jne .not_equals
		
		cmp r9b,0
		je .equals
		
		inc rsi
		inc rdi
		
		jmp .loop
		
	.equals:
		mov rax,1
		ret
	.not_equals:
		mov rax,0
		ret
		

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    xor rax, rax
	
	push 0
	mov rsi,rsp
	
	mov rax,0
	mov rdi,0
	mov rdx,1
	syscall
	
	pop rax
	
    ret

; Принимает: адрес начала буфера (rdi), размер буфера (rsi)
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:

	push r12

	mov r9,rdi	; pointer
	mov r10,rsi ; size
	xor r11,r11	; length
	mov r12,rdi
	
	.tabs:
	
		push r11
		call read_char
		pop r11
		
		cmp rax,0x0
		je .end
		cmp rax,' '
		je .tabs
		cmp rax,0x9
		je .tabs
		cmp rax,'\n'
		je .tabs
		
		mov [r9],rax
		inc r9
		
		inc r11
		dec r10
	
	.reader:
	
		cmp r10,0
		je .nul
		
		push r11
		call read_char
		pop r11
		
		cmp rax,0x0
		je .end
		cmp rax,0x20
		je .end
		cmp rax,0x9
		je .end
		cmp rax,0xA
		je .end
	
		mov [r9],rax
		inc r9
		
		inc r11
		dec r10
		
		jmp .reader
		
	
	.nul:
		mov rdx,0
		pop r12
		mov rax,0
		ret

	.end:
	
		mov rdx,r11
		
		mov rax,r12
		mov byte [r9],0
		
		pop r12
		
		ret


; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось


parse_uint:
    xor rax, rax
    xor rdx, rdx

	mov r9,10
	
	.loop:
	
		push rdx
	
		mov r10,[rdi]
		and r10,0x00000000000000ff

		cmp r10,0x30
		jl .not_a_number
		
		cmp r10,0x39
		jg .not_a_number
		
		cmp r10,0
		je .not_a_number
		
		mul r9
		add rax,r10
		
		sub rax,48
		
		inc rdi
		
		pop rdx
		inc rdx
		
		jmp .loop
		

	.not_a_number:
		pop rdx
		ret


; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был)
; rdx = 0 если число прочитать не удалось
parse_int:
    xor rax, rax
	
	;[rdi] - первый знак. Либо минус, либо число
	mov r11, [rdi]
	
	and r11,0x00000000000000ff
	cmp r11,45		; код минуса
	je .neg
	
	jmp parse_uint
	
	.neg:
	
		inc rdi
		
		call parse_uint
		
		neg rax
		inc rdx
	
		ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    xor rax, rax
	
	push rsi
    push rdi
    push rdx
		call string_length
	pop rdx
    pop rdi
    pop rsi
	
	cmp rax,rdx
	jg .zero_end
	
	mov byte [rsi],0
	
	mov rdx,0
	.loop:
	
		cmp byte [rdi],0
		je .not_zero_end
		
		mov r9, [rdi]
		and r9, 0x00000000000000ff
		mov [rsi], r9
		
		inc rsi
		inc rdi
		inc rdx
	
	jmp .loop
	
	.not_zero_end:
		mov rax,rdx
		ret
	
	.zero_end:
		mov rax,0
		ret
