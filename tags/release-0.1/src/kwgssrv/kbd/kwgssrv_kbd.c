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

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include <kwgsshr/kwgsshr.h>
#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     static variables
 * ======================== */

static struct termios  old_terminal_attr;      /* original terminal modes      */
static int             old_kbd_mode;
static unsigned char   key_state[KWKEY_LAST];
static unsigned int    key_modstate;
static int             kbd_socket_fd_send;
static pthread_t       kbd_thread;
static unsigned short  os_keymap[NUM_VGAKEYMAPS][NR_KEYS];

#include "kwgssrv_kbd_keymap.c"

/* ========================
 *     export variables
 * ======================== */

int  terminal_fd;            /* file descriptor for keyboard */
int  kbd_socket_fd_receive;

/* ========================
 *     static functions
 * ======================== */

static void
update_LED_state(void)
{
  int  ledstate = 0;
  
  /* should the CAPSLOCK led turn light? */
  if (key_modstate & KWKEY_MODSTATE_CAPS) {
    ledstate |= LED_CAP;
  }
  
  /* should the NUMLOCK led turn light? */
  if (key_modstate & KWKEY_MODSTATE_NUM) {
    ledstate |= LED_NUM;
  }
  
  ioctl(terminal_fd, KDSETLED, ledstate);
}

/* Update the internal keyboard state,
 * return TRUE if changed.
 */
static bool
update_key_state(int pressed, unsigned short kwkey)
{
  if (pressed == KEY_PRESSED) {
    switch (kwkey) {
    case KWKEY_NUMLOCK:
    case KWKEY_CAPSLOCK:
      /* change state on release because of auto-repeat */
      return false;
      
    case KWKEY_LCTRL:
      key_modstate |= KWKEY_MODSTATE_LCTRL;
      break;
      
    case KWKEY_RCTRL:
      key_modstate |= KWKEY_MODSTATE_RCTRL;
      break;
      
    case KWKEY_LSHIFT:
      key_modstate |= KWKEY_MODSTATE_LSHIFT;
      break;
      
    case KWKEY_RSHIFT:
      key_modstate |= KWKEY_MODSTATE_RSHIFT;
      break;
      
    case KWKEY_LALT:
      key_modstate |= KWKEY_MODSTATE_LALT;
      break;
      
    case KWKEY_RALT:
      key_modstate |= KWKEY_MODSTATE_RALT;
      break;
      
    case KWKEY_LMETA:
      key_modstate |= KWKEY_MODSTATE_LMETA;
      break;
      
    case KWKEY_RMETA:
      key_modstate |= KWKEY_MODSTATE_RMETA;
      break;
      
    case KWKEY_ALTGR:
      key_modstate |= KWKEY_MODSTATE_ALTGR;
      break;
      
    default:
      /* The other keys don't impact the internal key states. */
      break;
    }
  } else {
    /* KEY_RELEASED */
    switch (kwkey) {
    case KWKEY_NUMLOCK:
      key_modstate ^= KWKEY_MODSTATE_NUM;
      key_state[KWKEY_NUMLOCK] ^= KEY_PRESSED;
      
      update_LED_state();
      
      return true;
      
    case KWKEY_CAPSLOCK:
      key_modstate ^= KWKEY_MODSTATE_CAPS;
      key_state[KWKEY_CAPSLOCK] ^= KEY_PRESSED;
      
      update_LED_state();
      
      return true;
      
    case KWKEY_LCTRL:
      key_modstate &= ~KWKEY_MODSTATE_LCTRL;
      break;
      
    case KWKEY_RCTRL:
      key_modstate &= ~KWKEY_MODSTATE_RCTRL;
      break;
      
    case KWKEY_LSHIFT:
      key_modstate &= ~KWKEY_MODSTATE_LSHIFT;
      break;
      
    case KWKEY_RSHIFT:
      key_modstate &= ~KWKEY_MODSTATE_RSHIFT;
      break;
      
    case KWKEY_LALT:
      key_modstate &= ~KWKEY_MODSTATE_LALT;
      break;
      
    case KWKEY_RALT:
      key_modstate &= ~KWKEY_MODSTATE_RALT;
      break;
      
    case KWKEY_LMETA:
      key_modstate &= ~KWKEY_MODSTATE_LMETA;
      break;
      
    case KWKEY_RMETA:
      key_modstate &= ~KWKEY_MODSTATE_RMETA;
      break;
      
    case KWKEY_ALTGR:
      key_modstate &= ~KWKEY_MODSTATE_ALTGR;
      break;
      
    default:
      /* The other keys don't impact the internal key states. */
      break;
    }
  }
  
  /* Update internal keyboard state */
  key_state[kwkey] = (unsigned char)pressed;
  
  return true;
}

