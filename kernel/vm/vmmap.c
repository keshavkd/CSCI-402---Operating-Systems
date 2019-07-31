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

#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_create");*/
        dbg(DBG_PRINT, "(GRADING3A)\n");
        vmmap_t *new_vm = slab_obj_alloc(vmmap_allocator);
        list_init(&new_vm->vmm_list);
        new_vm->vmm_proc = NULL;
        return new_vm;
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */

void
vmmap_destroy(vmmap_t *map)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_destroy");*/
        KASSERT(NULL != map);  
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");
        vmarea_t *vm_area;        
        list_iterate_begin( &map->vmm_list, vm_area, vmarea_t, vma_plink ) {
        	dbg(DBG_PRINT, "(GRADING3A)\n");
            vm_area->vma_obj->mmo_ops->put(vm_area->vma_obj);
            if (list_link_is_linked(&vm_area->vma_olink)){
            dbg(DBG_PRINT, "(GRADING3A)\n");
                list_remove(&vm_area->vma_olink);
            }
            if (list_link_is_linked(&vm_area->vma_plink)){
            dbg(DBG_PRINT, "(GRADING3A)\n");
                list_remove(&vm_area->vma_plink);
            }
            vmarea_free(vm_area);           
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3A)\n");
        slab_obj_free(vmmap_allocator, map);
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
	/*NOT_YET_IMPLEMENTED("VM: vmmap_insert");*/
	KASSERT(NULL != map && NULL != newvma);
	KASSERT(NULL == newvma->vma_vmmap);
	KASSERT(newvma->vma_start < newvma->vma_end);
	KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
	dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
	newvma->vma_vmmap = map;
    	if list_empty(&map->vmm_list) 
    	{
    		dbg(DBG_PRINT, "(GRADING3A)\n");
        	list_insert_tail(&map->vmm_list, &newvma->vma_plink);
        	return;
    	}
    	dbg(DBG_PRINT, "(GRADING3A)\n");
    	vmarea_t *vma;
    	list_link_t *list=&map->vmm_list;
    	list_iterate_begin(list, vma, vmarea_t, vma_plink) 
    	{
    		dbg(DBG_PRINT, "(GRADING3A)\n");
    		if(vma->vma_plink.l_prev!=list)
    		{
    			dbg(DBG_PRINT, "(GRADING3D 1)\n");
    			vmarea_t *prev=list_item(vma->vma_plink.l_prev, vmarea_t, vma_plink);
            		if (!(newvma->vma_end > vma->vma_start || newvma->vma_start < prev->vma_end)) 
            		{
            			dbg(DBG_PRINT, "(GRADING3D 1)\n");
                		list_insert_before(&vma->vma_plink, &newvma->vma_plink);                	
                		return;
            		}
        	}
        	else
        	{
        		dbg(DBG_PRINT, "(GRADING3A)\n");
        		if (newvma->vma_end <= vma->vma_start)
            		{
            			dbg(DBG_PRINT, "(GRADING3A)\n");
                		list_insert_head(list, &newvma->vma_plink);
                		return;
            		}
        	}
        	dbg(DBG_PRINT, "(GRADING3A)\n");
    	}list_iterate_end();
    	dbg(DBG_PRINT, "(GRADING3A)\n");
    	list_insert_tail(list, &newvma->vma_plink);
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_find_range");*/
        if (dir == VMMAP_DIR_HILO || dir != 0) {
            dbg(DBG_PRINT, "(GRADING3A)\n");
            // if (list_empty(&map->vmm_list)) {
            //     dbg(DBG_PRINT, "(Pika)\n");
            //     return ((USER_MEM_HIGH/PAGE_SIZE) - npages);
            // }

            // dbg(DBG_PRINT, "(GRADING 3A)\n");
            list_link_t *list = &map->vmm_list;
            vmarea_t *c_vma;
            list_iterate_reverse(list, c_vma, vmarea_t, vma_plink) {
                if (c_vma->vma_plink.l_next != list) {
                    dbg(DBG_PRINT, "(GRADING3D 1)\n");
                    vmarea_t *vma_next = list_item(c_vma->vma_plink.l_next, vmarea_t, vma_plink);
                    if (!(vma_next->vma_start - c_vma->vma_end < npages)) {
                        dbg(DBG_PRINT, "(GRADING3D 1)\n");
                        return (vma_next->vma_start - npages);
                    }
                } else {
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    if (!((USER_MEM_HIGH/PAGE_SIZE) - c_vma->vma_end < npages)) {
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return ((USER_MEM_HIGH/PAGE_SIZE) - npages);
                    }
                }
            } list_iterate_end();

            if (c_vma->vma_start - (USER_MEM_LOW/PAGE_SIZE) < npages) {
                dbg(DBG_PRINT, "(GRADING3D 1)\n");
                return -1;
            } else {
                dbg(DBG_PRINT, "(GRADING3D 2)\n");
                return c_vma->vma_start - npages;
            }
        }// else {
            // dbg(DBG_PRINT, "(Pika)\n");
         //    if (list_empty(&map->vmm_list)) {
         //        dbg(DBG_PRINT, "(Pika)\n");
         //        return (USER_MEM_LOW/PAGE_SIZE);
         //    }

         //    list_link_t *list = &map->vmm_list;
         //    vmarea_t *c_vma;
         //    list_iterate_begin(list, c_vma, vmarea_t, vma_plink) {
         //        if (c_vma->vma_plink.l_prev != list) {
         //            dbg(DBG_PRINT, "(Pika)\n");
         //            vmarea_t *vma_prev = list_item(c_vma->vma_plink.l_prev, vmarea_t, vma_plink);
         //            if (!(c_vma->vma_start - vma_prev->vma_end < npages)) {
         //                dbg(DBG_PRINT, "(Pika)\n");
         //                return vma_prev->vma_end;
         //            }
         //        } else {
         //            dbg(DBG_PRINT, "(Pika)\n");
         //            if (!(c_vma->vma_start < npages + (USER_MEM_LOW/PAGE_SIZE))) {
         //                dbg(DBG_PRINT, "(Pika)\n");
         //                return (USER_MEM_LOW/PAGE_SIZE);
         //            }                
         //        }
         //    } list_iterate_end();

         //    if ((USER_MEM_HIGH/PAGE_SIZE) - c_vma->vma_end < npages) {
         //        dbg(DBG_PRINT, "(Pika)\n");
         //        return -1;
         //    } else {
         //        dbg(DBG_PRINT, "(Pika)\n");
         //        return c_vma->vma_end;
         //    } 
        // }
            dbg(DBG_PRINT, "(GRADING3A)\n");
            return -1;
}/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_lookup");*/
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");
    	vmarea_t *vma;
    	// scan AS for vma whose range covers vfn
    	list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
    	{
        	// if vma whose range cover vfn
        	dbg(DBG_PRINT, "(GRADING3A)\n");
        	if ((vma->vma_start<=vfn) && (vma->vma_end>vfn))
        	{
        		dbg(DBG_PRINT, "(GRADING3A)\n");
            		return vma;
        	}
        	dbg(DBG_PRINT, "(GRADING3A)\n");
    	} list_iterate_end();
    	dbg(DBG_PRINT, "(GRADING3A)\n");
    	// if page is unmapped
    	return NULL;
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
	/*NOT_YET_IMPLEMENTED("VM: vmmap_clone");*/
	dbg(DBG_PRINT, "(GRADING3A)\n");
    	vmmap_t *newmap = vmmap_create();
    	vmarea_t *area;
    	list_iterate_begin(&map->vmm_list, area, vmarea_t, vma_plink) 
    	{
    		dbg(DBG_PRINT, "(GRADING3A)\n");
        	vmarea_t *new = vmarea_alloc();
        	new->vma_flags = area->vma_flags;
        	new->vma_start = area->vma_start;
        	new->vma_prot = area->vma_prot;
        	new->vma_off = area->vma_off;
        	new->vma_end = area->vma_end;
        	list_link_init(&new->vma_plink);
        	vmmap_insert(newmap, new);
        	list_link_init(&new->vma_olink);
        	new->vma_obj = area->vma_obj;
        	new->vma_obj->mmo_ops->ref(new->vma_obj);
    	} list_iterate_end();
    	dbg(DBG_PRINT, "(GRADING3A)\n");
    	return newmap;
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_map");*/
    KASSERT(NULL != map);
    KASSERT(0 < npages);
    KASSERT(!(~(PROT_NONE | PROT_READ | PROT_WRITE | PROT_EXEC) & prot));
    KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
    KASSERT(PAGE_ALIGNED(off));
    dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
    if (lopage ==0) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        int range=vmmap_find_range(map,npages, dir);
        if(range == -1) {
        	dbg(DBG_PRINT, "(GRADING3D 2)\n");
            return -1;
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        lopage = (uint32_t)range;
    } else {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        int anotherMapping = vmmap_is_range_empty(map, lopage, npages); 
        if (anotherMapping == 0) {
        dbg(DBG_PRINT, "(GRADING3A)\n");
            int unmap = vmmap_remove(map,lopage, npages);
            /*if(unmap<0) {
            	dbg(DBG_PRINT, "(MAP)\n");
                return -1;
            }*/
           dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
    }
    dbg(DBG_PRINT, "(GRADING3A)\n");
    mmobj_t *mmobj;
    vmarea_t *vmarea = vmarea_alloc();
    vmarea->vma_flags = flags;
    vmarea->vma_start = lopage;
    vmarea->vma_prot = prot;
    vmarea->vma_off = ADDR_TO_PN(off);
    vmarea->vma_end = lopage + npages;
    list_link_init(&vmarea->vma_plink);
    list_link_init(&vmarea->vma_olink);

    if(file == NULL) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        mmobj = anon_create();
        /*if(mmobj == NULL) {
        	dbg(DBG_PRINT, "(MAP)\n");
            return -1;
        }*/
        dbg(DBG_PRINT, "(GRADING3A)\n");
    } else {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        file->vn_ops->mmap(file,vmarea,&mmobj);
    }
    vmarea->vma_obj = mmobj;
	dbg(DBG_PRINT, "(GRADING3A)\n");
    if ((flags & MAP_PRIVATE)!=0) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        mmobj_t *mmobj_shadow = shadow_create();
	mmobj_shadow->mmo_shadowed = vmarea->vma_obj;
        mmobj_shadow->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(vmarea->vma_obj);
        list_insert_head(&vmarea->vma_obj->mmo_un.mmo_vmas, &vmarea->vma_olink);
        vmarea->vma_obj = mmobj_shadow;
        mmobj_shadow->mmo_ops->ref(mmobj_shadow);
    }
    

    dbg(DBG_PRINT, "(GRADING3A)\n");
    if (new!=NULL) {
    	dbg(DBG_PRINT, "(GRADING3A)\n");
        *new = vmarea;
    }
	dbg(DBG_PRINT, "(GRADING3A)\n");
    vmmap_insert(map, vmarea);
    return 0;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_remove");*/
        // if (list_empty(&map->vmm_list)) 
        // {
        //     dbg(DBG_PRINT, "BULBA\n");
     //        return 0;
        // }
        if (vmmap_is_range_empty(map, lopage, npages)) 
        {
            dbg(DBG_PRINT, "(GRADING3D 2)\n");
            return 0;
        }

        vmarea_t *vma;
        uint32_t hipage = lopage + npages;
        dbg(DBG_PRINT, "(GRADING3A)\n");
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
        {
            //case 1
            if (!(vma->vma_start >= lopage || vma->vma_end <= hipage)) 
            {
                    vmarea_t *new = vmarea_alloc();
                    new->vma_start = vma->vma_start;
                    new->vma_obj = vma->vma_obj;
                    new->vma_obj->mmo_ops->ref(new->vma_obj);
                    new->vma_flags = vma->vma_flags;
                    new->vma_end = lopage;
                    new->vma_prot = vma->vma_prot;
                    new->vma_off = vma->vma_off;
                    new->vma_vmmap = vma->vma_vmmap;
                    list_link_init(&new->vma_plink);
                    list_insert_before(&vma->vma_plink, &new->vma_plink);
                    list_link_init(&new->vma_olink);
                    mmobj_t *bottom = mmobj_bottom_obj(vma->vma_obj);
                    dbg(DBG_PRINT, "(GRADING3D 2)\n");
                    if (bottom != vma->vma_obj) 
                    {
                        list_insert_head(&bottom->mmo_un.mmo_vmas, &new->vma_olink);
                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                    }
                    vma->vma_off = hipage - vma->vma_start + vma->vma_off;
                    vma->vma_start = hipage;
                    dbg(DBG_PRINT, "(GRADING3D 2)\n");
            }

            //case 2
            if (vma->vma_start < lopage && vma->vma_end <= hipage && vma->vma_end>lopage) 
            {
                    vma->vma_end = lopage;
                    dbg(DBG_PRINT, "(GRADING3D 2)\n");
            }

            //case 3
            if (vma->vma_start >= lopage && vma->vma_end > hipage && vma->vma_start < hipage) 
            {
                    vma->vma_off = hipage - vma->vma_start + vma->vma_off;
                    vma->vma_start = hipage;
                    dbg(DBG_PRINT, "(GRADING3D 2)\n");
            }

            //case 4
            if (!(vma->vma_start < lopage || vma->vma_end > hipage) && vma->vma_start < hipage && vma->vma_end>lopage) 
            {
                    vma->vma_obj->mmo_ops->put(vma->vma_obj);
                    list_remove(&vma->vma_plink);
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    if (list_link_is_linked(&vma->vma_olink)) 
                    {
                        list_remove(&vma->vma_olink);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                    } 
                vmarea_free(vma);
                dbg(DBG_PRINT, "(GRADING3A)\n");
            }
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_is_range_empty");
	uint32_t endvfn=startvfn+npages;
	KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
	dbg(DBG_PRINT, "(GRADING3A 3.e)\n");
	vmarea_t *vma;
	
	list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink)
	{
		dbg(DBG_PRINT, "(GRADING3A)\n");
		if(vma->vma_start<startvfn+npages && vma->vma_end>startvfn)
		{
			dbg(DBG_PRINT, "(GRADING3A)\n");
			return 0;
		}
		dbg(DBG_PRINT, "(GRADING3A)\n");
	}list_iterate_end();
	dbg(DBG_PRINT, "(GRADING3A)\n");
	return 1;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_read");*/
        uint32_t buf_index = (uint32_t)buf;
        uint32_t addr = (uint32_t)vaddr;
        uint32_t n=0;
        dbg(DBG_PRINT, "(GRADING3A)\n");
        while (n < count) {
            vmarea_t *vmarea = vmmap_lookup(map, ADDR_TO_PN(addr));
            uint32_t page_num = ADDR_TO_PN(addr) + vmarea->vma_off - vmarea->vma_start;
            pframe_t *pframe;
            pframe_lookup(vmarea->vma_obj, page_num, 0, &pframe);
            uint32_t size = PAGE_SIZE - PAGE_OFFSET(addr);
            uint32_t bytes = size < count ? size : count;
            char *read_ptr = (char *)pframe->pf_addr + PAGE_OFFSET(addr);
            memcpy((char *)buf_index, read_ptr, bytes);
            buf_index += bytes;
            n += bytes;
            addr += bytes;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_write");*/
        dbg(DBG_PRINT, "(GRADING3A)\n");
        uint32_t buf_index = (uint32_t)buf;
        uint32_t addr = (uint32_t)vaddr;
        uint32_t n=0;
        while (n < count) {
        	dbg(DBG_PRINT, "(GRADING3A)\n");
            vmarea_t *vmarea = vmmap_lookup(map, ADDR_TO_PN(addr));
            uint32_t page_num = ADDR_TO_PN(addr) + vmarea->vma_off - vmarea->vma_start;
            pframe_t *pframe;
            pframe_lookup(vmarea->vma_obj, page_num, 1, &pframe);
            pframe_dirty(pframe);
            uint32_t size = PAGE_SIZE - PAGE_OFFSET(addr);
            uint32_t bytes = size < count ? size : count;
            char *write_ptr = (char *)pframe->pf_addr + PAGE_OFFSET(addr);
            memcpy(write_ptr, (void *)buf_index, bytes);
            buf_index += bytes;
            n += bytes;
            addr += bytes;         
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}
