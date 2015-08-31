/*
 * test.h
 *
 *  Created on: 2015. 6. 16.
 *      Author: asran
 */

#ifndef INCLUDE_TEST_H_
#define INCLUDE_TEST_H_

#include <sys/time.h>
#include <time.h>
#include <stddef.h>

namespace test
{

class	Test
{
private:
	timeval		mStartTime;
	timeval		mEndTime;
	timeval		mDiffTime;
public:
				Test		(	void	);
	virtual	~Test		(	void	);
public:
	void	startMeasure	(	void	);
	void	endMeasure		(	void	);
	void	ptrMeasure		(	void	);
public:
	static size_t		calcMemUsage	(	void	);
};

};

#endif /* INCLUDE_TEST_H_ */
