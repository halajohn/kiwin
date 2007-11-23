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

/* ========================
 *     export functions
 * ======================== */

void
KWMDrawCaption(KWWnd_t *wp, int x, int y, int width, int height)
{
  if (wp->focus_in == true) {
    fillWndRect(wp, caption_light_color,
		x, y, width, height,
		false);
  } else {
    fillWndRect(wp, caption_dark_color,
		x, y, width, height,
		false);
  }
}

void
KWMDrawBorder(KWWnd_t *wp, int x, int y, int width, int height)
{
  fillWndRect(wp, wp->bg_color, x, y, width, height, false);
}
