# A MIPS assembly implementation of Quake III Arena's Q_rsqrt algorithm
#
# Computes 1/sqrt(x), useful for angle of incidence calculations
# Compares the algorithm result to the true result
#
# float Q_rsqrt( float number )
# {
#   long i;
#   float x2, y;
#   const float threehalfs = 1.5F;
#
#   x2 = number * 0.5F;
#   y  = number;
#   i  = * ( long * ) &y;                       // evil floating point bit level hacking
#   i  = 0x5f3759df - ( i >> 1 );               // what?
#   y  = * ( float * ) &i;
#   y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
# //    y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
#
#   return y;
# }

.data

# Program input
input:          .float  2.0

# Constants
one:            .float  1.0
half:           .float  0.5
threehalfs:     .float  1.5
magic:          .word   0x5F3759DF

# Strings
msg_input:      .asciiz "Input x   : "
msg_fast:       .asciiz "Q_rsqrt   : "
msg_true:       .asciiz "1/sqrt(x) : "
msg_newline:    .asciiz "\n"

.text

main:
    # Print input message
    la   $a0, msg_input
    li   $v0, 4
    syscall

    # Print input value
    la   $t0, input
    lwc1 $f12, ($t0)
    li   $v0, 2
    syscall

    # Print newline
    la   $a0, msg_newline
    li   $v0, 4
    syscall

    # Fast inverse square root
    la   $a0, msg_fast
    li   $v0, 4
    syscall

    la   $t0, input
    lwc1 $f12, ($t0)
    jal  fast_inv_sqrt          # Result in $f0

    # Print result
    mov.s $f12, $f0
    li   $v0, 2
    syscall

    la   $a0, msg_newline
    li   $v0, 4
    syscall

    # True inverse square root
    la   $a0, msg_true
    li   $v0, 4
    syscall

    la   $t0, input
    lwc1 $f0, ($t0)
    sqrt.s $f0, $f0             # $f0 = sqrt(x)

    la   $t0, one
    lwc1 $f1, ($t0)
    div.s $f0, $f1, $f0         # $f0 = 1 / sqrt(x)

    # Print result
    mov.s $f12, $f0
    li   $v0, 2
    syscall

    la   $a0, msg_newline
    li   $v0, 4
    syscall

    li   $v0, 10
    syscall


# fast_inv_sqrt(x=$f12) -> $f0
#
# Computes an approximation of 1/sqrt(x).
fast_inv_sqrt:
    # Interpret int as float
    mfc1 $t0, $f12              # $t0 = *(int*)&x

    # Approximate 1/sqrt(x)
    srl  $t0, $t0, 1            # $t0 = i >> 1
    lw   $t1, magic             # $t1 = magic  (approx log2(1/sqrt(x)))
    sub  $t0, $t1, $t0          # $t0 = magic - (i >> 1)

    # Interpret float as int
    mtc1 $t0, $f0               # y = *(float*)&i

    # Refine using Newton's method
    # This exploits the fact that y = 1/sqrt(x) is asolution to 1/y^2 - x = 0
    la   $t0, half
    lwc1 $f2, ($t0)
    mul.s $f2, $f12, $f2        # $f2 = x * 0.5

    mul.s $f4, $f0, $f0         # $f4 = y * y
    mul.s $f4, $f2, $f4         # $f4 = (x * 0.5) * (y * y)

    la   $t0, threehalfs
    lwc1 $f3, ($t0)
    sub.s $f4, $f3, $f4         # $f4 = 1.5 - (x * 0.5 * y * y)

    mul.s $f0, $f0, $f4         # $f0 = y * (1.5 - (x * 0.5 * y * y))

    jr   $ra
