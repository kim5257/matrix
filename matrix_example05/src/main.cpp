/*
 * main.cpp
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#include "test.h"

int		main	(	void	)
{
	try
	{
		test::Test		test;

		test.inpData();
		test.ptrDataSize();
		test.testSolution();
		test.ptrResult();
		//test.ptrDataSize();
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());
		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
