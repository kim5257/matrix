/*
 * test.cpp
 *
 *  Created on: 2015. 6. 16.
 *      Author: asran
 */

#include "test.h"
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>

namespace	test
{

#define	PROC_PATH		"/proc/%d/statm"
#define	STATM_FORMAT	"%lu %lu %lu %lu %lu %lu %lu"

size_t		Test::calcMemUsage	(	void	)
{
	size_t		memUsage	=	0;
	__pid_t	pid			=	getpid();
	char		path[PATH_MAX]	=	{0,};
	FILE*		file		=	NULL;

	size_t		size		=	0;
	size_t		resident	=	0;
	size_t		share		=	0;
	size_t		text		=	0;
	size_t		lib			=	0;
	size_t		data		=	0;
	size_t		dt			=	0;

	do
	{
		sprintf(path, PROC_PATH, pid);

		file	=	fopen(path, "r");
		if( file == NULL )
		{
			break;
		}

		if( fscanf(file, STATM_FORMAT, &size, &resident, &share, &text, &lib, &data, &dt) < 0 )
		{
			break;
		}

		memUsage	=	data * 4096;

	}while(0);

	return	memUsage;
}

};
