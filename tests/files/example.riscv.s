    .section .rodata
    .align 2
message_hello:
    .asciz "hello from riscv"
message_done:
    .asciz "done"
numbers:
    .word 1, 1, 2, 3, 5, 8, 13, 21

    .section .data
    .align 3
counter:
    .quad 0
buffer:
    .space 64

    .section .text
    .option push
    .option norelax
    .globl _start
    .type _start, @function

_start:
    la a0, message_hello
    call print_string
    la a0, numbers
    li a1, 8
    call sum_array
    mv s0, a0
    la a0, message_done
    call print_string
    j exit

print_string:
    addi sp, sp, -16
    sd ra, 8(sp)
    sd s0, 0(sp)
    mv s0, a0
1:
    lb t0, 0(s0)
    beq t0, zero, 2f
    addi s0, s0, 1
    j 1b
2:
    ld ra, 8(sp)
    ld s0, 0(sp)
    addi sp, sp, 16
    ret

sum_array:
    mv t0, a0
    mv t1, a1
    li t2, 0
3:
    beq t1, zero, 4f
    lw t3, 0(t0)
    add t2, t2, t3
    addi t0, t0, 4
    addi t1, t1, -1
    j 3b
4:
    mv a0, t2
    ret

atomic_increment:
    la t0, counter
5:
    lr.w t1, (t0)
    addi t1, t1, 1
    sc.w t2, t1, (t0)
    bnez t2, 5b
    ret

csr_snapshot:
    csrrw t0, cycle, zero
    csrrs t1, instret, zero
    csrrc t2, time, zero
    ret

copy_buffer:
    mv t0, a0
    mv t1, a1
    la t2, buffer
6:
    beq t1, zero, 7f
    lbu t3, 0(t0)
    sb t3, 0(t2)
    addi t0, t0, 1
    addi t2, t2, 1
    addi t1, t1, -1
    j 6b
7:
    ret

branch_demo:
    li t0, 10
    li t1, 20
    blt t0, t1, 8f
    bge t1, t0, 9f
8:
    addi t0, t0, 1
    j 10f
9:
    addi t1, t1, -1
10:
    ret

macro_demo:
    .macro SAVE_TWO reg1, reg2
        addi sp, sp, -16
        sd \reg1, 8(sp)
        sd \reg2, 0(sp)
    .endm

    .macro RESTORE_TWO reg1, reg2
        ld \reg1, 8(sp)
        ld \reg2, 0(sp)
        addi sp, sp, 16
    .endm

    SAVE_TWO ra, s0
    RESTORE_TWO ra, s0
    ret

load_data_address:
    auipc t0, %pcrel_hi(message_hello)
    addi t0, t0, %pcrel_lo(message_hello)
    auipc t1, %hi(message_done)
    addi t1, t1, %lo(message_done)
    ret

vector_demo:
    vsetvli t0, a0, e32, ta, ma
    vle32.v v0, (a0)
    vadd.vv v1, v0, v0
    vse32.v v1, (a1)
    ret

exit:
    li a7, 93
    li a0, 0
    ecall

    .option pop
