/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <1999>  Greg Haerr <greg@censoft.com>
 * Copyright (C) <1991>  David I. Bell
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

#include <stdlib.h>
#include <string.h>

#include <kwcommon/kwcommon.h>
#include <kwlog/kwlog.h>
#include <kwgssrv/kwgssrv.h>

/* ==============
 *     macros
 * ============== */

/* Check to see if there is enough memory in the present region */
#define MEMCHECK(reg, rect) {                                                 \
          if ((reg)->rects_use >= ((reg)->rects_have - 1)) {                  \
            ((reg)->rects) = realloc(((reg)->rects),                          \
                             (2 * (sizeof(KWRect_t)) * ((reg)->rects_have))); \
            if (((reg)->rects) == NULL) {                                     \
              return;                                                         \
            }                                                                 \
            (reg)->rects_have *= 2;                                           \
            (rect) = &((reg)->rects)[(reg)->rects_use];                       \
          }                                                                   \
        }

#define REGION_NOT_EMPTY(pReg) pReg->rects_use

#define EMPTY_REGION(pReg) {            \
          (pReg)->rects_use = 0;        \
          (pReg)->extents.left = 0;     \
          (pReg)->extents.top = 0;      \
          (pReg)->extents.right = 0;    \
          (pReg)->extents.bottom = 0;   \
        }

/*  1 if two RECTs overlap.
 *  0 if two RECTs do not overlap.
 */
#define CHECK_EXTENT_OVERLAP(r1, r2) \
	((r1)->right > (r2)->left && \
	 (r1)->left < (r2)->right && \
	 (r1)->bottom > (r2)->top && \
	 (r1)->top < (r2)->bottom)

/* ========================
 *     static functions
 * ======================== */

/* Attempt to merge the rects in the current band with those in the previous one.
 * Used only in the region_operation() function.
 *
 * @Return:
 *        The new index for the previous band.
 * @Side effects:
 *        If union takes place:
 *           - rectangles in the previous band will have their bottom fields altered.
 *           - pReg->rects_use will be decresed.
 */
