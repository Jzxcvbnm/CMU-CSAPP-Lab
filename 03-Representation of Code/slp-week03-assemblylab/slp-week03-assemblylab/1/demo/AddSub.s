
INCLUDE Irvine32.inc


.section .rodata

longIntFmt:
    .string "%ld"

resultFmt:
    .string "Result is %ld\n"

.section .text
    .globl addLongs
addLongs:
    addq %rdi, %rsi
    movq %rsi, (%rdx)
    ret

    .globl main
main:
    pushq %rbp
    subq $32, %rsp
    movq %rsp, %rbp

    movq $longIntFmt, %rdi
    leaq 0(%rbp), %rsi
    call scanf

    movq $longIntFmt, %rdi
    leaq 8(%rbp), %rsi
    call scanf

    movq 0(%rbp), %rdi
    movq 8(%rbp), %rsi
    leaq 16(%rbp), %rdx
    call addLongs
    movq $resultFmt, %rdi
    movq 16(%rbp), %rsi
    call printf
    addq $32, %rsp
    popq %rbp
    ret
