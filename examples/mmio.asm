.data
prompt: .asciiz "Enter a number: "
resp: .asciiz "Your number is: "
nl: .asciiz "\n"
input_buffer: .space 32     # Buffer for input string

# MARS MMIO addresses
.eqv RECEIVER_CONTROL   0xffff0000
.eqv RECEIVER_DATA      0xffff0004
.eqv TRANSMITTER_CONTROL 0xffff0008
.eqv TRANSMITTER_DATA   0xffff000c

.text

main:
    # Print prompt
    la $a0, prompt
    jal print_string

    # Read string input
    la $a0, input_buffer
    jal read_string

    # Print response
    la $a0, resp
    jal print_string

    # Print the input string back
    la $a0, input_buffer
    jal print_string

    # Print newline
    la $a0, nl
    jal print_string

    # Check if input is "0" - if not, loop
    la $t0, input_buffer
    lb $t1, 0($t0)          # Load first character
    lb $t2, 1($t0)          # Load second character

    # If first char is '0' and second is null/newline, exit
    li $t9, 48
    bne $t1, $t9, main      # 48 = '0'
    beq $t2, $zero, exit    # null terminator
    li $t9, 10
    beq $t2, $t9, exit      # newline
    j main

# Function: print_string
# Input: $a0 = address of null-terminated string
print_string:
    move $t0, $a0           # Copy string address

print_string_loop:
    lb $t1, 0($t0)          # Load byte from string
    beq $t1, $zero, print_string_done  # If null terminator, done

    # Wait for transmitter to be ready
print_string_wait:
    la $t9, TRANSMITTER_CONTROL
    lw $t2, ($t9)
    andi $t2, $t2, 1        # Check ready bit
    beq $t2, $zero, print_string_wait

    # Send character
    la $t9, TRANSMITTER_DATA
    sw $t1, ($t9)

    addi $t0, $t0, 1        # Move to next character
    j print_string_loop

print_string_done:
    jr $ra

# Function: read_string
# Input: $a0 = address of buffer to store string
read_string:
    move $t0, $a0           # Buffer pointer

read_string_loop:
    # Wait for receiver to have data
read_string_wait:
    la $t9, RECEIVER_CONTROL
    lw $t1, ($t9)
    andi $t1, $t1, 1        # Check ready bit
    beq $t1, $zero, read_string_wait

    # Read character
    la $t9, RECEIVER_DATA
    lw $t1, ($t9)

    # Check for Enter (newline)
    li $t9, 10
    beq $t1, $t9, read_string_done
    li $t9, 13
    beq $t1, $t9, read_string_done  # Also handle carriage return

    # Store character in buffer
    sb $t1, 0($t0)
    addi $t0, $t0, 1

    j read_string_loop

read_string_done:
    # Null terminate the string
    sb $zero, 0($t0)
    jr $ra

exit:
    li $v0, 10
    syscall