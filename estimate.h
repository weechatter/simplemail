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

#ifndef SM__ESTIMATE_H
#define SM__ESTIMAGE_H

struct estimate
{
	unsigned int init_seconds;
	unsigned int init_micros;
	unsigned int max_value;
};

void estimate_init(struct estimate *est, unsigned int new_max_value);
unsigned int estimate_calc(struct estimate *est,unsigned int value);
unsigned int estimate_calc_remaining(struct estimate *est,unsigned int value);

#endif