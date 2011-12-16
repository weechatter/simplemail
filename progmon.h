/**
 * @file progress.h
 */

#ifndef SM__PROGRESS_H
#define SM__PROGRESS_H

#ifndef SM__LISTS_H
#include "lists.h"
#endif

struct progmon
{
	/**
	 * Embedded node.
	 */
	struct node node;

	/**
	 * Notifies the progress monitor that a task is about
	 * to be started.
	 *
	 * @param work amount of work
	 * @param txt some useful text describing the task
	 */
	void (*begin)(unsigned int work, char *txt);

	/**
	 * Notifies the progess monitor that we are now
	 * working on the given specific problem.
	 * @param txt
	 */
	void (*working_on)(char *txt);

	/**
	 * Notifies the progess monitor that the given amount
	 * of work has been done.
	 *
	 * @param done
	 */
	void (*work)(unsigned int done);

	/**
	 * @return if the operation should be canceled or not.
	 */
	int (*canceled)(void);

	/**
	 * Notifies the progress monitor that the task
	 * has been completed.
	 */
	void (*done)(void);
};

struct progmon *progmon_create(void);
void progmon_delete(struct progmon *pm);

#endif
