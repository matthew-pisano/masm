.data
    num1: .float 10.5
    num2: .float 2.25

.text

main:
    # Load float values from memory into FPU registers
    lw $t0, num1
    lw $t1, num2
    mtc1 $t0, $f0
    mtc1 $t1, $f1

    add.s $f2, $f0, $f1     # $f2 = $f0 + $f1 (10.5 + 2.25 = 12.75)

    mov.s $f12, $f2         # Move result to $f12 for printing
    li $v0, 2               # Load system call code for print_float
    syscall

    # Exit program
    li $v0, 10              # Load system call code for exit
    syscall
