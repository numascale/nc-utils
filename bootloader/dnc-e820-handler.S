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
	add	$SYM_OFFSET(stack_start) / 16, %ax
	mov	%ax, %ss
	movl	$(stack_end - stack_start), %esp

	movl	%cs:SYM_OFFSET(cpu_status), %edx
	cmpl	$VECTOR_TRAMPOLINE, %edx
	je	init_trampoline
	cmpl	$VECTOR_PROBEFILTER_EARLY_f10, %edx
	je	init_probefilter_early_f10
	cmpl	$VECTOR_PROBEFILTER_EARLY_f15, %edx
	je	init_probefilter_early_f15
	cmpl	$VECTOR_ENABLE_CACHE, %edx
	je	init_enable_cache
	cmpl	$VECTOR_DISABLE_CACHE, %edx
	je	init_disable_cache
#ifdef LEGACY
	cmpl	$VECTOR_READBACK_MSR, %edx
	je	init_readback_msr
	cmpl	$VECTOR_READBACK_APIC, %edx
	je	init_readback_apic
#endif
1:
	cli
	hlt
	jmp	1b

init_trampoline:
	/* Disable cache */
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0
	wbinvd

	/* Flush TLB */
	mov %eax, %cr3
	mov %cr3, %eax

	STATUS(4)

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
	mov	%cs:SYM_OFFSET(new_mtrr_default + 4), %edx
	mov	$MSR_MTRR_DEFAULT, %ecx
	wrmsr

	/* Set fixed MTRRs */
	mov	$SYM_OFFSET(fixed_mtrr_regs), %edi /* 32-bit MSR to use */
	mov	$SYM_OFFSET(new_mtrr_fixed), %esi  /* 64-bit value to write */
1:
	mov	%cs:(%edi), %ecx /* MSR number */
	add	$4, %edi

	cmp $0xffffffff, %ecx
	je break

	mov	%cs:(%esi), %eax /* value[0] */
	add	$4, %esi
	mov	%cs:(%esi), %edx /* value[1] */
	add	$4, %esi

	wrmsr
	jmp	1b

break:
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
	mov	%cs:SYM_OFFSET(new_topmem_msr), %eax
	mov	%cs:SYM_OFFSET(new_topmem_msr + 4), %edx
	wrmsr

	/* Set TOP_MEM2 */
	mov	%cs:SYM_OFFSET(new_topmem2_msr), %eax
	mov	%cs:SYM_OFFSET(new_topmem2_msr + 4), %edx
	mov	$MSR_TOPMEM2, %ecx
	wrmsr

	/* Set SYSCFG MSR */
	mov	%cs:SYM_OFFSET(new_syscfg_msr), %eax
	mov	%cs:SYM_OFFSET(new_syscfg_msr + 4), %edx
	mov	$MSR_SYSCFG, %ecx
	wrmsr

	/* Set CPU WDT MSR */
	mov	%cs:SYM_OFFSET(new_cpuwdt_msr), %eax
	mov	%cs:SYM_OFFSET(new_cpuwdt_msr + 4), %edx
	mov	$MSR_CPUWDT, %ecx
	wrmsr

	/* Ensure MCEs aren't redirected into SMIs */
	mov	$0, %eax
	mov	$0, %edx
	mov	$MSR_MCE_REDIR, %ecx
	wrmsr

	/* Set HWCR MSR */
	mov	%cs:SYM_OFFSET(new_hwcr_msr), %eax
	mov	%cs:SYM_OFFSET(new_hwcr_msr + 4), %edx
	mov	$MSR_HWCR, %ecx
	wrmsr

	/* Set MC4_MISC0 MSR */
	mov	%cs:SYM_OFFSET(new_mc4_misc0_msr), %eax
	mov	%cs:SYM_OFFSET(new_mc4_misc0_msr + 4), %edx
	mov	$MSR_MC4_MISC0, %ecx
	wrmsr

	/* Set MC4_MISC1 MSR */
	mov	%cs:SYM_OFFSET(new_mc4_misc1_msr), %eax
	mov	%cs:SYM_OFFSET(new_mc4_misc1_msr + 4), %edx
	mov	$MSR_MC4_MISC1, %ecx
	wrmsr

	/* Set MC4_MISC2 MSR */
	mov	%cs:SYM_OFFSET(new_mc4_misc2_msr), %eax
	mov	%cs:SYM_OFFSET(new_mc4_misc2_msr + 4), %edx
	mov	$MSR_MC4_MISC2, %ecx
	wrmsr

