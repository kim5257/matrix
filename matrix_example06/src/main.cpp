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
		//test.testCompare();
		//test.testEqual();
		//test.testAdd();
		//test.testSub();
		test.testMul();
		//test.testElmMul();
		//test.testTMul();
		test.testStMul();
		//test.ptrDataSize();
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());
		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
