/*
 * @file		main.cpp
 * @author		asran
 * @created	2014. 12. 26.
 * @brief		희소행렬 클래스를 사용하여 기본 연산을 수행하고,\n
 * 				수행시간을 측정하는 예제프로그램이다.
 */

#include "test.h"

int		main	(	void	)
{
	try
	{
		test::Test		test;

		test.inpData();
		test.ptrDataSize();
		test.testCompare();
		test.testEqual();
		test.testAdd();
		test.testSub();
		test.testMul();
		test.testElmMul();
		//test.testTMul();
		test.testStMul();
		test.ptrDataSize();
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());
		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