static int
region_union_similar_band(KWRegion_t *pReg, /* Region to coalesce */
			  int prevStart,  /* Index of start of previous band */
			  int curStart)   /* Index of start of current band */
{
  KWRect_t *prev_band_rect_ptr = NULL; /* Current rect in previous band */
  KWRect_t *cur_band_rect_ptr = NULL;  /* Current rect in current band */
  KWRect_t *region_end = NULL;         /* End of region */
  int prev_band_rect_num;            /* Number of rectangles in previous band */
  int cur_band_rect_num;             /* Number of rectangles in current band */
  int bandtop;                       /* top coordinate for current band */
  
  region_end = pReg->rects + pReg->rects_use;
  
  prev_band_rect_ptr = &(pReg->rects[prevStart]);
  prev_band_rect_num = curStart - prevStart;
  
  /* Figure out how many rectangles are in the current band.
   * Have to do this bacause multiple bands could have been added
   * in region_operation() at the end.
   */
  cur_band_rect_ptr = &(pReg->rects[curStart]);
  bandtop = cur_band_rect_ptr->top;
  for (cur_band_rect_num = 0;
       (cur_band_rect_ptr != region_end) && (cur_band_rect_ptr->top == bandtop);
       cur_band_rect_num++) {
    cur_band_rect_ptr++;
  }
  
  if (cur_band_rect_ptr != region_end) {
    /* If more than one band was added, we have to find the start
     * of the last band added so the next coalescing job can start
     * at the right place... (given when multiple bands are added,
     * this may be useless -- see above).
     */
    region_end--; /* Because the reg->rects array is counted from 0,
		   * the actual end of the rectangles array have to be at
		   * region_end--;
		   */
    while (region_end[-1].top == region_end->top) {
      region_end--;
    }
    
    curStart = region_end - pReg->rects;
    region_end = pReg->rects + pReg->rects_use; /* Restore the region_end's meaningful value. */
  }
  
  if ((cur_band_rect_num == prev_band_rect_num) && (cur_band_rect_num != 0)) {
    cur_band_rect_ptr -= cur_band_rect_num; /* calculate the cur_band_rect_ptr's meaningful value. */
    
    /* The bands will only be coalesced if the bottom of the previous
     * matches the top scanline of the current.
     */
    if (prev_band_rect_ptr->bottom == cur_band_rect_ptr->top) {
      /* Make sure the bands have rects in the same places.
       * This assumes that rects have been added in such a way that
       * they cover the most area possible.
       * i.e. two rects in a band must have some horizontal space
       * between them.
       * If two rects in a band does not have horizontal space between them,
       * they will be union to 1 rect.
       */
      do {
	if ((prev_band_rect_ptr->left != cur_band_rect_ptr->left) ||
	    (prev_band_rect_ptr->right != cur_band_rect_ptr->right)) {
	  /* The bands don't line up so they can't be coalesced. */
	  return (curStart);
	}
	
	prev_band_rect_ptr++;
	cur_band_rect_ptr++;
	prev_band_rect_num -= 1;
      } while (prev_band_rect_num != 0);
      
      /* At this time, we sure this 2 bands can be merged into 1 band,
       * so let's do it.
       */
      pReg->rects_use -= cur_band_rect_num;
      cur_band_rect_ptr -= cur_band_rect_num; /* Restore the 2 variables' meaningful value. */
      prev_band_rect_ptr -= cur_band_rect_num;
      
      /* The bands may be merged, so set the bottom of each rect
       * in the previous band to that of the corresponding rect
       * in the current band.
       */
      do {
	prev_band_rect_ptr->bottom = cur_band_rect_ptr->bottom;
	prev_band_rect_ptr++;
	cur_band_rect_ptr++;
	cur_band_rect_num -= 1;
      } while (cur_band_rect_num != 0);
      
      /* If only one band was added to the region,
       * we have to reset the curStart to the start of the previous band.
       *
       * If more than one band was added to the region,
       * copy the other bands down.
       * The assumption here is that the other bands came from
       * the same region as the current one and no further
       * coalescing can be done on them since it's all been done
       * already... curStart is already in the right place.
       */
      if (cur_band_rect_ptr == region_end) {
	curStart = prevStart;
      } else {
	do {
	  *prev_band_rect_ptr++ = *cur_band_rect_ptr++;
	} while (cur_band_rect_ptr != region_end);
      }
    }
  }
  
  return (curStart);
}

/* Apply an operation to 2 regions.
 *
 * @Return:
 *      None
 * @Side Effects:
 *      The new region is overwritten.
 */
