/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hw-config.h"
#include "dnc-defs.h"

#define EXPORT(sym) .global sym ## _relocate; sym ## _relocate: sym:
#define SYM_OFFSET(sym) ((sym) - asm_relocate_start)
#define STATUS(val) movl $val, %cs:SYM_OFFSET(cpu_status)

	.code16
	.text
	.align 4096

	.global asm_relocate_start
asm_relocate_start:
EXPORT(init_dispatch)
	cli

	mov	%cs, %ax
	add	$SYM_OFFSET(stack_start)/16, %ax
	mov	%ax, %ss
	mov	$(stack_end - stack_start), %sp

	movl	%cs:SYM_OFFSET(cpu_status), %edx
	cmpl	$VECTOR_TRAMPOLINE, %edx
	je	init_trampoline
	cmpl	$VECTOR_PROBEFILTER_EARLY_f10, %edx
	je	init_probefilter_early_f10
	cmpl	$VECTOR_PROBEFILTER_EARLY_f15, %edx
	je	init_probefilter_early_f15
	cmpl	$VECTOR_ENABLE_CACHE, %edx
	je	init_enable_cache
1:
	cli
	hlt
	jmp	1b

	.align 16
init_trampoline:
	STATUS(4)

	/* Disable caching */
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0
	wbinvd

	/* Disable all MTRR entries */
	mov	$MSR_MTRR_PHYS_MASK0, %ecx
1:
	xor	%eax, %eax
	xor	%edx, %edx
	wrmsr
	add	$2, %ecx
	cmp	$MSR_MTRR_PHYS_MASK7, %ecx
	jna	1b

	/* Set default memory type */
	mov	%cs:SYM_OFFSET(new_mtrr_default), %eax
	mov	%cs:SYM_OFFSET(new_mtrr_default+4), %edx
	mov	$MSR_MTRR_DEFAULT, %ecx
	wrmsr

	/* Set fixed MTRRs */
	mov	$SYM_OFFSET(fixed_mtrr_regs), %edi /* 32-bit MSR to use */
	mov	$SYM_OFFSET(new_mtrr_fixed), %esi  /* 64-bit value to write */
1:
	mov	%cs:(%edi), %ecx /* MSR register */
	add	$4, %edi

	mov	%cs:(%esi), %eax /* value[0] */
	add	$4, %esi
	mov	%cs:(%esi), %edx /* value[1] */
	add	$4, %esi

	wrmsr
	cmp	$0, %edi
	jna	1b

	/* Set variable MTRRs */
	mov	$MSR_MTRR_PHYS_BASE0, %ecx
	mov	$SYM_OFFSET(new_mtrr_var_base), %esi
	mov	$SYM_OFFSET(new_mtrr_var_mask), %edi
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
	mov	%cr0, %eax
	and	$~0x40000000, %eax
	mov	%eax, %cr0

	/* Copy old TOP_MEM value, set global value */
	mov	$MSR_TOPMEM, %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(rem_topmem_msr)
	mov	%edx, %cs:SYM_OFFSET(rem_topmem_msr+4)
	mov	%cs:SYM_OFFSET(new_topmem_msr), %eax
	mov	%cs:SYM_OFFSET(new_topmem_msr+4), %edx
	wrmsr

	/* Set TOP_MEM2 */
	mov	%cs:SYM_OFFSET(new_topmem2_msr), %eax
	mov	%cs:SYM_OFFSET(new_topmem2_msr+4), %edx
	mov	$MSR_TOPMEM2, %ecx
	wrmsr

	/* Set SYSCFG MSR */
	mov	%cs:SYM_OFFSET(new_syscfg_msr), %eax
	mov	%cs:SYM_OFFSET(new_syscfg_msr+4), %edx
	mov	$MSR_SYSCFG, %ecx
	wrmsr

	/* Set OSVW_ID_LEN MSR */
	mov	%cs:SYM_OFFSET(new_osvw_id_len_msr), %eax
	mov	%cs:SYM_OFFSET(new_osvw_id_len_msr+4), %edx
	mov	$MSR_OSVW_ID_LEN, %ecx
	wrmsr

	/* Set OSVW_STATUS MSR */
	mov	%cs:SYM_OFFSET(new_osvw_status_msr), %eax
	mov	%cs:SYM_OFFSET(new_osvw_status_msr+4), %edx
	mov	$MSR_OSVW_STATUS, %ecx
	wrmsr

	/* Set HWCR MSR */
	mov	%cs:SYM_OFFSET(new_hwcr_msr), %eax
	mov	%cs:SYM_OFFSET(new_hwcr_msr+4), %edx
	mov	$MSR_HWCR, %ecx
	wrmsr

	/* Set Interrupt Pending and CMP-Halt Register MSR */
	mov	%cs:SYM_OFFSET(new_int_halt_msr), %eax
	mov	%cs:SYM_OFFSET(new_int_halt_msr+4), %edx
	mov	$MSR_HWCR, %ecx
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

	// AMD Fam 15h Errate #572: Access to PCI Extended Configuration Space in SMM is Blocked
	// Suggested Workaround
	// BIOS should set MSRC001_102A[27] = 1b.
	// We do this unconditionally (ie on fam10h as well).
	mov	$MSR_CU_CFG2, %ecx
	rdmsr
	or	$0x8000000, %eax
	wrmsr

	mov	$MSR_SMM_BASE, %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(rem_smm_base_msr)
	mov	%edx, %cs:SYM_OFFSET(rem_smm_base_msr+4)
        
	mov	$SYM_OFFSET(cpu_done2), %si
	call	print_strz

	STATUS(0)
