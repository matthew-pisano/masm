# Hello, World!
.data ## Data declaration section
## String to be printed:

out_string: .asciiz "\nHello, World!"
out_string2: .asciiz "\nHello, World!"
label: .word 42
.text ## Assembly language instructions go in text segment
main: ## Start of code section
    addiu $v0, $zero, 4 # system call code for printing string = 4
    la $a0, out_string # load address of string to be printed into $a0
    syscall # call operating system to perform operation
    # specified in $v0
    # syscall takes its arguments from $a0, $a1, ...
    addiu $v0, $zero, 10 # terminate program
    syscall
    
.data

tmp: .space 5