/* translate a keycode and modifier state to an unsigned short (ascii code value). */
static unsigned short
translate_keycode(int keycode)
{
  unsigned short  kwkey = 0;
  int  map = 0;
  
  /* determine appropriate kernel table */
  /* different key binding will result to different keymap table,
   * and, of course, will result different ascii code value.
   *
   * ex:
   *         key        ascii value
   *          \             5c
   *      alt+\             5c
   *    shift+\             7c
   *     ctrl+\             1c
   */
  if (key_modstate & KWKEY_MODSTATE_SHIFT) {
    map |= (1 << KG_SHIFT);
  }
  
  if (key_modstate & KWKEY_MODSTATE_CTRL) {
    map |= (1 << KG_CTRL);
  }
  
  if (key_modstate & KWKEY_MODSTATE_ALT) {
    map |= (1 << KG_ALT);
  }
  
  if (key_modstate & KWKEY_MODSTATE_ALTGR) {
    map |= (1 << KG_ALTGR);
  }
  
  /* KTYP() used to get the key type from a 2-byte key symbol. */
  if (KTYP(os_keymap[map][keycode]) == KT_LETTER) {
    /* This means that the key we pressed or released is an upper case letter */
    if (key_modstate & KWKEY_MODSTATE_CAPS) {
      map |= (1 << KG_SHIFT);
    }
  }
  
  /* re-fetch the key type,
   * because we may change the key mapping from the above if statement.
   */
  if (KTYP(os_keymap[map][keycode]) == KT_PAD) {
    if (key_modstate & KWKEY_MODSTATE_NUM) {
      switch (keymap[keycode]) {
      case KWKEY_KP0:
      case KWKEY_KP1:
      case KWKEY_KP2:
      case KWKEY_KP3:
      case KWKEY_KP4:
      case KWKEY_KP5:
      case KWKEY_KP6:
      case KWKEY_KP7:
      case KWKEY_KP8:
      case KWKEY_KP9:
	/* remember that the return value is in _ascii_ format. */
	kwkey = keymap[keycode] - KWKEY_KP0 + '0';
	break;
	
      case KWKEY_KP_PERIOD:
	/* remember that the return value is in _ascii_ format. */
	kwkey = '.';
	break;
	
      case KWKEY_KP_DIVIDE:
	kwkey = '/';
	break;
	
      case KWKEY_KP_MULTIPLY:
	kwkey = '*';
	break;
	
      case KWKEY_KP_MINUS:
	kwkey = '-';
	break;
	
      case KWKEY_KP_PLUS:
	kwkey = '+';
	break;
	
      case KWKEY_KP_ENTER:
	kwkey = KWKEY_ENTER;
	break;
	
      case KWKEY_KP_EQUALS:
	kwkey = '-';
	break;
      }
    }
  } else {
    /* not numeric pad key */
    /* remember that the return value is in _ascii_ format. */
    kwkey = KVAL(os_keymap[map][keycode]);
  }
  
  if (kwkey == 0) {
    /* If the kernel keymap table can not tell us what the key's ascii code is,
     * then try our own keymap table.
     */
    kwkey = keymap[keycode];
  }
  
  /* perform additional translations,
   * remember, the kwkey now is in _ascii_ format.
   */
  switch (kwkey) {
  case 127:
    /* the ascii code 127 (0x7f) is delete key,
     * and we expect that the delete key will act the same with backspace key.
     *
     * KWKEY_BACKSPACE = 0x8.
     * i.e. we re-map 0x7f to 0x8.
     */
    kwkey = KWKEY_BACKSPACE;
    break;
    
  case KWKEY_BREAK:
  case KWKEY_PAUSE:
    /* we will treat Pause & Break key as Kiwin's quit key now. */
    kwkey = KWKEY_QUIT;
    break;
    
  case KWKEY_PRINT:
  case KWKEY_SYSREQ:
    /* we will treat PrtSc & SysRq key as Kiwin's print key now. */
    kwkey = KWKEY_PRINT;
    break;
  }
  
  return kwkey;
}