1:
	cli
	hlt
	jmp	1b

init_probefilter_early_f10:
	/* Disable cacheing */
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0
	wbinvd

	/* Enable probe filter support */
	mov $MSR_CU_CFG2, %ecx
	rdmsr
	or	$(1 << (42 - 32)), %edx
	wrmsr

	STATUS(0)
1:
	cli
	hlt
	jmp	1b

init_probefilter_early_f15:
	/* Ensure CD bit is shared amongst cores */
	mov $MSR_CU_CFG3, %ecx
	rdmsr
	or	$(1 << (49 - 32)), %edx
	wrmsr

	/* Disable cacheing */
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0
	wbinvd

	/* Enable probe filter support */
	mov $MSR_CU_CFG2, %ecx
	rdmsr
	or	$(1 << (42 - 32)), %edx
	wrmsr

	STATUS(0)
1:
	cli
	hlt
	jmp	1b

init_enable_cache:
	/* Reenable caching */
	mov	%cr0, %eax
	and	$~0x40000000, %eax
	mov	%eax, %cr0

	STATUS(0)
1:
	cli
	hlt
	jmp	1b

	.align 16
EXPORT(cpu_status)
	.long 0
EXPORT(cpu_apic_renumber)
	.byte 0
EXPORT(cpu_apic_hi)
	.byte 0
EXPORT(new_mcfg_msr)
	.long 0, 0
EXPORT(new_topmem_msr)
	.long 0, 0
EXPORT(new_topmem2_msr)
	.long 0, 0
EXPORT(new_mtrr_default)
	.long 0, 0
EXPORT(fixed_mtrr_regs)
	.long MSR_MTRR_FIX64K_00000
	.long MSR_MTRR_FIX16K_80000
	.long MSR_MTRR_FIX16K_A0000
	.long MSR_MTRR_FIX4K_C0000
	.long MSR_MTRR_FIX4K_C8000
	.long MSR_MTRR_FIX4K_D0000
	.long MSR_MTRR_FIX4K_D8000
	.long MSR_MTRR_FIX4K_E0000
	.long MSR_MTRR_FIX4K_E8000
	.long MSR_MTRR_FIX4K_F0000
	.long MSR_MTRR_FIX4K_F8000
	.long 0
EXPORT(new_mtrr_fixed)
	.skip 11*8, 0
EXPORT(new_mtrr_var_base)
	.skip 8*8, 0
EXPORT(new_mtrr_var_mask)
	.skip 8*8, 0
EXPORT(new_syscfg_msr)
	.long 0, 0
EXPORT(rem_topmem_msr)
	.long 0, 0
EXPORT(rem_smm_base_msr)
	.long 0, 0
EXPORT(new_osvw_id_len_msr)
	.long 0, 0
EXPORT(new_osvw_status_msr)
	.long 0, 0
EXPORT(new_hwcr_msr);
	.long 0, 0
EXPORT(new_int_halt_msr);
	.long 0, 0

cpu_mtrr:
	.asciz	"\r\nMTRR "
cpu_cr:
	.asciz	"\r\nCRx "
cpu_done1:
	.asciz	"\r\nCPU "
cpu_done2:
	.asciz  " done setting MTRRs.\r\n"

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