static void
region_operation(KWRegion_t *newReg, /* Place to store result */
		 KWRegion_t *reg1,   /* 1st region in operation */
		 KWRegion_t *reg2,   /* 2nd region in operation */
		 void (*overlapFunc)(),     /* Function to call for over-lapping bands */
		 void (*nonOverlap1Func)(), /* Function to call for non-overlapping bands in region 1 */
		 void (*nonOverlap2Func)()) /* Function to call for non-overlapping bands in region 2 */
{
  KWRect_t *region1_rect_ptr;   /* Pointer into 1st region */
  KWRect_t *region2_rect_ptr;   /* Pointer into 2nd region */
  KWRect_t *region1_end;        /* End of 1st region */
  KWRect_t *region2_end;        /* End of 2nd region */
  KWRect_t *region1_band_end;   /* End of current band in region 1 */
  KWRect_t *region2_band_end;   /* End of current band in region 2 */
  
  int overlap_top;            /* Top of intersection */
  int overlap_bot;            /* Bottom of intersection */
  int nonoverlap_top;         /* Top of non-overlapping band */
  int nonoverlap_bot;         /* Bottom of non-overlapping band */
  
  KWRect_t *oldRects;           /* Old rects for newReg */
  
  int prevBand;               /* Index of start of previous band in newReg */
  int curBand;                /* Index of start of current band in newReg */
  
  /* Initialization */
  region1_rect_ptr = reg1->rects;
  region2_rect_ptr = reg2->rects;
  region1_end = region1_rect_ptr + reg1->rects_use;
  region2_end = region2_rect_ptr + reg2->rects_use;
  
  /* newReg may be one of the src regions so we can't empty it now.
   * We need to keep a record of its rects pointer so that we can free them later.
   * Preserve its extens and simply set its rects_use to zero.
   */
  oldRects = newReg->rects;
  newReg->rects_use = 0;
  
  /* Allocate a reasonable number of rectangles for the new region.
   * The idea is to allocate enough so the individual functions don't need to
   * reallocate and copy the array, which is time consuming,
   * yet we don't have to worry about using too much memory.
   */
  newReg->rects_have = MAX(reg1->rects_use, reg2->rects_use) * 2;
  
  if ((newReg->rects = malloc(sizeof(KWRect_t) * newReg->rects_have)) == NULL) {
    LOG(("%s\n", "<Server Error> allocate newReq->rects failed."));
    newReg->rects_have = 0;
    return;
  }
  
  /* Initialize overlap_bot and overlap_top.
   * In the upcoming loop, overlap_bot and overlap_top serve different functions
   * depending on whether the band being handled is an overlapping or non-overlapping
   * band.
   *
   * In the case of a non-overlapping band (only one of the regions
   * has points in the band), overlap_bot is the bottom of the most recent
   * intersection and thus clips the top of the rectangles in that band.
   * overlap_top is the top of the next intersection between the two regions and
   * serves to clip the bottom of the rectangles in the current band.
   *
   * For an overlapping band (where the two regions intersect),
   * overlap_top clips the top of the rectangles of both regions and
   * overlap_bot clips the bottoms.
   */
  if (reg1->extents.top < reg2->extents.top) {
    overlap_bot = reg1->extents.top;
  } else {
    overlap_bot = reg2->extents.top;
  }
  
  /* prevBand serves to mark the start of the previous band so rectangles
   * can be coalesced into larger rectangles.
   * i.e. the above region_union_similar_band() function.
   *
   * In the beginning, there is no previous band, so prevBand == curBand
   * (curBand is set later on, of course, but the first band will always
   * start at index 0).
   */
  prevBand = 0;
  
  do {
    /* curBand is the place we add the new band to the newReg */
    curBand = newReg->rects_use;
    
    /* This algorithm proceeds one source-band (as opposed to a
     * destination band) at a time.
     *
     * region1_band_end and region2_band_end serve to mark the rectangle
     * after the last one in the current band for their respective regions.
     */
    /* figure out the current band of the region 1 */
    region1_band_end = region1_rect_ptr;
    while ((region1_band_end != region1_end) && (region1_band_end->top == region1_rect_ptr->top)) {
      region1_band_end++;
    }
    
    /* figure out the current band of the region 2 */
    region2_band_end = region2_rect_ptr;
    while ((region2_band_end != region2_end) && (region2_band_end->top == region2_rect_ptr->top)) {
      region2_band_end++;
    }
    
    /* First handle the band that doesn't intersect, if any.
     *
     * Note that attention is restricted to one band in the
     * non-intersecting region at once, so if a region has n
     * bands between the current position and the next place it overlaps
     * the other, this entire loop will be passed through n times.
     */
    if (region1_rect_ptr->top < region2_rect_ptr->top) {
      /* there exists at least one one-intersecting region between region1 and region 2 */
      nonoverlap_top = MAX(region1_rect_ptr->top, overlap_bot);
      nonoverlap_bot = MIN(region1_rect_ptr->bottom, region2_rect_ptr->top);
      
      if ((nonoverlap_top != nonoverlap_bot) && (nonOverlap1Func != (void (*)())NULL)) {
	(*nonOverlap1Func)(newReg, region1_rect_ptr, region1_band_end, nonoverlap_top, nonoverlap_bot);
      }
      
      overlap_top = region2_rect_ptr->top;
    } else if (region2_rect_ptr->top < region1_rect_ptr->top) {
      nonoverlap_top = MAX(region2_rect_ptr->top, overlap_bot);
      nonoverlap_bot = MIN(region2_rect_ptr->bottom, region1_rect_ptr->top);
      
      if ((nonoverlap_top != nonoverlap_bot) && (nonOverlap2Func != (void (*)())NULL)) {
	(*nonOverlap2Func)(newReg, region2_rect_ptr, region2_band_end, nonoverlap_top, nonoverlap_bot);
      }
      
      overlap_top = region1_rect_ptr->top;
    } else {
      overlap_top = region1_rect_ptr->top;
    }
    
    /* If any rectangles got added to the region, try to coalesce them
     * with rectangles from the previous band.
     */
    if (newReg->rects_use != curBand) {
      prevBand = region_union_similar_band(newReg, prevBand, curBand);
    }
    
    /* Now see if we've hit an intersecting band.
     * The two bands only intersect if overlap_bot > overlap_top.
     */
    overlap_bot = MIN(region1_rect_ptr->bottom, region2_rect_ptr->bottom);
    curBand = newReg->rects_use;
    if (overlap_bot > overlap_top) {
      (*overlapFunc)(newReg, region1_rect_ptr, region1_band_end,
		     region2_rect_ptr, region2_band_end,
		     overlap_top, overlap_bot);
      
      if (newReg->rects_use != curBand) {
	prevBand = region_union_similar_band(newReg, prevBand, curBand);
      }
    }
    
    /* If we have finished working on one band (bottom == overlap_bot),
     * we will skip forward to the next band in that region.
     */
    if (region1_rect_ptr->bottom == overlap_bot) {
      region1_rect_ptr = region1_band_end;
    }
    if (region2_rect_ptr->bottom == overlap_bot) {
      region2_rect_ptr = region2_band_end;
    }
  } while ((region1_rect_ptr != region1_end) && (region2_rect_ptr != region2_end));
  
  /* Deal with whichever region still has rectangles left */
  curBand = newReg->rects_use;
  if ((region1_rect_ptr != region1_end) && (nonOverlap1Func != (void (*)())NULL)) {
    /* region 1 still has rectangles left */
    do {
      region1_band_end = region1_rect_ptr;
      while ((region1_band_end < region1_end) && (region1_band_end->top == region1_rect_ptr->top)) {
	region1_band_end++;
      }
      
      (*nonOverlap1Func)(newReg, region1_rect_ptr, region1_band_end,
			 MAX(region1_rect_ptr->top, overlap_bot), region1_rect_ptr->bottom);
      region1_rect_ptr = region1_band_end;
    } while (region1_rect_ptr != region1_end);
  } else if ((region2_rect_ptr != region2_end) && (nonOverlap2Func != (void (*)())NULL)) {
    /* region 2 still has rectangles left */
    do {
      region2_band_end = region2_rect_ptr;
      while ((region2_band_end < region2_end) && (region2_band_end->top == region2_rect_ptr->top)) {
	region2_band_end++;
      }
      
      (*nonOverlap2Func)(newReg, region2_rect_ptr, region2_band_end,
			 MAX(region2_rect_ptr->top, overlap_bot), region2_rect_ptr->bottom);
      region2_rect_ptr = region2_band_end;
    } while (region2_rect_ptr != region2_end);
  }
  
  if (newReg->rects_use != curBand) {
    (void)region_union_similar_band(newReg, prevBand, curBand);
  }
  
  /* A bit of cleanup.
   * To keep regions from growing without bound,
   * we shrink the array of rectangles to match the new number of
   * rectangles in the region. This never goes to 0, however...
   *
   * Only do this stuff if the number of rectangles allocated is more than
   * twice the number of rectangles in the region (a simple optimization...).
   */
  if (newReg->rects_use < (newReg->rects_have >> 1)) {
    if (REGION_NOT_EMPTY(newReg)) {
      KWRect_t  *prev_rects = newReg->rects;
      int      prev_num   = newReg->rects_have;
      
      newReg->rects_have = newReg->rects_use;
      newReg->rects = realloc(newReg->rects, sizeof(KWRect_t) * newReg->rects_have);
      
      if (newReg->rects == NULL) {
	newReg->rects = prev_rects;
	newReg->rects_have = prev_num;
      }
    } else {
      /* the region is empty */
      newReg->rects_have = 1;
      free(newReg->rects);
      newReg->rects = malloc(sizeof(KWRect_t));
      if (newReg->rects == NULL) {
	LOG(("%s\n", "<Server Error> allocate one rectangle failed."));
      }
    }
  }
  free(oldRects);
}

