main:
	li $t1, 24
	li $t0, -1
loop:	
	addi $a0, $t0, 0
	li $v0, 1
	syscall
	
	
	addi $a0, $zero, 32
	li $v0, 11
	syscall
	
	addi $t0, $t0, 1
	bgt $t0, $t1, exit
	j loop
exit:
	addi $a0, $zero, 47
	li $v0, 11
	syscall
	
	li $v0, 10
	syscall