/* Handle switching to another VC, returns when our VC is back */
static bool
switch_vt(unsigned short which)
{
  struct vt_stat vtstate;
  unsigned short current;
  
  /* Figure out whether or not we're switching to a new console */
  /* This VT_GETSTATE ioctl gets the state of the _active_ VT.
   * The argument passed to ioctl() is a pointer to a struct vt_stat.
   */
  if ((ioctl(terminal_fd, VT_GETSTATE, &vtstate) < 0) || (which == vtstate.v_active)) {
    /* We still at current virtual terminal,
     * so do not need a switching,
     * just return.
     */
    return false;
  }
  current = vtstate.v_active;
  
  /* goto text mode */
  ioctl(terminal_fd, KDSETMODE, KD_TEXT);
  
  /* New console, switch to it */
  if (ioctl(terminal_fd, VT_ACTIVATE, which) == 0) {
    /* Wait for our console to be activated again */
    while (ioctl(terminal_fd, VT_WAITACTIVE, current) < 0) {
      if ((errno != EINTR) && (errno != EAGAIN)) {
	/* Unknown VT error, cancel */
	LOG(("%s\n", "<Server Error> VT_WAITACTIVE ioctl error."));
	break;
      }
      usleep(100000);
    }
  }
  
  /* Restore graphics mode and the contents of the screen */
  ioctl(terminal_fd, KDSETMODE, KD_GRAPHICS);
  return true;
}

/* This reads one keystroke from the keyboard, and the current state of
 * the modifier keys (ALT, SHIFT, etc).  Returns -1 on error, 0 if no data
 * is ready, 1 on a keypress, and 2 on keyrelease.
 * This is a non-blocking call.
 */
static int
kbd_read(unsigned short *kbuf)
{
  int  cc;  /* characters read */
  int  pressed;
  int  keycode;
  unsigned short  kwkey;
  unsigned char   buf[128];
  
  /* the following cc is a keycode, not a scancode or an ascii code. */
  cc = read(terminal_fd, buf, 1);
  if (cc > 0) {
    pressed = (*buf & 0x80) ? KEY_RELEASED : KEY_PRESSED;
    keycode = *buf & 0x7f;
    kwkey = keymap[keycode]; /* we first parse this keycode through our keymap[] array. */
    
    /* Handle Alt-FN for vt switch */
    switch (kwkey) {
    case KWKEY_F1:
    case KWKEY_F2:
    case KWKEY_F3:
    case KWKEY_F4:
    case KWKEY_F5:
    case KWKEY_F6:
    case KWKEY_F7:
    case KWKEY_F8:
    case KWKEY_F9:
    case KWKEY_F10:
    case KWKEY_F11:
    case KWKEY_F12:
      if (key_modstate & KWKEY_MODSTATE_ALT) {
	/* If we already pressed ALT,
	 * then pressing F1 ~ F12 means a virtual terminal switching.
	 */
	if (switch_vt(kwkey - KWKEY_F1 + 1)) {
	  kwkey = KWKEY_REDRAW;
	}
      }
      break;
      
    case ' ':
      if (key_modstate & KWKEY_MODSTATE_CTRL) {
	/* CTRL + space == IME key. */
	kwkey = KWKEY_IME;
      }
      break;
      
      /* Fall through to _normal_ processing */
    default:
      /* update internal key states */
      if (update_key_state(pressed, kwkey) == false) {
	return 0;
      }
      
      /* kwkey is 0 if only a modifier is hit */
      if (kwkey != KWKEY_LCTRL &&
	  kwkey != KWKEY_RCTRL &&
	  kwkey != KWKEY_LALT &&
	  kwkey != KWKEY_RALT &&
	  kwkey != KWKEY_LSHIFT &&
	  kwkey != KWKEY_RSHIFT) {
	/* translate keycode to _ascii_ key value */
	kwkey = translate_keycode(keycode);
      }
      
      break;
    } /* switch(kwkey) */
    
    *kbuf = kwkey;
    
    return pressed ? 1 : 2;
  } /* if (cc > 0) */
  
  if ((cc < 0) && (errno != EINTR) && (errno != EAGAIN)) {
    return -1;
  }
  
  /* because the 3rd argument of read above is 1,
   * so the only 1 condition that read() will return 0 is end-of-file,
   * but this is not possible for the /dev/tty device,
   * so the read above doesn't have possible to return 0,
   * so we don't need to handle it, and just return 0.
   */
  
  return 0;
}

