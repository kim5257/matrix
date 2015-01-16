/*
 * main.cpp
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#include "test.h"

#define	COL_SIZE				(5000000)
#define	ROW_SIZE				(5000000)

#define	VAL_RANGE_START		(1)
#define	VAL_RANGE_END			(10)

#define	COL_PER_VAL			(7)

int		main	(	void	)
{
	try
	{
		test::Test		test;

		test.inpData();
		test.testCompare();
		test.testEqual();
		test.testAdd();
		test.testSub();
		test.testMul();
		test.testElmMul();
		test.testTMul();
		test.ptrDataSize();
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());
		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
