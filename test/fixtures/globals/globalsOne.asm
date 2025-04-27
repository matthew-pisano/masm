.data

local_string: .asciiz "Hello, World again!"

.text
main:
    li $v0, 4
    la $a0, global_string
    syscall

    li $v0, 4
    la $a0, local_string
    syscall

    li $v0, 10
    syscall
