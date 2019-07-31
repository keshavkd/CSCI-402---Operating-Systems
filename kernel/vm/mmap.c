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
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
 

int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
	dbg(DBG_PRINT, "(GRADING3A)\n");
	int pagelen,retur;
	vmarea_t *vm;
	if(len%PAGE_SIZE==0)
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		pagelen=len/PAGE_SIZE;
	}
	else
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		pagelen=(len/PAGE_SIZE)+1;
	}
	if (len<=0)
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}
	if(!(PAGE_ALIGNED(addr)))
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}
	if(!(PAGE_ALIGNED(off)))
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}

	if(!((MAP_SHARED & flags) || (MAP_PRIVATE & flags)))
	{
		dbg(DBG_PRINT, "GRADING3D 1\n");
		return -1;
	}
	/*if(((MAP_SHARED & flags) && (MAP_PRIVATE & flags)))
	{
		dbg(DBG_PRINT, "(TEST)\n");
		return -1;
	}*/
	if((addr==0)&&(flags&MAP_PRIVATE) && (flags&MAP_FIXED) && (prot&PROT_READ) && (prot&PROT_WRITE)==0)
	{
		dbg(DBG_PRINT, "GRADING3D 1\n");
		return -1;
	}
	
	if ((flags&MAP_ANON)==0)
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		if (fd<0||fd>=NFILES||curproc->p_files[fd]==NULL)
		{
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			return -EBADF;
		}
		/*if((curproc->p_files[fd]->f_mode&FMODE_READ)==0)
		{
			dbg(DBG_PRINT, "(TEST)\n");
			return -EACCES;
		}*/
		/*if((MAP_PRIVATE & flags) && (curproc->p_files[fd]->f_mode&FMODE_WRITE)==0)
		{
			dbg(DBG_PRINT, "(TEST)\n");
			return -1;
		}*/
		if (((flags&MAP_SHARED)!=0)&&((prot&PROT_WRITE)!=0)&&((curproc->p_files[fd]->f_mode&FMODE_READ)==0||(curproc->p_files[fd]->f_mode&FMODE_WRITE)==0))
		{
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			return -EACCES;
		}
		/*if((prot&PROT_WRITE)!=0 && curproc->p_files[fd]->f_mode&FMODE_APPEND)
		{
			dbg(DBG_PRINT, "(TEST)\n");
			return -EACCES;
		}*/
	}
	dbg(DBG_PRINT, "(GRADING3A)\n");
    	retur=vmmap_map(curproc->p_vmmap,curproc->p_files[fd]->f_vnode,ADDR_TO_PN(addr),pagelen,prot,flags,off,VMMAP_DIR_HILO,&vm);
    	
    	if(retur<0)
    	{
    		dbg(DBG_PRINT, "(GRADING3D 1)\n");
    		return -1;
    	}
	if(addr==NULL)
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		*ret=(void *)(uintptr_t)PN_TO_ADDR(vm->vma_start);
		tlb_flush_range((uintptr_t)PN_TO_ADDR(vm->vma_start),pagelen);
		pt_unmap_range(curproc->p_pagedir,(uintptr_t)PN_TO_ADDR(vm->vma_start),(uintptr_t)PN_TO_ADDR(vm->vma_start)+(uintptr_t)PN_TO_ADDR(pagelen));
	}
	else
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		*ret=addr;
		tlb_flush_range((uintptr_t)addr,pagelen);
		pt_unmap_range(curproc->p_pagedir,(uintptr_t)addr,(uintptr_t)addr+(uintptr_t)(PN_TO_ADDR(pagelen)));
		
	}
	KASSERT(NULL != curproc->p_pagedir);
	dbg(DBG_PRINT, "(GRADING3A 2.a)\n");
	return 0;
}





/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
	/*NOT_YET_IMPLEMENTED("VM: do_munmap");*/
	dbg(DBG_PRINT, "(GRADING3A)\n");
	int retur,pagelen;
	if(len%PAGE_SIZE==0)
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		pagelen=len/PAGE_SIZE;
	}
	else
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		pagelen=(len/PAGE_SIZE)+1;
	}		
	if((uintptr_t)addr<USER_MEM_LOW || (uintptr_t)addr+len >USER_MEM_HIGH || len==0 || len==(size_t)-1)
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}
	retur=vmmap_remove(curproc->p_vmmap,ADDR_TO_PN((uintptr_t)addr),pagelen);
	tlb_flush_range((uintptr_t)addr,pagelen);
	pt_unmap_range(curproc->p_pagedir,(uintptr_t)addr,(uintptr_t)PN_TO_ADDR(ADDR_TO_PN((uintptr_t)addr)+pagelen));
	dbg(DBG_PRINT, "(GRADING3A)\n");
	return retur;
}

