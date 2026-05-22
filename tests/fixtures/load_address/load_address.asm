.data

label: .asciiz "hello there"
label2: .asciiz "hello there"

.text
    la $s0, label
    lw $s1, ($s0)

    add $a0, $zero, $s1
	li $v0, 11
	syscall

	li $v0, 10
    syscall