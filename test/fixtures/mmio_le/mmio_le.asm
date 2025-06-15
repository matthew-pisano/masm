# Program to input and output a 4 character field

# Data area

.data

buffer:     .space 32       # Space for our user input characters


.text

# Memory mapped addresses of device fields.
.eqv kbInCtl 0xFFFF0000         # 0xFFFF0000 rcv contrl
.eqv kbInData 0xFFFF0004        # 0xFFFF0004 rcv data
.eqv dispOutCtl 0xFFFF0008      # 0xFFFF0008 tx contrl
.eqv dispOutData 0xFFFF000C     # 0xFFFF000c tx data


main:
# Get 4 characters from the keyboard and save them in the buffer
    la      $a0, buffer     # Load the address of the memory buffer into $a0
    li      $a1, 4          # Load the number of character to get into $a1
    jal     getNChar        # Run the getNChar procedure

# Put 4 characters from the buffer to the display
    la      $a0, buffer     # Load the address of the memory buffer into $a0
    li      $a1, 4          # Load the number of character to put into $a1
    jal     putNChar        # Run the putNChar procedure

# Exit
    li      $v0, 10         # Load exit cde into $v0
    syscall                 # Syscall


## Get Char Procedure ##

getNChar:
    la      $t0, kbInData       # Load address of input data word into $t0
    la      $t1, kbInCtl        # Load address of input control word into $t1
    li      $t2, 0              # Initialize counter

# Loop until all character have been read
getLoop:
    lb      $t3, 0($t1)         # Read control word
    andi    $t3, $t3, 0x0001    # Extract ready bit
    beq     $t3, $0, getLoop    # Keep polling till ready

    lb      $t4, 0($t0)         # Read character into temporary register from input data word
    add     $t5, $a0, $t2       # Get store address of character, offset from base buffer address
    sb      $t4, 0($t5)         # Store the byte into the buffer at the offset
    addi    $t2, $t2, 1         # Increment counter
    slt     $t5, $t2, $a1       # Check if the correct number of character has been read
    bne     $t5, $zero, getLoop # Loop if more character need to be read
    jr      $ra                 # Return to caller


## Put Char Procedure ##

putNChar:
    la      $t1, dispOutCtl     # Load address of output control word into $t1
    la      $t5, dispOutData    # Load address of output data word into $t5
    li      $t2, 0              # Initialize counter

# Loop until last character written out
putLoop:
    lb      $t3, 0($t1)         # Read control word
    andi    $t3, $t3, 0x0001    # Extract ready bit
    beq     $t3, $0, putLoop    # Poll till ready for next character

    add     $t4, $a0, $t2       # Get stored address of character, offset from base buffer address
    lbu     $t6, 0($t4)         # Load character into $t6
    sb      $t6, 0($t5)         # Store character into output data word at the offset
    addi    $t2, $t2, 1         # Increment counter
    slt     $t7, $t2, $a1       # Check if all characters have been written
    bne     $t7, $zero, putLoop # Loop if more character need to be written
    jr      $ra                 # Return to caller