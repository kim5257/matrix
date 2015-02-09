/*
 * test.cpp
 *
 *  Created on: 2015. 1. 14.
 *      Author: asran
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

namespace	test
{

Test::Test			(	void	)
{
	matrixA	=	matrix_t(COL_SIZE, ROW_SIZE);
	matrixB	=	matrix_t(COL_SIZE, 1);;
}

Test::~Test		(	void	)
{

}

void	Test::startMeasure	(	void	)
{
	gettimeofday(&mStartTime, NULL);
}

void	Test::endMeasure		(	void	)
{
	gettimeofday(&mEndTime, NULL);
	timersub(&mEndTime, &mStartTime, &mDiffTime);
}

void	Test::ptrMeasure		(	void	)
{
	printf("걸린시간 - %ld:%06ld\n\n", mDiffTime.tv_sec, mDiffTime.tv_usec);
}

void	Test::inpData			(	void	)
{
	const matrix::elem_t		data1	=	0.0051634;
	const matrix::elem_t		data2	=	-0.5;

	printf("\n\n"
			"데이터 입력\n"
			"==========\n");

	printf("행렬 A1에 데이터 입력 중...");
	fflush(stdout);

	startMeasure();

	for(size_t col=0;col<matrixA.getCol();++col)
	{
		for(size_t row=0;row<matrixA.getRow();++row)
		{
			matrix::elem_t	val		=	0;

			if( col == row )
			{
				val		=	data2;
			}
			else
			{
				val		=	data1;
			}

			matrixA.setElem(col, row, val);
		}
	}

	endMeasure();
	printf("완료\n");
	ptrMeasure();

	printf("행렬 B에 데이터 입력 중...");
	fflush(stdout);

	startMeasure();

	for(size_t col=0;col<matrixA.getCol();++col)
	{
		matrix::elem_t	val		=	-sin((double)(col + 1) * 2 * M_PI / (matrixB.getCol()+1));

		matrixB.setElem(col,0, val);
	}

	endMeasure();
	printf("완료\n");
	ptrMeasure();
}

void	Test::ptrDataSize		(	void	)
{
	printf("\n\n"
			"데이터 크기\n"
			"===========\n");

	printf("%-15s : %ld\n"
			"%-15s : %ld\n"
			"%-15s : %ld\n"
			"%-15s : %ld\n",
			"matrixA",
			matrixA.getSize(),
			"matrixB",
			matrixB.getSize(),
			"matrixResult1",
			matrixResult1.getSize(),
			"matrixResult2",
			matrixResult2.getSize());
}

void	Test::ptrResult		(	void	)
{
	for(size_t col=0;col<matrixResult1.getCol();++col)
	{
		for(size_t row=0;row<matrixResult1.getRow();++row)
		{
			//printf("[%lu,%lu]\t%lf\n", col, row, matrixResult1.getElem(col,row));
		}
	}

}

void	Test::testSolution	(	void	)
{
	printf("\n\n"
			"행렬 해 연산 시험\n"
			"===============\n");

	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.solution(matrixB);

	endMeasure();

	printf("완료\n");

	ptrMeasure();
}

};
