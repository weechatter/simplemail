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
** filterwnd.c
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libraries/iffparse.h> /* MAKE_ID */
#include <libraries/mui.h>
#include <mui/betterstring_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include "configuration.h"
#include "filter.h"
#include "folder.h"
#include "simplemail.h"
#include "smintl.h"
#include "support_indep.h"

#include "compiler.h"
#include "foldertreelistclass.h"
#include "mailtreelistclass.h"
#include "muistuff.h"
#include "utf8stringclass.h"
#include "searchwnd.h"

struct MUI_NListtree_TreeNode *FindListtreeUserData(Object *tree, APTR udata);

static Object *search_wnd;
static Object *search_folder_popobject;
static Object *search_folder_text;
static Object *search_folder_tree;
static Object *search_from_string;
static Object *search_to_string;
static Object *search_subject_string;
static Object *search_body_string;
static Object *search_start_button;
static Object *search_stop_button;
static Object *search_mail_tree;

static int has_mails;

STATIC ASM VOID folder_objstr(register __a2 Object *list, register __a1 Object *str)
{
	struct MUI_NListtree_TreeNode *tree_node;
	tree_node = (struct MUI_NListtree_TreeNode *)xget(list,MUIA_NListtree_Active);

	if (tree_node)
	{
		if (tree_node->tn_User)
		{
			struct folder *f = (struct folder*)tree_node->tn_User;
			set(str,MUIA_Text_Contents,f->name);
		}
	}
}

STATIC ASM LONG folder_strobj(register __a2 Object *list, register __a1 Object *str)
{
	char *s;
	struct folder *f;

	get(str,MUIA_Text_Contents,&s);
	
	search_refresh_folders();

  f = folder_find_by_name(s);
  if (f)
  {
		set(list,MUIA_NListtree_Active, FindListtreeUserData(search_folder_tree,f));
  }
	return 1;
}

#define MAKE_NULL(x) if ((x) && !*(x)) (x) = NULL;

/**************************************************************************
 Begin the search process
**************************************************************************/
static void searchwnd_start(void)
{
	struct search_options so;
	memset(&so,0,sizeof(so));
	so.folder = (char*)xget(search_folder_text,MUIA_Text_Contents);
	so.from = (char*)xget(search_from_string,MUIA_UTF8String_Contents);
	so.to = (char*)xget(search_to_string,MUIA_UTF8String_Contents);
	so.subject = (char*)xget(search_subject_string,MUIA_UTF8String_Contents);
	so.body = (char*)xget(search_body_string,MUIA_UTF8String_Contents);

	MAKE_NULL(so.folder);
	MAKE_NULL(so.from);
	MAKE_NULL(so.to);
	MAKE_NULL(so.subject);
	MAKE_NULL(so.body);

	callback_start_search(&so);
}

