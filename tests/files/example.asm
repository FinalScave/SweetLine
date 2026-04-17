; Assembly sample (x86-64 NASM syntax)

; This is a comment
# This is also a comment
! Another comment style
| Yet another comment

/*
 * Multi-line comment
 * can span multiple lines
 */

; Data section
section .data
    message db "Hello, World!", 0xA
    len equ $ - message
    number dd 42
    float_num dd 3.14
    hex_value dd 0xDEADBEEF
    bin_value db 0b10101010

; BSS section
section .bss
    buffer resb 256
    array resd 10

; Text section
section .text
    global _start

; Entry point
_start:
    ; System call: write
    mov rax, 1          ; syscall number for write
    mov rdi, 1          ; file descriptor (stdout)
    mov rsi, message    ; message to write
    mov rdx, len        ; message length
    syscall             ; make system call

    ; System call: exit
    mov rax, 60         ; syscall number for exit
    xor rdi, rdi        ; exit code 0
    syscall             ; make system call

; Function example
my_function:
    push rbp            ; save base pointer
    mov rbp, rsp        ; set new base pointer
    sub rsp, 16         ; allocate stack space
    
    ; Function body
    mov qword [rbp-8], 42
    
    ; Return value
    mov rax, [rbp-8]
    
    ; Epilogue
    add rsp, 16         ; deallocate stack space
    pop rbp             ; restore base pointer
    ret                 ; return

; Loop example
loop_example:
    mov rcx, 10         ; loop counter
.loop_label:
    ; Loop body
    dec rcx             ; decrement counter
    jnz .loop_label     ; jump if not zero
    ret

; Conditional example
conditional_example:
    cmp rax, 0
    je .is_zero
    jg .is_positive
    jl .is_negative
    
.is_zero:
    mov rax, 0
    ret
    
.is_positive:
    mov rax, 1
    ret
    
.is_negative:
    mov rax, -1
    ret

; Macro definition
%macro my_macro 2
    mov rax, %1
    mov rbx, %2
    add rax, rbx
%endmacro

; Conditional assembly
%ifdef DEBUG
    mov rax, 1
%else
    mov rax, 0
%endif

; Include file
%include "macros.inc"

; Repeat block
%rep 5
    nop
%endrep

; Data types
byte_var:   db 0xFF
word_var:   dw 0xFFFF
dword_var:  dd 0xFFFFFFFF
qword_var:  dq 0xFFFFFFFFFFFFFFFF

; ASCII string
ascii_str:  ascii "Hello"
asciz_str:  asciz "World"

; Floating point
float_var:  float 3.14159
double_var: double 2.71828

; Register operations
mov rax, rbx
mov eax, ebx
mov ax, bx
mov al, bl

; Memory operations
mov rax, [rbp-8]
mov qword [rbp-8], 42
lea rax, [rbp-8]

; Arithmetic operations
add rax, rbx
sub rax, rbx
imul rax, rbx
idiv rbx

; Logical operations
and rax, rbx
or rax, rbx
xor rax, rbx
not rax

; Shift operations
shl rax, 4
shr rax, 4

; Stack operations
push rax
pop rbx

; Jump instructions
jmp label
call function
ret

; Comparison
cmp rax, rbx
test rax, rax

; SIMD instructions (SSE/AVX)
movaps xmm0, xmm1
addps xmm0, xmm1
mulps xmm0, xmm1

; Labels
label1:
.label2:
label3:

; Directives
.global main
.extern printf
.align 16
.size main, .-main
.type main, @function
