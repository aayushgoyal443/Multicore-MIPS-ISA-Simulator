# Assembly program to calculate the algebraic area under the curve formed by given cartesian co-ordinates

.text
.globl main

main:

    # t0 = sum, intialized to 0, which will store the value sigma(i=1 to n)(y_i + y_i-1)(x_i - x_i-1) after i iterations. 
    # The actual area is half of this sum value.
    li $t0,0                

    li $v0,5
    syscall
    move $t1,$v0                # t1 = n, the total number of points

    li $v0,5
    syscall
    move $t2,$v0                # t2 = x0

    li $v0,5
    syscall
    move $t3,$v0                # t3 = y0

    addi $t1,$t1,-1             # because we have to do only n-1 iterations to get the desired result.

loop:
    beq $t1,$zero,loop_exit     # When the value of n becomes 0 we exit the loop (by jumping to loop_exit)

    li $v0,5
    syscall
    move $t4,$v0                # t4 = x1

    li $v0,5
    syscall
    move $t5,$v0                # t5 = y1

    sub $t6,$t4,$t2             # t6 = x1-x0
    add $t7,$t5,$t3             # t7 = y1+y0

    mult $t6,$t7    
    mflo $t8                    # t8 = (y1+y0)*(x1-x0)
    add $t0,$t0,$t8             # update sum = sum + (y1+y0)*(x1-x0)

    move $t2,$t4                # update x, x0 = x1
    move $t3,$t5                # update y, y0 = y1 

    # decrement the value of 'n' else we will be stuck in an infinite loop
    addi $t1,$t1,-1             

    # jump back and check whether the loop is to be executed again or not, accordingly do the needful
    j loop                   

loop_exit:

    mtc1 $t0, $f1
    cvt.s.w $f1, $f1            # Converted the integer sum into a float value, f1 = sum

    li.s $f2, 2.0;              # f2 = 2.0
    div.s $f12, $f1, $f2        # f12 = sum/2 including decimal, because area is half of the value calculated in sum, 
                                # as noted earlier

    #print on console directly, the value stored in $f12 gets printed just like for integer $a0 gets printed
    li $v0,2                    
    syscall                     

    li $v0,10
    syscall                     # exit the code