/* re-calculate the extents of a region */
static void
set_region_extents(KWRegion_t *pReg)
{
  KWRect_t *pRect = NULL;
  KWRect_t *pRectEnd = NULL;
  KWRect_t *pExtents = NULL;
  
  if (pReg->rects_use == 0) {
    pReg->extents.left   = 0;
    pReg->extents.top    = 0;
    pReg->extents.right  = 0;
    pReg->extents.bottom = 0;
    return;
  }
  
  pExtents = &pReg->extents;
  pRect = pReg->rects;
  pRectEnd = &pRect[pReg->rects_use - 1];
  
  /* Since pRect is the first rectangle in the region,
   * it must have the smallest top and since pRectEnd is the last rectangle
   * in the region,
   * it must have the largest bottom, because of banding.
   * Initialize top and right from pRect and pRectEnd, resp.,
   * as good things to initialize them to...
   */
  pExtents->top = pRect->top;
  pExtents->bottom = pRectEnd->bottom;
  
  pExtents->left = pRect->left;
  pExtents->right = pRectEnd->right;
  
  while (pRect <= pRectEnd) {
    if (pRect->left < pExtents->left) {
      pExtents->left = pRect->left;
    }
    if (pRect->right > pExtents->right) {
      pExtents->right = pRect->right;
    }
    pRect++;
  }
}

