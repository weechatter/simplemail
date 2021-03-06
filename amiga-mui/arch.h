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
** arch.h - architecture specific definitions
*/

#ifndef SM__ARCH_H
#define SM__ARCH_H

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif

/* Paths */
#define SM_DIR					"PROGDIR:"
#define SM_CHARSET_DIR	"PROGDIR:Charsets"
#define SM_FOLDER_DIR	"PROGDIR:.folders"
#define SM_CURRENT_DIR	""

/* Operating system */
#if defined(__AMIGAOS4__)
#define SM_OPERATINGSYSTEM "AmigaOS4/MUI"
#elif defined(__MORPHOS__)
#define SM_OPERATINGSYSTEM "MorphOS/MUI"
#elif defined(__AROS__)
#define SM_OPERATINGSYSTEM "AROS/MUI"
#else
#define SM_OPERATINGSYSTEM "AmigaOS/MUI"
#endif

/* Debug - defines ARCH_DEBUG_EXTRA */
#if defined(__GNUC__) && defined(__AMIGAOS4__)
#define ARCH_DEBUG_EXTRA ((IExec->FindTask)(NULL)->tc_Node.ln_Name)
#else
#define ARCH_DEBUG_EXTRA (FindTask(NULL)->tc_Node.ln_Name)
#endif

#endif
