.data
prompt: .asciiz "Enter a number: "
resp: .asciiz "Your number is: "
nl: .asciiz "\n"

.text

main:
    la $a0, prompt      # Load in address of prompt
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the prompt

    li $v0, 5           # Set syscall flag to read integer
    syscall             # Print the message

    move $t0, $v0

    la $a0, resp        # Load in address of response
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the response

    li $v0, 1           # Set syscall flag to print integer
    move $a0, $t0
    syscall             # Print the integer

    la $a0, nl          # Load in address of newline
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the prompt

    bne $t0, $zero, main

    li $v0, 10
    syscall             # Exit gracefully