#ifdef BROKEN
	/* Set OSVW_ID_LEN MSR */
	mov	%cs:SYM_OFFSET(new_osvw_id_len_msr), %eax
	mov	%cs:SYM_OFFSET(new_osvw_id_len_msr + 4), %edx
	mov	$MSR_OSVW_ID_LEN, %ecx
	wrmsr

	/* Set OSVW_STATUS MSR */
	mov	%cs:SYM_OFFSET(new_osvw_status_msr), %eax
	mov	%cs:SYM_OFFSET(new_osvw_status_msr + 4), %edx
	mov	$MSR_OSVW_STATUS, %ecx
	wrmsr

	/* Set Interrupt Pending and CMP-Halt Register MSR */
	mov	%cs:SYM_OFFSET(new_int_halt_msr), %eax
	mov	%cs:SYM_OFFSET(new_int_halt_msr + 4), %edx
	mov	$MSR_HWCR, %ecx
	wrmsr
#endif

	/* Set LSCFG MSR */
	mov	%cs:SYM_OFFSET(new_lscfg_msr), %eax
	mov	%cs:SYM_OFFSET(new_lscfg_msr + 4), %edx
	mov	$MSR_LSCFG, %ecx
	wrmsr

	/* Set CUCFG2 MSR */
	mov	%cs:SYM_OFFSET(new_cucfg2_msr), %eax
	mov	%cs:SYM_OFFSET(new_cucfg2_msr + 4), %edx
	mov	$MSR_CU_CFG2, %ecx
	wrmsr

#ifdef FIXME /* Family 15h only */
	/* Set CUCFG3 MSR */
	mov	%cs:SYM_OFFSET(new_cucfg3_msr), %eax
	mov	%cs:SYM_OFFSET(new_cucfg3_msr + 4), %edx
	mov	$MSR_CU_CFG3, %ecx
	wrmsr
#endif

	/* Set NBCFG MSR */
	mov %cs:SYM_OFFSET(new_nbcfg_msr), %eax
	mov %cs:SYM_OFFSET(new_nbcfg_msr + 4), %edx
	mov $MSR_NB_CFG, %ecx
	wrmsr

	mov	$MSR_APIC_BAR, %ecx
	rdmsr
	push	%eax
	mov	$0x70800, %eax
	mov	$MSR_APIC_BAR, %ecx
	wrmsr
	mov	$MSR_APIC_BAR, %ecx
	mov	$0x7000, %ax
	mov	%ax, %es

	mov	%cs:SYM_OFFSET(cpu_apic_renumber), %al
	shl	$24, %eax
	mov	%eax, %es:0x20

	pop	%eax
	and	$~0x100, %eax	/* Clear BSP flag to let core accept INIT and STARTUP IPIs */
	mov	$MSR_APIC_BAR, %ecx
	wrmsr

	mov	$MSR_NODE_ID, %ecx
	rdmsr

	/* "Bios scratch" given as [11:6], so we're limited to an
	   8-bit prefix here for the time being.  Ideally we want 8 bits,
	   and since all upper bits of this MSR appear to be r/w, we
	   could just take some liberties with the register. */
	and	$~0xfc0, %eax
	xor	%ebx, %ebx
	mov	%cs:SYM_OFFSET(cpu_apic_hi), %bl
	shl	$6, %ebx
	or	%ebx, %eax
	wrmsr

	mov	$MSR_MCFG_BASE, %ecx
	mov	%cs:SYM_OFFSET(new_mcfg_msr), %eax
	mov	%cs:SYM_OFFSET(new_mcfg_msr + 4), %edx
	wrmsr

	mov	$MSR_SMM_BASE, %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(rem_smm_base_msr)
	mov	%edx, %cs:SYM_OFFSET(rem_smm_base_msr + 4)

	/* Load APIC base address into FS */
	mov	$MSR_APIC_BAR, %ecx
	rdmsr
	and $~0xfff, %eax
	mov $0, %edx
	mov $MSR_FS_BASE, %ecx
	wrmsr

	/* Read and store APIC LVTs */
	mov %fs:(0x500), %eax
	mov %eax, %cs:SYM_OFFSET(lvt)
	mov %fs:(0x510), %eax
	mov %eax, %cs:SYM_OFFSET(lvt + 4)
	mov %fs:(0x520), %eax
	mov %eax, %cs:SYM_OFFSET(lvt + 8)
	mov %fs:(0x530), %eax
	mov %eax, %cs:SYM_OFFSET(lvt + 12)

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

