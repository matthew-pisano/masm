# Sorts an array of integers using recursive merge sort.

.data

array:      .word 38, 27, 43, 3, 9, 82, 10, 1
array_size: .word 8
temp_array: .space 32           # Scratch space for merge (8 words)

msg_before: .asciiz "Before: "
msg_after:  .asciiz "After:  "
msg_space:  .asciiz " "
msg_newline: .asciiz "\n"

.text

main:
    # Print array before sorting
    la   $a0, msg_before
    li   $v0, 4
    syscall

    la   $a0, array
    lw   $a1, array_size
    jal  print_array

    # Call merge_sort(array, 0, size-1)
    la   $a0, array
    li   $a1, 0
    lw   $a2, array_size
    subi $a2, $a2, 1            # right = size - 1
    jal  merge_sort

    # Print array after sorting
    la   $a0, msg_after
    li   $v0, 4
    syscall

    la   $a0, array
    lw   $a1, array_size
    jal  print_array

    # Exit
    li   $v0, 10
    syscall


# merge_sort(arr=$a0, left=$a1, right=$a2)
#
# Sorts the given array with the merge sort algorithm
merge_sort:

    # Prologue
    subi $sp, $sp, 20
    sw   $ra, 0($sp)
    sw   $s0, 4($sp)
    sw   $s1, 8($sp)
    sw   $s2, 12($sp)
    sw   $s3, 16($sp)

    move $s0, $a0               # s0 = arr
    move $s1, $a1               # s1 = left
    move $s2, $a2               # s2 = right

    # Base case: if left >= right, then return
    bge  $s1, $s2, ms_done

    # mid = (left + right) / 2
    add  $s3, $s1, $s2
    srl  $s3, $s3, 1

    # merge_sort(arr, left, mid)
    move $a0, $s0
    move $a1, $s1
    move $a2, $s3
    jal  merge_sort

    # merge_sort(arr, mid+1, right)
    move $a0, $s0
    addi $a1, $s3, 1
    move $a2, $s2
    jal  merge_sort

    # merge(arr, left, mid, right)
    move $a0, $s0
    move $a1, $s1
    move $a2, $s3
    move $a3, $s2
    jal  merge

ms_done:
    # Epilogue
    lw   $ra, 0($sp)
    lw   $s0, 4($sp)
    lw   $s1, 8($sp)
    lw   $s2, 12($sp)
    lw   $s3, 16($sp)
    addi $sp, $sp, 20
    jr   $ra


# merge(arr=$a0, left=$a1, mid=$a2, right=$a3)
#
# Helper method that merges arr[left...mid] and arr[mid+1...right] using temp_array as scratch space.
merge:
    # Prologue
    subi $sp, $sp, 24
    sw   $ra, 0($sp)
    sw   $s0, 4($sp)
    sw   $s1, 8($sp)
    sw   $s2, 12($sp)
    sw   $s3, 16($sp)
    sw   $s4, 20($sp)

    move $s0, $a0               # $s0 = arr
    move $s1, $a1               # $s1 = left
    move $s2, $a2               # $s2 = mid
    move $s3, $a3               # $s3 = right
    la   $s4, temp_array        # $s4 = temp

    # Copy arr[left...right] into temp[0...right - left]
    # $t0 = source index (i = left)
    # $t1 = dest index (k = 0)
    move $t0, $s1
    li   $t1, 0

copy_loop:
    bgt  $t0, $s3, copy_done    # while i <= right

    sll  $t2, $t0, 2            # Byte offset = i * 4
    add  $t2, $s0, $t2          # Get offset from array base address

    lw   $t3, 0($t2)            # $t3 = arr[i]

    sll  $t4, $t1, 2            # Byte offset = dest index * 4
    add  $t4, $s4, $t4          # Get offset into temp array

    sw   $t3, 0($t4)            # temp[k] = arr[i]

    # Increment pointers
    addi $t0, $t0, 1
    addi $t1, $t1, 1
    j    copy_loop
copy_done:

    # Merge temp back into arr[left...right]
    # i = 0          (left half cursor into temp)
    # j = mid-left+1 (right half cursor into temp)
    # k = left       (write cursor into arr)
    li   $t0, 0                 # i = 0
    sub  $t1, $s2, $s1
    addi $t1, $t1, 1            # j = mid - left + 1
    move $t2, $s1               # k = left

    # Precompute size of left half and total size for bounds
    sub  $t5, $s2, $s1
    addi $t5, $t5, 1            # left_size = mid - left + 1
    sub  $t6, $s3, $s1
    addi $t6, $t6, 1            # total_size = right - left + 1

merge_loop:
    bgt  $t2, $s3, merge_done   # while k <= right

    # Check if left half exhausted (i >= left_size)
    bge  $t0, $t5, take_right

    # Check if right half exhausted (j >= total_size)
    bge  $t1, $t6, take_left

    # Compare temp[i] vs temp[j]
    sll  $t7, $t0, 2
    add  $t7, $s4, $t7
    lw   $t8, 0($t7)            # $t8 = temp[i]
    sll  $t7, $t1, 2
    add  $t7, $s4, $t7
    lw   $t9, 0($t7)            # $t9 = temp[j]

    ble  $t8, $t9, take_left

take_right:
    sll  $t7, $t1, 2
    add  $t7, $s4, $t7
    lw   $t8, 0($t7)            # value = temp[j]
    addi $t1, $t1, 1            # j++
    j    write_val

take_left:
    sll  $t7, $t0, 2
    add  $t7, $s4, $t7
    lw   $t8, 0($t7)            # value = temp[i]
    addi $t0, $t0, 1            # i++

write_val:
    sll  $t7, $t2, 2
    add  $t7, $s0, $t7
    sw   $t8, 0($t7)            # arr[k] = value
    addi $t2, $t2, 1            # k++
    j    merge_loop

merge_done:
    # Epilogue
    lw   $ra, 0($sp)
    lw   $s0, 4($sp)
    lw   $s1, 8($sp)
    lw   $s2, 12($sp)
    lw   $s3, 16($sp)
    lw   $s4, 20($sp)
    addi $sp, $sp, 24
    jr   $ra


# print_array(arr=$a0, size=$a1)
#
# Prints out the given array with the given size
print_array:
    # Prologue
    subi $sp, $sp, 16
    sw   $ra, 0($sp)
    sw   $s0, 4($sp)
    sw   $s1, 8($sp)
    sw   $s2, 12($sp)

    move $s0, $a0               # $s0 = arr
    move $s1, $a1               # $s1 = size
    li   $s2, 0                 # $s2 = i = 0

print_loop:
    bge  $s2, $s1, print_done   # while i < size

    sll  $t0, $s2, 2
    add  $t0, $s0, $t0

    # Print arr[i]
    lw   $a0, 0($t0)
    li   $v0, 1
    syscall

    # Print space
    la   $a0, msg_space
    li   $v0, 4
    syscall

    addi $s2, $s2, 1            # i++

    j    print_loop

print_done:
    # Print newline
    la   $a0, msg_newline
    li   $v0, 4
    syscall

    # Epilogue
    lw   $ra, 0($sp)
    lw   $s0, 4($sp)
    lw   $s1, 8($sp)
    lw   $s2, 12($sp)
    addi $sp, $sp, 16
    jr   $ra
