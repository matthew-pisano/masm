.data

string: .asciiz "hello\n"

.text

main:
    # Print String
	li $v0, 4
	la $a0, string
	syscall

	# Set rand seed
	li $v0, 40
	li $a0, 0
	li $a1, 42
	syscall

	# Get random float
	li $v0, 43
	li $a0, 0
	syscall

	# Move from $f0 to $f12
	mov.s $f12, $f0

	# Print float
	li $v0, 2
	syscall

	li $v0, 10
    syscall