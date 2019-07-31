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

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

int shadow_count = 0; /* for debugging/verification purposes */
#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_init");*/
	// initialize shadow_allocator object
    
	shadow_allocator= slab_allocator_create("saObject",sizeof(mmobj_t));
	KASSERT(shadow_allocator);
	dbg(DBG_PRINT,"(GRADING3A)\n");
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros or functions which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_create");*/
	// use the shadow_allocator to allocate the mmobj
    
	mmobj_t *mmobject= slab_obj_alloc(shadow_allocator);
    /*if(mmobject == NULL) {
    	dbg(DBG_PRINT,"GRADING 2asd\n");
        return NULL;
    }*/
	// initialize mmobj
    mmobj_init(mmobject, &shadow_mmobj_ops);
    //mmobject->mmo_un.mmo_bottom_obj=mmobj_bottom_obj(mmobject);
    // initialize refcount
    mmobject->mmo_refcount=1;
    dbg(DBG_PRINT,"(GRADING3D 2)\n");
    return mmobject;

    //return NULL;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_ref");*/
	KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
    o->mmo_refcount++;
    dbg(DBG_PRINT,"(GRADING3D 2)\n");
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_put");*/
	KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
    
	if(o->mmo_refcount == o->mmo_nrespages+1){
            pframe_t *pf;
            list_iterate_begin(&o->mmo_respages, pf, pframe_t, pf_olink){
                pframe_unpin(pf);
				if (pframe_is_dirty(pf)) {
						pframe_clean(pf);
						dbg(DBG_PRINT,"(GRADING3D 1)\n");
				}
                pframe_free(pf);
                dbg(DBG_PRINT,"(GRADING3D 1)\n");
            } list_iterate_end();
            o->mmo_shadowed->mmo_ops->put(o->mmo_shadowed);
            o->mmo_un.mmo_bottom_obj->mmo_ops->put(o->mmo_un.mmo_bottom_obj);
            dbg(DBG_PRINT,"(GRADING3D 1)\n");
        }
        --o->mmo_refcount;
        if(o->mmo_refcount==0){
            slab_obj_free(shadow_allocator, o);
            dbg(DBG_PRINT,"(GRADING3D 1)\n");
        }
}

/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. It is important to
 * use iteration rather than recursion here as a recursive implementation
 * can overflow the kernel stack when looking down a long shadow chain */
static int
shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        
    if (0 == forwrite) {
        mmobj_t *bottom_obj = mmobj_bottom_obj(o);
        for (;o != bottom_obj;o = o->mmo_shadowed) {
            *pf = pframe_get_resident(o, pagenum);
            if (*pf) {
        		KASSERT(NULL != (*pf));
        		KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
        		dbg(DBG_PRINT,"(GRADING3B 1)\n");
                return 0;
            }
            dbg(DBG_PRINT,"(GRADING3B 1)\n");
        }
		pframe_lookup(o, pagenum, 0, pf);
		if(*pf==NULL){
			dbg(DBG_PRINT,"(GRADING3D 2)\n");
			return -1;
		}
		dbg(DBG_PRINT,"(GRADING3B 1)\n");
    } else {
		int ret = pframe_get(o, pagenum, pf);
		if(ret<0){
				dbg(DBG_PRINT,"(GRADING3D 2)\n");
	        	return ret;
		}
		dbg(DBG_PRINT,"(GRADING3B 1)\n");
    }
	KASSERT(NULL != (*pf));
	KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
	dbg(DBG_PRINT,"(GRADING3B 1)\n");
    return 0;
        
}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain).
 * It is important to use iteration rather than recursion here as a
 * recursive implementation can overflow the kernel stack when
 * looking down a long shadow chain */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
    mmobj_t *bottom_obj = mmobj_bottom_obj(o);
	KASSERT(pframe_is_busy(pf));
	KASSERT(!pframe_is_pinned(pf));
	pframe_t *pft = NULL;
    
    for (o = o->mmo_shadowed;o != bottom_obj;o = o->mmo_shadowed) {
        pft = pframe_get_resident(o, pf->pf_pagenum);
        if (pft) {
            
            memcpy(pf->pf_addr, pft->pf_addr, PAGE_SIZE);
            pframe_pin(pf);
            dbg(DBG_PRINT,"(GRADING3D 2)\n");
            return 0;
        }
        dbg(DBG_PRINT,"(GRADING3D 2)\n");
    }

    pframe_pin(pf);

    
    int err;
    if ((err = pframe_lookup(o, pf->pf_pagenum, 1, &pft)) < 0) {
    	dbg(DBG_PRINT,"(GRADING3D 2)\n");
        return err;
    }
    
    memcpy(pf->pf_addr, pft->pf_addr, PAGE_SIZE);
    dbg(DBG_PRINT,"(GRADING3D 2)\n");
    return 0;
        
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
        //NOT_YET_IMPLEMENTED("VM: shadow_dirtypage");
	dbg(DBG_PRINT,"(GRADING3B 1)\n");
    return 0;
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
        //NOT_YET_IMPLEMENTED("VM: shadow_cleanpage");
	dbg(DBG_PRINT,"(GRADING3D 1)\n");
    return 0;
}

