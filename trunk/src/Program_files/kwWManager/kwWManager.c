/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
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

#include "kwWManager.h"

/* ========================
 *     static variables
 * ======================== */

static bool           regfile_has_inited = false;

static int            border_size = 0;
static int            caption_height = 0;

static unsigned int   caption_light_color = WHITE;
static unsigned int   caption_dark_color = WHITE;

static KWWMThread_t  *wmthread_list = NULL;

/* ========================
 *     static functions
 * ======================== */

static bool
retrive_attribute_from_regfile(KWWmProps_t props)
{
  datum key_data;
  datum return_data;
  char *tail = NULL;
  
  switch (props) {
  case KWWmPropsDefBorderSize:
    key_data.dptr = "default_border_size";
    key_data.dsize = strlen("default_border_size") + 1;
    
    return_data = gdbm_fetch(reg_file, key_data);
    if (return_data.dptr == NULL) {
      LOG(("<Window Manager Error> window manager %d: default border size doesn't found.\n", getpid()));
      return false;
    }
    
    errno = 0;
    border_size = strtol(return_data.dptr, &tail, 0);
    if (errno != 0) {
      LOG(("<Window Manager Error> window manager %d: strtol() overflow.\n", getpid()));
    }
    
    free(return_data.dptr);
    break;
    
  case KWWmPropsDefCaptionHeight:
    key_data.dptr = "default_caption_height";
    key_data.dsize = strlen("default_caption_height") + 1;
    
    return_data = gdbm_fetch(reg_file, key_data);
    if (return_data.dptr == NULL) {
      LOG(("<Window Manager Error> window manager %d: default caption height doesn't found.\n", getpid()));
      return false;
    }
    
    errno = 0;
    caption_height = strtol(return_data.dptr, &tail, 0);
    if (errno != 0) {
      LOG(("<Window Manager Error> window manager %d: strtol() overflow.\n", getpid()));
    }
    
    free(return_data.dptr);
    break;
    
  case KWWmPropsDefCaptionLightColor:
    key_data.dptr = "default_caption_light_color";
    key_data.dsize = strlen("default_caption_light_color") + 1;
    
    return_data = gdbm_fetch(reg_file, key_data);
    if (return_data.dptr == NULL) {
      LOG(("<Window Manager Error> window manager %d: default caption light color doesn't found.\n", getpid()));
      return false;
    }
    
    errno = 0;
    caption_light_color = strtoul(return_data.dptr, &tail, 16);
    if (errno != 0) {
      LOG(("<Window Manager Error> window manager %d: strtol() overflow.\n", getpid()));
    }
    
    free(return_data.dptr);
    break;
    
  case KWWmPropsDefCaptionDarkColor:
    key_data.dptr = "default_caption_dark_color";
    key_data.dsize = strlen("default_caption_dark_color") + 1;
    
    return_data = gdbm_fetch(reg_file, key_data);
    if (return_data.dptr == NULL) {
      LOG(("<Window Manager Error> window manager %d: default caption dark color doesn't found.\n", getpid()));
      return false;
    }
    
    errno = 0;
    caption_dark_color = strtoul(return_data.dptr, &tail, 16);
    if (errno != 0) {
      LOG(("<Window Manager Error> window manager %d: strtol() overflow.\n", getpid()));
    }
    
    free(return_data.dptr);
    break;
  }
  
  return true;
}

