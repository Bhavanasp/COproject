
.word 4

#the factorial value is finally stored in $s4

fact :	add $s1,$zero,$s4
	add $s3,$zero,$zero
	beq $s2,$zero exit
	addi $s2,$s2,-1

	m : beq $s2,$s3 fact
	    addi $s3,$s3,1
	    add $s4,$s4,$s1
	j m

main :
	addi $s0,$s0,0
	sll $s0,$s0,16
	lw $s1,0($s0)
	beq $s1,$zero L
	addi $s2,$s1,-1
	add $s4,$zero,$s1
	j fact
	L : addi $s4,$zero,1 	#fact(0) is 1

exit :  sw $s4,4($s0)
        jr $ra
