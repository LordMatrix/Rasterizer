.globl min
@ int min(int n1, int n2, int n3, int n4);
@ r0 - n1
@ r1 - n2
@ r2 - n3
@ r3 - n4

min:
	stmdb sp!, {r4}
	cmp r1, r0
	movlt r0, r1
	cmp r2, r0
	movlt r0, r2
	cmp r3, r0
	movlt r0, r3
	ldmia sp!,{r4}
bx lr

.globl max
@ int max(int n1, int n2, int n3, int n4)
@ r0 - n1
@ r1 - n2
@ r2 - n3
@ r3 - n4

max:
	stmdb sp!, {r4}
	cmp r1, r0
	movgt r0, r1
	cmp r2, r0
	movgt r0, r2
	cmp r3, r0
	movgt r0, r3
	ldmia sp!,{r4}
bx lr