/**************************************************************************
 Init the search window
**************************************************************************/
static void init_search(void)
{
	static struct Hook folder_objstr_hook, folder_strobj_hook;

	init_hook(&folder_objstr_hook, (HOOKFUNC)folder_objstr);
	init_hook(&folder_strobj_hook, (HOOKFUNC)folder_strobj);

	search_wnd = WindowObject,
		MUIA_Window_ID, MAKE_ID('S','E','A','R'),
		MUIA_Window_Title, _("SimpleMail - Search"),
		WindowContents, VGroup,
			Child, ColGroup(2),
				Child, MakeLabel(_("Search _into")),
				Child, search_folder_popobject = PopobjectObject,
					MUIA_Popstring_Button, PopButton(MUII_PopUp),
					MUIA_Popstring_String, search_folder_text = TextObject, TextFrame, MUIA_Background, MUII_TextBack, End,
					MUIA_Popobject_ObjStrHook, &folder_objstr_hook,
					MUIA_Popobject_StrObjHook, &folder_strobj_hook,
					MUIA_Popobject_Object, NListviewObject,
							MUIA_NListview_NList, search_folder_tree = FolderTreelistObject,
							End,
						End,
					End,

				Child, MakeLabel(_("_From")),
				Child, search_from_string = UTF8StringObject,
					StringFrame,
					MUIA_CycleChain, 1,
					MUIA_String_AdvanceOnCR, TRUE,
					MUIA_UTF8String_Charset, user.config.default_codeset,
					End,

				Child, MakeLabel(_("_To")),
				Child, search_to_string = UTF8StringObject,
					StringFrame,
					MUIA_CycleChain, 1,
					MUIA_String_AdvanceOnCR, TRUE,
					MUIA_UTF8String_Charset, user.config.default_codeset,
					End,

				Child, MakeLabel(_("_Subject")),
				Child, search_subject_string = UTF8StringObject,
					StringFrame,
					MUIA_CycleChain, 1,
					MUIA_String_AdvanceOnCR, TRUE,
					MUIA_UTF8String_Charset, user.config.default_codeset,
					End,

				Child, MakeLabel(_("_Body")),
				Child, search_body_string = UTF8StringObject,
					StringFrame,
					MUIA_CycleChain, 1,
					MUIA_String_AdvanceOnCR, TRUE,
					MUIA_UTF8String_Charset, user.config.default_codeset,
					End,

				End,

			Child, HGroup,
				Child, search_start_button = MakeButton(_("S_tart search")),
				Child, search_stop_button = MakeButton(_("Sto_p search")),
				End,

			Child, NListviewObject,
				MUIA_NListview_NList, search_mail_tree = MailTreelistObject,
					End,
				End,
			End,
		End;

	if (search_wnd)
	{
		set(search_stop_button, MUIA_Disabled, TRUE);
		DoMethod(App, OM_ADDMEMBER, search_wnd);
		DoMethod(search_wnd, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, search_wnd, 3, MUIM_Set, MUIA_Window_Open, FALSE);
		DoMethod(search_folder_tree, MUIM_Notify, MUIA_NListtree_DoubleClick, MUIV_EveryTime, search_folder_popobject, 2, MUIM_Popstring_Close, TRUE);
		DoMethod(search_start_button, MUIM_Notify, MUIA_Pressed, FALSE, search_wnd, 3, MUIM_CallHook, &hook_standard, searchwnd_start);
		DoMethod(search_stop_button, MUIM_Notify, MUIA_Pressed, FALSE, search_wnd, 3, MUIM_CallHook, &hook_standard, callback_stop_search);
		search_refresh_folders();
	}
}

/**************************************************************************
 Refreshes the folders
**************************************************************************/
void search_refresh_folders(void)
{
	if (search_folder_tree) DoMethod(search_folder_tree, MUIM_FolderTreelist_Refresh,NULL);
}

/**************************************************************************
 Opens the search window
**************************************************************************/
void search_open(char *foldername)
{
	if (!search_wnd)
	{
		init_search();
		if (!search_wnd) return;
	}

	set(search_wnd, MUIA_Window_Open, TRUE);
	set(search_folder_text, MUIA_Text_Contents, foldername);
}

/**************************************************************************
 Opens the search window
**************************************************************************/
void search_clear_results(void)
{
	DoMethod(search_mail_tree, MUIM_NListtree_Clear, NULL, 0);
	has_mails = 0;
}

/**************************************************************************
 Opens the search window
**************************************************************************/
void search_add_result(struct mail **array, int size)
{
	int i;

	if (!size) return;

	if (size > 1)
		set(search_mail_tree, MUIA_NListtree_Quiet, TRUE);

	for (i=0;i<size;i++)
	{
		DoMethod(search_mail_tree,MUIM_NListtree_Insert,"" /*name*/, array[i], /*udata */
					 MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,0/*flags*/);
	}

	if (size > 1)
		set(search_mail_tree, MUIA_NListtree_Quiet, FALSE);

  has_mails = 1;
}

/**************************************************************************
 
**************************************************************************/
void search_enable_search(void)
{
	set(search_start_button,MUIA_Disabled,TRUE);
	set(search_stop_button,MUIA_Disabled,FALSE);
}

/**************************************************************************
 
**************************************************************************/
void search_disable_search(void)
{
	set(search_start_button,MUIA_Disabled,FALSE);
	set(search_stop_button,MUIA_Disabled,TRUE);
}

/**************************************************************************
 Returns wheather there are any mails or not
**************************************************************************/
int search_has_mails(void)
{
	return has_mails;
}

/**************************************************************************
 Remove the given mail
**************************************************************************/
void search_remove_mail(struct mail *m)
{
	struct MUI_NListtree_TreeNode *treenode = FindListtreeUserData(search_mail_tree, m);
	if (treenode)
		DoMethod(search_mail_tree, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, treenode,0);
}
