.text
.globl main

main:
    li $t0,0        #Initialize sum to zero

    li $v0,5
    syscall
    move $t1,$v0    #n

    li $v0,5
    syscall
    move $t2,$v0    #x0

    li $v0,5
    syscall
    move $t3,$v0    #y0

    addi $t1,$t1,-1

loop:
    beq $t1,$zero,loop_exit

    li $v0,5
    syscall
    move $t4,$v0    #x1

    li $v0,5
    syscall
    move $t5,$v0    #y1

    sub $t6,$t4,$t2 #x1-x0
    add $t7,$t5,$t3 #y1+y0

    mult $t6,$t7    
    mflo $t8
    add $t0,$t0,$t8 #update sum

    move $t2,$t4    #update x
    move $t3,$t5    #update y

    addi $t1,$t1,-1

    j loop

loop_exit:

    srl $t0,$t0,1

    li $v0,1            #print on console
    move $a0,$t0
    syscall

    li $v0,10
    syscall

