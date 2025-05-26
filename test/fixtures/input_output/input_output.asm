.data

promptmsg:  .asciiz "Enter a number to calculate the factorial: "
outmsg:     .asciiz "The factorial is: "


.text

main:
    li      $v0, 4                  # Load code for integer input in $v0
    la      $a0, promptmsg          # Load address of prompt message in $a0
    syscall                         # Syscall to ask for input using address in argument
    
    li      $v0, 5                  # Load code for integer input in $v0
    syscall                         # Syscall to read integer input
    move    $a0, $v0                # Move input value into $a0 for factorial calculation

    jal     fact                    # Call factorial procedure
    move    $s1, $v0                # Move returned value into $v0

    li      $v0, 4                  # Load code for integer output in $v0
    la      $a0, outmsg             # Load address of output message in $a0
    syscall                         # Syscall to output an integer with a message
    
    li      $v0, 1                  # Load code for integer output in $v0
    move    $a0, $s1                # Move result into $a0 for output
    syscall                         # Syscall to output the result

    li      $v0, 10                 # Load code for exit
    syscall                         # Syscall


fact:
    # "prolog"
    addi    $sp, $sp, -8            # Adjust stack for 2 words
    sw      $ra, 4($sp)             # Save return address
    sw      $a0, 0($sp)             # Save argument

    slti    $t0, $a0, 1             # Test for n < 1 and store in $t0
    beq     $t0, $zero, recurse     # Branch to recursive case if n >= 1

    # "epilog" for base case
    addi    $v0, $zero, 1           # Store 1 in $v0 to return
    addi    $sp, $sp, 8             # Pop 2 items from stack, nothing is modified, so nothing is restored
    jr      $ra                     # Return to caller (originall call or recursive ancestor)

recurse:
    addi    $a0, $a0, -1            # Else decrement n
    jal     fact                    # Recursive call to fact(n - 1)
    lw      $a0, 0($sp)             # Restore original n
    lw      $ra, 4($sp)             # Restore return address

    # "epilog" for recursive case
    addi    $sp, $sp, 8             # Pop 2 items from stack
    mul     $v0, $a0, $v0           # Multiply to get result and store in $v0
    jr      $ra                     # Return to caller (originall call or recursive ancestor)

