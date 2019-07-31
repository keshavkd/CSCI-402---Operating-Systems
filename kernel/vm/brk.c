/******************************************************************************/
/* Important Fall 2018 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "globals.h"
#include "errno.h"
#include "util/debug.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/mman.h"

#include "vm/mmap.h"
#include "vm/vmmap.h"

#include "proc/proc.h"

/*
 * This function implements the brk(2) system call.
 *
 * This routine manages the calling process's "break" -- the ending address
 * of the process's "dynamic" region (often also referred to as the "heap").
 * The current value of a process's break is maintained in the 'p_brk' member
 * of the proc_t structure that represents the process in question.
 *
 * The 'p_brk' and 'p_start_brk' members of a proc_t struct are initialized
 * by the loader. 'p_start_brk' is subsequently never modified; it always
 * holds the initial value of the break. Note that the starting break is
 * not necessarily page aligned!
 *
 * 'p_start_brk' is the lower limit of 'p_brk' (that is, setting the break
 * to any value less than 'p_start_brk' should be disallowed).
 *
 * The upper limit of 'p_brk' is defined by the minimum of (1) the
 * starting address of the next occuring mapping or (2) USER_MEM_HIGH.
 * That is, growth of the process break is limited only in that it cannot
 * overlap with/expand into an existing mapping or beyond the region of
 * the address space allocated for use by userland. (note the presence of
 * the 'vmmap_is_range_empty' function).
 *
 * The dynamic region should always be represented by at most ONE vmarea.
 * Note that vmareas only have page granularity, you will need to take this
 * into account when deciding how to set the mappings if p_brk or p_start_brk
 * is not page aligned.
 *
 * You are guaranteed that the process data/bss region is non-empty.
 * That is, if the starting brk is not page-aligned, its page has
 * read/write permissions.
 *
 * If addr is NULL, you should "return" the current break. We use this to
 * implement sbrk(0) without writing a separate syscall. Look in
 * user/libc/syscall.c if you're curious.
 *
 * You should support combined use of brk and mmap in the same process.
 *
 * Note that this function "returns" the new break through the "ret" argument.
 * Return 0 on success, -errno on failure.
 */
int
do_brk(void *addr, void **ret)
{
	dbg(DBG_PRINT, "(GRADING3A)\n");
    if(addr == NULL) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        *ret = curproc->p_brk;
        return 0;
    } else if (addr < curproc->p_start_brk) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        return -ENOMEM;
    }
	dbg(DBG_PRINT, "(GRADING3A)\n");	
    uintptr_t init_addr;
    if ((uintptr_t) addr < USER_MEM_HIGH)
    {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        init_addr = (uintptr_t) addr; 
    }
    else
    {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        init_addr = USER_MEM_HIGH;
    }
    dbg(DBG_PRINT, "(GRADING3A)\n");
    vmarea_t *area = vmmap_lookup(curproc->p_vmmap, ADDR_TO_PN((uint32_t) curproc->p_start_brk));
    int npages = (ADDR_TO_PN(init_addr - 1) - area->vma_end) + 1;

    int err = -1;
    init_addr = (uintptr_t)addr - 1;

    if((ADDR_TO_PN(init_addr) >= area->vma_start) && (ADDR_TO_PN(init_addr) < area->vma_end)) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        err *= err;
    } else if(ADDR_TO_PN(init_addr) >= area->vma_end) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        if (vmmap_is_range_empty(curproc->p_vmmap, area->vma_end, npages)) {
       	    dbg(DBG_PRINT, "(GRADING3A)\n");
            err *= err;
        }
    }
    dbg(DBG_PRINT, "(GRADING3A)\n");

    if (err != 1)
    {
        return -ENOMEM;
    }

    curproc->p_brk = addr;
    *ret = curproc->p_brk;
    area->vma_end = ADDR_TO_PN(init_addr) + 1;
    dbg(DBG_PRINT, "(GRADING3A)\n");
    return 0;
}