init_disable_cache:
	mov	%cr0, %eax
	or	$0x40000000, %eax
	mov	%eax, %cr0
	wbinvd

	/* Flush TLB */
	mov %eax, %cr3
	mov %cr3, %eax

	STATUS(0)
1:
	cli
	hlt
	jmp	1b

#ifdef LEGACY
init_readback_msr:
	mov	%cs:SYM_OFFSET(msr_readback), %ecx
	rdmsr
	mov	%eax, %cs:SYM_OFFSET(msr_readback)
	mov	%edx, %cs:SYM_OFFSET(msr_readback + 4)

	STATUS(0)
1:
	cli
	hlt
	jmp	1b
#endif

	.align 64
EXPORT(cpu_status)
	.long 0
EXPORT(new_mcfg_msr)
	.long 0, 0
EXPORT(new_topmem_msr)
	.long 0, 0
EXPORT(new_topmem2_msr)
	.long 0, 0
EXPORT(new_cpuwdt_msr)
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
	.long 0xffffffff
EXPORT(new_mtrr_fixed)
	.skip 11*8, 0
EXPORT(new_mtrr_var_base)
	.skip 8*8, 0
EXPORT(new_mtrr_var_mask)
	.skip 8*8, 0
EXPORT(new_syscfg_msr)
	.long 0, 0
EXPORT(rem_smm_base_msr)
	.long 0, 0
EXPORT(new_hwcr_msr)
	.long 0, 0
EXPORT(new_mc4_misc0_msr)
	.long 0, 0
EXPORT(new_mc4_misc1_msr)
	.long 0, 0
EXPORT(new_mc4_misc2_msr)
	.long 0, 0
#ifdef BROKEN
EXPORT(new_osvw_id_len_msr)
	.long 0, 0
EXPORT(new_osvw_status_msr)
	.long 0, 0
EXPORT(new_int_halt_msr)
	.long 0, 0
#endif
EXPORT(msr_readback)
	.long 0, 0
EXPORT(apic_offset)
	.long 0
EXPORT(apic_readback)
	.long 0
EXPORT(new_lscfg_msr)
	.long 0, 0
EXPORT(new_cucfg2_msr)
	.long 0, 0
EXPORT(new_cucfg3_msr)
	.long 0, 0
EXPORT(new_nbcfg_msr)
	.long 0, 0
EXPORT(lvt)
	.long 0, 0, 0, 0
EXPORT(cpu_apic_renumber)
	.byte 0
EXPORT(cpu_apic_hi)
	.byte 0

	.align 64
stack_start:  .skip 4096, 0
stack_end:

EXPORT(new_e820_map)
	.skip E820_MAP_LEN, 0
EXPORT(old_int15_vec)
	.long 0
EXPORT(new_e820_len)
	.word 0

	.align 64
EXPORT(new_e820_handler)
	cmp	$0xe820, %eax
	jne	1f
	cmp	$0x534d4150, %edx
	je	2f
1:	ljmp	%cs:*SYM_OFFSET(old_int15_vec)

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
	and	$~1, 4(%esp)	/* Clear carry flag */
	cmp	%cs:SYM_OFFSET(new_e820_len), %bx
	jge	2f
	iret

1:	or	$1, 4(%esp)	/* Set carry flag to signify error */
2:	xor	%ebx, %ebx
	iret
	.global asm_relocate_end
asm_relocate_end:

	/* Replacement code for brute-force SMM disable */
	.code16
	.text
	.align 4096

	.global smm_handler_start
smm_handler_start:
	rsm
	.global smm_handler_end
smm_handler_end:
