.text

main:
    # Load immediate values into registers
    li $t0, 10
    li $t1, 20
    li $t2, 2

    add $t3, $t0, $t1   # $t3 = $t0 + $t1 (10 + 20 = 30)

    mul $t4, $t3, $t2   # $t4 = $t3 * $t2 (30 * 2 = 60)

    move $a0, $t4       # Move result to argument register $a0
    li $v0, 1           # Load system call code for printing an integer
    syscall

    # Exit program
    li $v0, 10          # Load system call code for exit
    syscall
