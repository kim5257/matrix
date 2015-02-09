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
#include <setting.h>

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
	matrix_t		matrixB;
	matrix_t		matrixResult1;
	matrix_t		matrixResult2;
public:
				Test		(	void	);
	virtual	~Test		(	void	);
private:
	void	startMeasure	(	void	);
	void	endMeasure		(	void	);
	void	ptrMeasure		(	void	);
public:
	void	inpData		(	void	);
	void	ptrDataSize	(	void	);
	void	ptrResult		(	void	);
	void	testSolution	(	void	);
};

};

#endif /* TEST_H_ */
