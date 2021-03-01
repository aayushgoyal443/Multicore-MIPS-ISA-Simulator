# Assembly program to evaluate postfix expression

.text
.globl main

main:

    li $s0,0        #current size of stack

    li $v0,8        #Taking string as input
    la $a0,buffer
    li $a1,52       #Allocating maximum space to string
    syscall
    move $s1,$a0    # iterator 

    #ASCII bounds
    li $s3,48   # 0
    li $s4,57   # 9
    li $s5,42   # *
    li $s6,43   # +
    li $s7,45   # -

loop:

    lb $s2,($s1)
    beq $s2,$zero,loop_exit

    beq $s2,$s5,multiply
    beq $s2,$s6,addition
    beq $s2,$s7,subtraction

    # slt $t4,$s2,$s3
    # bne $t4,$zero,char_error
    # sgt $t4,$s2,$s4
    # bne $t4,$zero,char_error

    addi $s2,$s2,-48
    addi $sp,$sp,-4
    sw $s2,($sp)
    addi $s0,$s0,1

    addi $s1,$s1,1
    j loop

multiply:
    slti $t0,$s0,2
    bne $t0,$zero,exp_error
    addi $s0,$s0,-1

    lw $t1,($sp)
    lw $t2,4($sp)
    addi $sp,$sp,4

    mult $t1, $t2			
    mflo $t3

    sw $t3,($sp)
    j loop					


addition:
    slti $t0,$s0,2
    bne $t0,$zero,exp_error
    addi $s0,$s0,-1

    lw $t1,($sp)
    lw $t2,4($sp)
    addi $sp,$sp,4

    add	$t3, $t1, $t2
    sw $t3,($sp)
    j loop

subtraction:
    slti $t0,$s0,2
    bne $t0,$zero,exp_error
    addi $s0,$s0,-1

    lw $t1,($sp)
    lw $t2,4($sp)
    addi $sp,$sp,4

    sub	$t3, $t2, $t1
    sw $t3,($sp)
    j loop

char_error:

    li $v0,4
    la $a0,error
    syscall

    li $v0,10
    syscall

exp_error:

    li $v0,4
    la $a0,error2
    syscall

    li $v0,10
    syscall

loop_exit:

    li $t5,2
    bne	$s0,$t5,exp_error

    li $v0,1
    lw $a0,($sp)
    syscall

    li $v0,10
    syscall

.data
buffer: .space 52
error: .asciiz "\nInvalid character\n"
error2: .asciiz "\nInvalid expression\n"