static void
wnd_thread_func(void *eventp)
{
  KWEventInitWMComponent_t *event = (KWEventInitWMComponent_t *)eventp;
  KWWnd_t                  *wndlist = NULL;
  KWWnd_t                  *wp = NULL;
  int                       x, y, width, height;
  
  unsigned int              caption_wid;
  unsigned int              left_border_wid;
  unsigned int              right_border_wid;
  unsigned int              top_border_wid;
  unsigned int              bottom_border_wid;
  
  width = event->width;
  height = event->height;
  
  if (event->props | WNDPROPS_HAVE_BORDER) {
    /* top border */
    top_border_wid = KCreateWindow(event->parentid,
				   0,
				   0,
				   width,
				   border_size,
				   border_color, 0);
    
    /* bottom border */
    bottom_border_wid = KCreateWindow(event->parentid,
				      0,
				      height - border_size,
				      width,
				      border_size,
				      border_color, 0);
    
    /* left border */
    left_border_wid = KCreateWindow(event->parentid,
				    0,
				    border_size,
				    border_size,
				    height - (border_size * 2),
				    border_color, 0);
    
    /* right border */
    right_border_wid = KCreateWindow(event->parentid,
				     height - border_size,
				     border_size,
				     border_size,
				     height - (border_size * 2),
				     border_color, 0);
    
    KSelectEvents(top_border_wid, EVENT_MASK_EXPOSURE);
    KSelectEvents(bottom_border_wid, EVENT_MASK_EXPOSURE);
    KSelectEvents(left_border_wid, EVENT_MASK_EXPOSURE);
    KSelectEvents(right_border_wid, EVENT_MASK_EXPOSURE);
    
    KMapWindow(top_border_wid);
    KMapWindow(bottom_border_wid);
    KMapWindow(left_border_wid);
    KMapWindow(right_border_wid);
  }
  
  if (event->props | WNDPROPS_HAVE_CAPTION) {
    caption_wid = KCreateWindow(event->parentid,
				0,
				0,
				event->width,
				caption_height,
				caption_dark_bgcolor, 0);
    
    KSelectEvents(caption_wid, EVENT_MASK_EXPOSURE);
    KMapWindow(caption_wid);
  }
  
  for (;;) {
    get_next_event(&event);
    switch (event.type) {
    case EVENT_TYPE_EXPOSURE:
      x = event.exposure.x;
      y = event.exposure.y;
      width = event.exposure.width;
      height = event.exposure.height;
      
      if (event.exposure.wid == caption_wid) {
	if ((wp = find_window(caption_wid)) != NULL) {
	  drawCaption(wp, x, y, width, height);
	}
      }
      
      if (event.exposure.wid == left_border_wid) {
	if ((wp = find_window(left_border_wid)) != NULL) {
	  drawBorder(wp, x, y, width, height);
	}
      }
      
      if (event.exposure.wid == right_border_wid) {
	if ((wp = find_window(right_border_wid)) != NULL) {
	  drawBorder(wp, x, y, width, height);
	}
      }
      
      if (event.exposure.wid == top_border_wid) {
	if ((wp = find_window(top_border_wid)) != NULL) {
	  drawBorder(wp, x, y, width, height);
	}
      }
      
      if (event.exposure.wid == bottom_border_wid) {
	if ((wp = find_window(bottom_border_wid)) != NULL) {
	  drawBorder(wp, x, y, width, height);
	}
      }
      break;
    }
  }
}

static KWWMThread_t*
allocKWWMThread(void)
{
  KWWMThread_t *result = NULL;
  
  result = (KWWMThread_t *)malloc(sizeof(KWWMThread_t));
  if (wmthread_list == NULL) {
    wmthread_list = result;
  } else {
    result->next = wmthread_list;
    wmthread_list = result;
  }
  
  return result;
}

/* ========================
 *     export functions
 * ======================== */

int
main(int argc, char **argv)
{
  KWWMThread_t *thread = NULL;
  bool          regfile_has_inited = false;
  unsigned int  wmid;
  
  regfile_init();
  regfile_has_inited = true;
  
  retrive_attribute_from_regfile(KWWmPropsDefBorderSize);
  retrive_attribute_from_regfile(KWWmPropsDefCaptionHeight);
  retrive_attribute_from_regfile(KWWmPropsDefCaptionLightColor);
  retrive_attribute_from_regfile(KWWmPropsDefCaptionDarkColor);
  
  mmid = KRegisterWManager(WMANAGER_HAVE_CAPTION | WMANAGER_HAVE_BORDER);
  if (wmid == BAD_WMANAGER_ID) {
    LOG(("<Window Manager Error> window manager %d: can not create the window manager.\n", getpid()));
    if (regfile_has_inited == true) {
      regfile_finalize();
    }
    return;
  }
  
  for (;;) {
    KGetNextEvent(&event);
    switch (event.type) {
    case EVENT_TYPE_INIT_WMCOMPONENT:
      thread = allocKWWMThread();
      pthread_create(thread->pid, NULL, wnd_thread_func, &event);
      break;
    }
  }
}
