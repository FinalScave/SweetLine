; NASM - Intel syntax assembly example
; This demonstrates Intel/NASM style assembly

section .data
    message db "Hello, World!", 0xA, 0
    len equ $ - message
    hex_num dd 0xDEADBEEF
    bin_num dd 0b10101010

section .bss
    buffer resb 64

section .text
    global _start

_start:
    ; System call to write
    mov rax, 1          ; sys_write
    mov rdi, 1          ; stdout
    mov rsi, message    ; message address
    mov rdx, len        ; message length
    syscall

    ; Exit syscall
    mov rax, 60         ; sys_exit
    xor rdi, rdi        ; exit code 0
    syscall

; Function example
calculate_sum:
    push rbp
    mov rbp, rsp
    
    ; Add two numbers
    mov eax, [rbp+16]   ; first argument
    add eax, [rbp+24]   ; second argument
    
    pop rbp
    ret

; SIMD example using AVX
simd_operations:
    vmovaps ymm0, [rdi]
    vmovaps ymm1, [rsi]
    vaddps ymm2, ymm0, ymm1
    vmovaps [rdx], ymm2
    ret

; Macro example
%macro PRINT 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

; Conditional assembly
%ifdef DEBUG
    %warning "Debug mode enabled"
%endif

; Include file
%include "macros.inc"

; CPU directives
cpu SSE4.2
cpu AVX