static void
kbd_process(void)
{
  KWKbdEventInternal_t  event;
  unsigned short  ch;
  int             result;
  int             written;
  int             length = sizeof(KWKbdEventInternal_t);
  char           *buf = (char *)(&event);
  
  result = kbd_read(&ch);
  
  switch (result) {
  case 1:
    /* key down */
    event.type = KEY_DOWN;
    event.ch = ch;
    
    break;
  case 2:
    /* key up */
    event.type = KEY_UP;
    event.ch = ch;
    
    break;
  case 0:
  case -1:
    return;
  }
  
  do {
    written = write(kbd_socket_fd_send, buf, length);
    if (written < 0) {
      if (errno == EAGAIN || errno == EINTR) {
	continue;
      }
      
      LOG(("%s\n", "<Server Error> send keyboard event to graphic server failed."));
      return;
    }
    
    buf += written;
    length -= written;
  } while (length > 0);
  
  return;
}

/* load Linux keyboard mappings, used as the 1st try for keycode conversion */
static void
load_kernel_keymaps(void)
{
  int  map, i;
  struct kbentry  entry;
  
  /* Load all the keysym mappings */
  for (map = 0; map < NUM_VGAKEYMAPS; map++) {
    memset(os_keymap[map], 0, NR_KEYS * sizeof(unsigned short));
    
    for (i = 0; i < NR_KEYS; i++) {
      entry.kb_table = map;
      entry.kb_index = i;
      
      if (ioctl(terminal_fd, KDGKBENT, &entry) == 0) {
	/* change K_ENTER to \r */
	if (entry.kb_value == K_ENTER) {
	  entry.kb_value = K(KT_ASCII, 13);
	}
	
	if ((KTYP(entry.kb_value) == KT_LATIN) ||
	    (KTYP(entry.kb_value) == KT_ASCII) ||
	    (KTYP(entry.kb_value) == KT_PAD) ||
	    (KTYP(entry.kb_value) == KT_LETTER)) {
	  os_keymap[map][i] = entry.kb_value;
	}
      }
    }
  }
}

static void*
kbd_thread_func(void *unused)
{
  while (1) {
    kbd_process();
  }
}

/* ========================
 *     export functions
 * ======================== */

/* Open the keyboard.
 * This is real simple, we just use a special file handle
 * that allows non-blocking I/O, and put the terminal into
 * character mode.
 */
