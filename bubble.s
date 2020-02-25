#########################################################################################################################

# There should be a main in the code
# There should be "jr $ra" to indicate the end of the code, else the program goes into an infinte loop
# .data,.text,.globl main are ignored( they are not currently handled, so should not be there in the code)
# only .word is used to put data in the datasegment and the memmory allocation starts from index 0 in the dataseg array
# labels are recognised by ':' after them
# for load word and store word base addresses should be multiples of 4 and less than 600
# offsets in lw and sw should be multiples of 4 

#################################################### BUBBLE SORT ########################################################

.word 7, 1, 0, 2

main:	addi $s0,$zero,0x0           # storing address in $s0
	    add $t0,$zero,$zero
	    addi $t5,$zero,2
	
	for1 : slti $t4,$t0,3
	       beq $t4,$zero exit
	       addi $t1,$zero,-1
	       add $s1,$zero,$s0
	       sub $t6,$t5,$t0
	       for2 : slt $t7,$t1,$t6
	       	      beq $t7,$zero L2
		      lw $t2,0($s1)
		      lw $t3,4($s1)
		      slt $t4,$t3,$t2
		      beq $t4,$zero L1
		      	sw $t3,0($s1)
		      	sw $t2,4($s1)
		      
		      L1 : addi $s1,$s1,4
			       addi $t1,$t1,1
		      j for2

	    L2 : addi $t0,$t0,1
		j for1

	    exit : jr $ra