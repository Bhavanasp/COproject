.word 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35

main : addi $s0,$zero,0x0
	   addi $t0,$zero,1
	   addi $t3,$zero,3

	   addi $t1,$zero,0
	   
	   for : slti $t2,$t1,36
	   		 beq $t2,$zero exit

	   		 bne $t0,$t3 else

	   		 addi $t4,$t1,100
	   		 sw $t4,0($s0)
	   		 addi $t0,$zero,0
	   		 j L

	   		 else :

	   		 lw $t4,0($s0)

	   		 L : addi $t0,$t0,1
	   		 	 addi $s0,$s0,4
	   		 	 addi $t1,$t1,1
	   	j for
	   		
	   	exit : jr $ra