/* =======================
 *     subtract region
 * ======================= */

/* Deal with non-overlapping band for subtraction.
 * Any parts from region 2 we discard.
 * Any parts from region 1 we add to the region.
 */
static void
subtract_none_overlapping_band(KWRegion_t *pReg, KWRect_t *r, KWRect_t *rEnd,
			       int top, int bottom)
{
  KWRect_t *pNextRect = &(pReg->rects[pReg->rects_use]);
  
  while (r != rEnd) {
    MEMCHECK(pReg, pNextRect);
    
    pNextRect->left   = r->left;
    pNextRect->top    = top;
    pNextRect->right  = r->right;
    pNextRect->bottom = bottom;
    
    pReg->rects_use += 1;
    pNextRect++;
    r++;
  }
}

/* Overlapping band subtraction.
 *
 * @Side Effects:
 *        pReg may have rectangles added to it.
 */
static void
subtract_overlapping_band(KWRegion_t *pReg, KWRect_t *r1, KWRect_t *r1End,
			  KWRect_t *r2, KWRect_t *r2End, int top, int bottom)
{
  KWRect_t *pNextRect = &(pReg->rects[pReg->rects_use]);
  int left = r1->left;
  
  while ((r1 != r1End) && (r2 != r2End)) {
    if (r2->right <= left) {
      r2++;
    } else if (r2->left <= left) {
      left = r2->right;
      if (left >= r1->right) {
	r1++;
	if (r1 != r1End) {
	  left = r1->left;
	}
      } else {
	r2++;
      }
    } else if (r2->left < r1->right) {
      MEMCHECK(pReg, pNextRect);
      
      pNextRect->left   = left;
      pNextRect->top    = top;
      pNextRect->right  = r2->left;
      pNextRect->bottom = bottom;
      
      pReg->rects_use += 1;
      pNextRect++;
      left = r2->right;
      
      if (left >= r1->right) {
	r1++;
	if (r1 != r1End) {
	  left = r1->left;
	}
      } else {
	r2++;
      }
    } else {
      if (r1->right > left) {
	MEMCHECK(pReg, pNextRect);
	
	pNextRect->left   = left;
	pNextRect->top    = top;
	pNextRect->right  = r1->right;
	pNextRect->bottom = bottom;
	
	pReg->rects_use += 1;
	pNextRect++;
      }
      
      r1++;
      left = r1->left;
    }
  }
  
  while (r1 != r1End) {
    MEMCHECK(pReg, pNextRect);
    
    pNextRect->left   = left;
    pNextRect->top    = top;
    pNextRect->right  = r1->right;
    pNextRect->bottom = bottom;
    
    pReg->rects_use += 1;
    pNextRect++;
    r1++;
    
    if (r1 != r1End) {
      left = r1->left;
    }
  }
}

