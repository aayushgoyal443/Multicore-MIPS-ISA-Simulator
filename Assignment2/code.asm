# Assembly program to evaluate postfix expression

.text
.globl main

main:

    li $s0,0        #sum variable

    li $v0,8        #Taking string as input
    la $a0,buffer
    li $a1,52       #Allocating maximum space to string
    syscall
    move $s1,$a0    # iterator 

loop:

    lb $s2,($s1)
    beq $s2,$zero,loop_exit

    #ASCII bounds
    li $t0,48   # 0
    li $t1,57   # 9
    li $t2,42   # *
    li $t3,42   # +
    li $t4,45   # -

    addi $s2,$s2,1

    li $v0,11
    move $a0,$s2
    syscall

    addi $s0,$s0,1
    addi $s1,$s1,1

    j loop


char_error:

    li $v0,4
    la $a0,error
    syscall

    li $v0,10
    syscall

loop_exit:

    li $v0,1
    move $a0,$s0
    syscall

    li $v0,10
    syscall

.data
buffer: .space 52
error: .asciiz "\nPlease enter a valid character\n"