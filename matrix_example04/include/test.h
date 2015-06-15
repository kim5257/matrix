/*
 * test.h
 *
 *  Created on: 2015. 1. 14.
 *      Author: asran
 */

#ifndef TEST_H_
#define TEST_H_

#include <sys/time.h>
#include <time.h>
#include "setting.h"

namespace	test
{

class	Test
{
private:
	timeval		mStartTime;
	timeval		mEndTime;
	timeval		mDiffTime;
private:
	matrix_t		matrixA;
	matrix_t		matrixB1;
	matrix_t		matrixB2;
	matrix_t		matrixResult1;
	matrix_t		matrixResult2;
public:
				Test		(	void	);
	virtual	~Test		(	void	);
private:
	void	startMeasure	(	void	);
	void	endMeasure		(	void	);
	void	ptrMeasure		(	void	);
	void	initRandomVal	(	void	);
	matrix::elem_t
			getRandomVal	(	size_t		start,
								size_t		end
							);
public:
	void	inpData		(	void	);
	void	ptrDataSize	(	void	);
	void	testEqual		(	void	);
	void	testCompare	(	void	);
	void	testAdd		(	void	);
	void	testSub		(	void	);
	void	testMul		(	void	);
	void	testElmMul		(	void	);
	void	testTMul		(	void	);
	void	testStMul		(	void	);
};

};

#endif /* TEST_H_ */