/* ========================
 *     intersect region
 * ======================== */

/* Handle an overlapping band for intersect_region().
 *
 * @Results:
 *      None.
 *
 * @Side Effects:
 *      Rectangles may be added to the region.
 */
static void
intersect_overlapping_band(KWRegion_t *pReg,  KWRect_t *r1, KWRect_t *r1End,
			   KWRect_t *r2, KWRect_t *r2End, int top, int bottom)
{
  int        left, right;
  KWRect_t  *pNextRect = &pReg->rects[pReg->rects_use];
  
  while ((r1 != r1End) && (r2 != r2End)) {
    left = MAX(r1->left, r2->left);
    right = MIN(r1->right, r2->right);
    
    /* If there's any overlap between the two rectangles, add that
     * overlap to the new region.
     * There's no need to check for subsumption because the only way
     * such a need could arise is if some region has two rectangles
     * right next to each other. Since that should never happen...
     */
    if (left < right) {
      MEMCHECK(pReg, pNextRect);
      
      pNextRect->left = left;
      pNextRect->top = top;
      pNextRect->right = right;
      pNextRect->bottom = bottom;
      
      pReg->rects_use += 1;
      pNextRect++;
    }
    
    /* Need to advance the pointers. Shift the one that extends
     * to the right the least, since the other still has a chance to
     * overlap with that region's next rectangle, if you see what I mean.
     */
    if (r1->right < r2->right) {
      r1++;
    } else if (r2->right < r1->right) {
      r2++;
    } else {
      r1++;
      r2++;
    }
  }
}

/* ========================
 *     export functions
 * ======================== */

bool
init_region(KWRegion_t *region, int left, int top, int right, int bottom)
{
  if (region->rects != NULL) {
    free(region->rects);
  }
  
  if ((region->rects = (KWRect_t *)malloc(sizeof(KWRect_t))) == NULL) {
    LOG(("%s\n", "<Server Error> allocate a new fresh region failed."));
    region->rects_have = 0;
    return false;
  }
  
  region->rects_have = 1;
  
  /* setup the first rect in the rects array. */
  if (left != right && top != bottom) {
    region->rects->left   = region->extents.left   = left;
    region->rects->top    = region->extents.top    = top;
    region->rects->right  = region->extents.right  = right;
    region->rects->bottom = region->extents.bottom = bottom;
    region->rects_use = 1;
  }
  
  region->rects->left   = 0;
  region->rects->top    = 0;
  region->rects->right  = 0;
  region->rects->bottom = 0;
  region->rects_use = 0;
  
  return true;
}

void
subtract_region(KWRegion_t *regD, KWRegion_t *regM, KWRegion_t *regS)
{
  if ((regM->rects_use == 0) || (regS->rects_use == 0) ||
      (CHECK_EXTENT_OVERLAP(&regM->extents, &regS->extents) == 0)) {
    return;
  } else {
    region_operation(regD, regM, regS,
		     (voidProcp) subtract_overlapping_band,
		     (voidProcp) subtract_none_overlapping_band,
		     (voidProcp) NULL);
    set_region_extents(regD);
  }
}

void
intersect_region(KWRegion_t *newReg, KWRegion_t *reg1, KWRegion_t *reg2)
{
  /* check for trivial reject */
  if ((reg1->rects_use == 0) ||
      (reg2->rects_use == 0) ||
      (CHECK_EXTENT_OVERLAP(&reg1->extents, &reg2->extents) == 0)) {
    /* if go into here, it means those 2 regions don't overlap with each other */
    newReg->rects_use = 0;
  } else {
    region_operation(newReg, reg1, reg2, 
		     (voidProcp) intersect_overlapping_band,
		     (voidProcp) NULL,
		     (voidProcp) NULL);
  }
  
  /* Can't alter newReg's extents before we call region_operation() because
   * it might be one of the source regions and region_operation() depends
   * on the extents of those regions being the same. Besides, this
   * way there's no checking against rectangles that will be nuked
   * due to coalescing, so we have to examine fewer rectangles.
   */
  set_region_extents(newReg);
}
