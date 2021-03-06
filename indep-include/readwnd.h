/***************************************************************************
 SimpleMail - Copyright (C) 2000 Hynek Schlawack and Sebastian Bauer

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
***************************************************************************/

/*
** readwnd.h
*/

#ifndef SM__READWND_H
#define SM__READWND_H

int read_window_open(char *folder, struct mail_info *mail, int window);
void read_window_activate(int num);
void read_window_close(int num);
void read_window_cleanup(void);
struct mail_complete *read_window_get_displayed_mail(int num);
void read_refresh_prevnext_button(struct folder *f);

#endif
