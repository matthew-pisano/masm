.data
msg: .asciiz "Hello there"

.text

main:
    la $a0, msg         # Load in address of message
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the message

    li $v0, 10
    syscall             # Exit gracefully