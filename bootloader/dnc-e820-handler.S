//
// Id: $Id$
//
// Copyright © 2009 Numascale AS, Oslo, Norway
// Author: Arne Georg Gleditsch
//
// Distribution restricted.
//

#include "hw-config.h"

#define MSR_HWCR      0xc0010015
#define MSR_TOPMEM    0xc001001a
#define MSR_TOPMEM2   0xc001001d
#define MSR_MCFG_BASE 0xc0010058
#define MSR_SYSCFG    0xc0010010
#define MSR_NB_CFG    0xc001001f
#define MSR_APIC_BAR  0x0000001b
#define MSR_LSCFG     0xc0011020
#define MSR_NODE_ID   0xc001100c
#define MSR_SMM_BASE  0xc0010111

#define MSR_MTRR_PHYS_BASE0 0x00000200
#define MSR_MTRR_PHYS_BASE1 0x00000202
#define MSR_MTRR_PHYS_BASE2 0x00000204
#define MSR_MTRR_PHYS_BASE3 0x00000206
#define MSR_MTRR_PHYS_BASE4 0x00000208
#define MSR_MTRR_PHYS_BASE5 0x0000020a
#define MSR_MTRR_PHYS_BASE6 0x0000020c
#define MSR_MTRR_PHYS_BASE7 0x0000020e

#define MSR_MTRR_PHYS_MASK0 0x00000201
#define MSR_MTRR_PHYS_MASK1 0x00000203
#define MSR_MTRR_PHYS_MASK2 0x00000205
#define MSR_MTRR_PHYS_MASK3 0x00000207
#define MSR_MTRR_PHYS_MASK4 0x00000209
#define MSR_MTRR_PHYS_MASK5 0x0000020b
#define MSR_MTRR_PHYS_MASK6 0x0000020d
#define MSR_MTRR_PHYS_MASK7 0x0000020f

#define MSR_IORR_PHYS_BASE0 0xc0010016
#define MSR_IORR_PHYS_BASE1 0xc0010018
#define MSR_IORR_PHYS_MASK0 0xc0010017
#define MSR_IORR_PHYS_MASK1 0xc0010019
        
#define EXPORT(sym) .global sym ## _relocate; sym ## _relocate: sym:

#define SYM_OFFSET(sym) ((sym) - asm_relocate_start)

	.code16
	.text
	.align 4096

	.global asm_relocate_start
asm_relocate_start:

EXPORT(init_trampoline)
	invd
	mov	%cs, %ax
	add	$SYM_OFFSET(stack_start)/16, %ax
	mov	%ax, %ss
	mov	$(stack_end - stack_start), %sp

	/* Disable caching */
	wbinvd
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0

	/* Disable all MTRR entries */
	mov	$MSR_MTRR_PHYS_MASK0, %ecx
1:	
	xor	%eax, %eax
	xor	%edx, %edx
	wrmsr
	add	$2, %ecx
	cmp	$MSR_MTRR_PHYS_MASK7, %ecx
	jna	1b

	/* Add stored MTTRs */
	mov	$MSR_MTRR_PHYS_BASE0, %ecx
	mov	$SYM_OFFSET(new_mtrr_base), %esi
	mov	$SYM_OFFSET(new_mtrr_mask), %edi
1:	
	mov	%cs:(%esi), %eax
	add	$4, %esi
	mov	%cs:(%esi), %edx
	add	$4, %esi
	wrmsr
	inc	%ecx

	mov	%cs:(%edi), %eax
	add	$4, %edi
	mov	%cs:(%edi), %edx
	add	$4, %edi
	wrmsr
	inc	%ecx

	cmp	$MSR_MTRR_PHYS_MASK7, %ecx
	jna	1b

	/* Reenable caching */
	wbinvd
	mov	%cr0, %eax
	and	$~0x40000000, %eax
	mov	%eax, %cr0

	/* Set TOP_MEM2 */
	mov	%cs:SYM_OFFSET(new_topmem_msr), %eax
	mov	%cs:SYM_OFFSET(new_topmem_msr+4), %edx
	mov	$MSR_TOPMEM2, %ecx
	wrmsr

	mov	$SYM_OFFSET(cpu_cr), %si
	call	print_strz

	mov	%cr0, %eax
	call	print_hex32

	mov	%cr3, %eax
	call	print_hex32

	mov	%cr4, %eax
	call	print_hex32

	mov	$SYM_OFFSET(cpu_done1), %si
	call	print_strz

	mov	$MSR_APIC_BAR, %ecx
	rdmsr
	push	%eax
	call	print_hex32
	mov	$0x70800, %eax
	mov	$MSR_APIC_BAR, %ecx
	wrmsr
	mov	$MSR_APIC_BAR, %ecx
	rdmsr
	call	print_hex32
	mov	$0x7000, %ax
	mov	%ax, %es
	mov	%es:0x20, %eax
	call	print_hex32
	mov	%es:0x30, %eax
	call	print_hex32

	mov	%cs:SYM_OFFSET(cpu_apic_renumber), %al
	shl	$24, %eax
	mov	%eax, %es:0x20
	mov	%es:0x20, %eax
	call	print_hex32

	pop	%eax
	and	$~0x100, %eax	# Clear BSP flag to let core accept INIT and STARTUP IPIs.
	mov	$MSR_APIC_BAR, %ecx
	wrmsr
	rdmsr
	call	print_hex32

	mov	$MSR_NODE_ID, %ecx
	rdmsr

	/* "Bios scratch" given as [11:6], so we're limited to an
	   8-bit prefix here for the time being.  Ideally we want 8 bits,
	   and since all upper bits of this MSR appear to be r/w, we
	   could just take some liberties with the register. */
	and	$~0xfc0, %eax
	xor	%ebx,%ebx
	mov	%cs:SYM_OFFSET(cpu_apic_hi), %bl
	shl	$6, %ebx
	or	%ebx, %eax
	wrmsr

	mov	$MSR_MCFG_BASE, %ecx
	mov	%cs:SYM_OFFSET(new_mcfg_msr), %eax
	mov	%cs:SYM_OFFSET(new_mcfg_msr+4), %edx
	wrmsr

	// ERRATA #N28: Disable HT Lock mechanism. 
	// AMD Email dated 31.05.2011 :
	// There is a switch that can help with these high contention issues,
	// but it isn't "productized" due to a very rare potential for live lock if turned on.
	// Given that HUGE caveat, here is the information that I got from a good source:
	// LSCFG[44] =1 will disable it. MSR number is C001_1020.
	mov	$MSR_LSCFG, %ecx
	rdmsr
	or	$0x1000, %edx
	wrmsr

	mov	$MSR_TOPMEM, %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(rem_topmem_msr)
	mov	%edx, %cs:SYM_OFFSET(rem_topmem_msr+4)

	mov	$MSR_SMM_BASE, %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(rem_smm_base_msr)
	mov	%edx, %cs:SYM_OFFSET(rem_smm_base_msr+4)
        
	mov	$SYM_OFFSET(cpu_done2), %si
	call	print_strz

	wbinvd
	movb	$1, %cs:SYM_OFFSET(cpu_init_finished)