bool
kbd_init(void)
{
  int     i;
  int     ledstate = 0;
  size_t  size;
  int     tries;
  int     ret = 0;
  struct sockaddr_un  name;
  struct termios      new_terminal_attr;
  
  /* Open /dev/tty0 device */
  terminal_fd = open(KEYBOARD_DEV_FILE, 0);
  if (terminal_fd < 0) {
    LOG(("%s\n", "<Server Error> keyboard init error because of opening the terminal file failed."));
    return false;
  }
  
  /* Save previous settings,
   * so that we can put the keyboard mode back to the original state.
   */
  if (ioctl(terminal_fd, KDGKBMODE, &old_kbd_mode) < 0) {
    LOG(("%s\n", "<Server Error> keyboard init error because of ioctl(KDGKBMODE) failed."));
    close(terminal_fd);
    terminal_fd = -1;
    return false;
  }
  if (tcgetattr(terminal_fd, &old_terminal_attr) < 0) {
    LOG(("%s\n", "<Server Error> keyboard init error because of tcgetattr() failed."));
    close(terminal_fd);
    terminal_fd = -1;
    return false;
  }
  
  new_terminal_attr = old_terminal_attr;
  /* ISIG and BRKINT must be set otherwise '2' is ^C (scancode 3)!! */
  new_terminal_attr.c_lflag     &= ~(ICANON | ECHO | ISIG);
  new_terminal_attr.c_iflag     &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON | BRKINT);
  new_terminal_attr.c_cc[VMIN]   = 0;
  new_terminal_attr.c_cc[VTIME]  = 0;
  
  if (tcsetattr(terminal_fd, TCSAFLUSH, &new_terminal_attr) < 0) {
    kbd_finalize();
    return false;
  }
  
  /* Set medium-raw keyboard mode.
   * This mode will return the keycode rather than scancode or ascii code.
   */
  if (ioctl(terminal_fd, KDSKBMODE, K_MEDIUMRAW) < 0) {
    LOG(("%s\n", "<Server Error> keyboard init error because of ioctl(KDSKBMODE, K_MEDIUMRAW) failed."));
    kbd_finalize();
    return false;
  }
  
  /* Load OS keymappings */
  load_kernel_keymaps();
  
  /* Initialize keyboard state */
  key_modstate = KWKEY_MODSTATE_NONE;
  for (i = 0; i < KWKEY_LAST; i++) {
    key_state[i] = KEY_RELEASED;
  }
  
  /* preset CAPSLOCK and NUMLOCK from startup LED state */
  if (ioctl(terminal_fd, KDGETLED, &ledstate) == 0) {
    if (ledstate & LED_CAP) {
      /* the CAPSLOCK led should turn light. */
      key_modstate |= KWKEY_MODSTATE_CAPS;
      key_state[KWKEY_CAPSLOCK] = KEY_PRESSED;
    }
    if (ledstate & LED_NUM) {
      /* the NUMLOCK led should turn light. */
      key_modstate |= KWKEY_MODSTATE_NUM;
      key_state[KWKEY_NUMLOCK] = KEY_PRESSED;
    }
  }
  update_LED_state();
  
  /* connect keyboard driver to graphic server using socket. */
  kbd_socket_fd_send = socket(AF_UNIX, SOCK_STREAM, 0);
  if (kbd_socket_fd_send == -1) {
    LOG(("%s\n", "<Server Error> can not open the keyboard socket fd."));
    kbd_finalize();
    return false;
  }
  
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path, KW_GS_NAMED_SOCKET);
  size = ((unsigned int)(((struct sockaddr_un *)0)->sun_path) + strlen(name.sun_path) + 1);
  
  for (tries = 1; tries <= 10; tries++) {
    struct timespec req;
    
    ret = connect(kbd_socket_fd_send, (struct sockaddr *) &name, size);
    if (ret >= 0) {
      break;
    }
    req.tv_sec = 0;
    req.tv_nsec = 100000000;
    nanosleep(&req, NULL);
    LOG(("<Server Error> keyboard driver retry connect to graphic server attempt %d\n", tries));
  }
  
  if (ret == -1) {
    close(kbd_socket_fd_send);
    LOG(("%s\n", "<Server Error> retry 10 times and keyboard driver still can not connect to graphic server."));
    kbd_finalize();
    return false;
  }
  
  pthread_create(&kbd_thread, NULL, kbd_thread_func, NULL);
  
  return true;
}

/* Close the keyboard.
 * This resets the terminal modes.
 */
bool
kbd_finalize(void)
{
  /* KDSETLED
   * ...
   * However, if a higher order bit is set, the LEDs revert to normal:
   * ...
   */
  int  ledstate = 0x80000000;
  
  if (terminal_fd >= 0) {
    /* revert LEDs to follow key modifiers */
    if (ioctl(terminal_fd, KDSETLED, ledstate) < 0) {
      LOG(("%s\n", "<Server Warning> can not finalize keyboard driver correctly because of ioctl(KDSETLED) failed."));
    }
    
    /* reset terminal mode */
    if (ioctl(terminal_fd, KDSKBMODE, old_kbd_mode) < 0) {
      LOG(("%s\n", "<Server Warning> can not finalize keyboard driver correctly because of ioctl(KDSKBMODE) failed."));
    }
    tcsetattr(terminal_fd, TCSAFLUSH, &old_terminal_attr);
    
    close(terminal_fd);
  }
  
  terminal_fd = -1;
  return true;
}
