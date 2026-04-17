# AT&T/GAS syntax assembly example
# This demonstrates AT&T style assembly for x86-64

.section .data
    message: .ascii "Hello, World!\n"
    len = . - message
    hex_num: .long 0xDEADBEEF
    bin_num: .long 0b10101010

.section .bss
    .lcomm buffer, 64

.section .text
    .globl _start
    .type _start, @function

_start:
    # System call to write
    movq $1, %rax           # sys_write
    movq $1, %rdi           # stdout
    leaq message(%rip), %rsi # message address
    movq $len, %rdx         # message length
    syscall

    # Exit syscall
    movq $60, %rax          # sys_exit
    xorq %rdi, %rdi         # exit code 0
    syscall

    .size _start, .-_start

# Function example
calculate_sum:
    pushq %rbp
    movq %rsp, %rbp
    
    # Add two numbers
    movl 16(%rbp), %eax     # first argument
    addl 24(%rbp), %eax     # second argument
    
    popq %rbp
    ret
    .size calculate_sum, .-calculate_sum

# SIMD example using AVX
simd_operations:
    vmovaps (%rdi), %ymm0
    vmovaps (%rsi), %ymm1
    vaddps %ymm0, %ymm1, %ymm2
    vmovaps %ymm2, (%rdx)
    ret
    .size simd_operations, .-simd_operations

# String operations
string_copy:
    cld
    leaq source(%rip), %rsi
    leaq dest(%rip), %rdi
    movq $64, %rcx
    rep movsb
    ret

# Branch instructions
conditional_jump:
    cmpl $0, %eax
    je .Lzero
    jg .Lpositive
    jl .Lnegative
    ret

.Lzero:
    movq $0, %rax
    ret

.Lpositive:
    movq $1, %rax
    ret

.Lnegative:
    movq $-1, %rax
    ret

# Macro example
.macro PRINT msg, len
    movq $1, %rax
    movq $1, %rdi
    leaq \msg(%rip), %rsi
    movq $\len, %rdx
    syscall
.endm

# Conditional assembly
#ifdef DEBUG
    .warning "Debug mode enabled"
#endif

# Align directive
    .align 16

# Section with flags
    .section .rodata
    format_string: .asciz "Value: %d\n"