1:	
	hlt
	jmp	1b 

cpu_mtrr:
	.asciz	"\r\nMTRR "
cpu_cr:
	.asciz	"\r\nCRx "
cpu_done1:
	.asciz	"\r\nCPU "
cpu_done2:
	.asciz  " done setting MTRRs.\r\n"

EXPORT(cpu_init_finished)
	.byte 0
EXPORT(cpu_apic_renumber)
	.byte 0
EXPORT(cpu_apic_hi)
	.byte 0
EXPORT(new_mcfg_msr)
	.long 0, 0
EXPORT(new_topmem_msr)
	.long 0, 0
EXPORT(new_mtrr_base)
	.skip 8*8, 0
EXPORT(new_mtrr_mask)
	.skip 8*8, 0
EXPORT(rem_topmem_msr)
	.long 0, 0
EXPORT(rem_smm_base_msr)
	.long 0, 0

// Print asciiz string at by cs:si
print_strz:
#if DEBUG_TRAMP
	xor	%bx, %bx
	xor	%dx, %dx
1:	
	mov	%cs:(%si), %al
	inc	%si
	or	%al, %al
	jz	2f
	mov	$0x0e, %ah
	int	$0x10
	mov	$0x01, %ah
	int	$0x14
	jmp	1b
2:	
#endif
	ret


hex32_buf:	
	.asciz	"[--------] "

print_hex32:
#if DEBUG_TRAMP
	push	%eax
	push	%ebx
	push	%ecx
	push	%edx
	push	%esi
	mov	%eax, %ebx
	mov	$8, %ecx
	mov	$SYM_OFFSET(hex32_buf)+8, %si
	
1:	
	mov	%bl, %al
	and	$0x0f, %al
	add	$0x30, %al
	cmp	$0x3a, %al
	jb	2f
	add	$0x27, %al
2:	
	mov	%al, %cs:(%si)
	dec	%si
	shr	$4, %ebx
	loop	1b

	mov	$SYM_OFFSET(hex32_buf), %si
	call	print_strz
	pop	%esi
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%eax
#endif
	ret

	.align 16
stack_start:  .skip 4096, 0
stack_end:	

	.align 16
EXPORT(old_int15_vec)
	.word 0,0
EXPORT(new_e820_len)
	.word 0
EXPORT(new_e820_map)
	.skip 2560, 0

EXPORT(new_e820_handler)
	cmp	$0xe820, %eax
	jne	1f
	cmp	$0x534d4150, %edx
	je	2f
1:	ljmp	%cs:*SYM_OFFSET(old_int15_vec)
//addr16	ljmp	%cs:*SYM_OFFSET(old_int15_vec)

2:	mov	$0x534d4150, %eax
	cmp	$20, %ecx
	jl	1f
	test	$0xffff0000, %ebx
	jnz	1f
	cmp	%cs:SYM_OFFSET(new_e820_len), %bx
	jg	1f

	push	%ds
	push	%cs
	pop	%ds
	push	%di
	push	%esi
	shl	$2, %ebx
	leal	SYM_OFFSET(new_e820_map)(%ebx, %ebx, 4), %esi
	mov	$5, %ecx
	rep	movsl
	pop	%esi
	pop	%di
	pop	%ds
	shr	$2, %ebx
	inc	%ebx
	mov	$20, %ecx
	and	$~1, 4(%esp)	# Clear carry flag
	cmp	%cs:SYM_OFFSET(new_e820_len), %bx 
	jge	2f
	iret

1:	or	$1, 4(%esp)	# Set carry flag to signify error
2:	xor	%ebx, %ebx
	iret
	
	.align 16

EXPORT(new_mpfp)
	.skip 16, 0
EXPORT(new_mptable)
	.skip 768, 0

	.global asm_relocate_end
asm_relocate_end:


// Replacement code for brute-force SMM disable

	.code16
	.text
	.align 4096

	.global smm_handler_start
smm_handler_start:
	rsm
	.global smm_handler_end
smm_handler_end:
