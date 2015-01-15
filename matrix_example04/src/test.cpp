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

#define	COL_SIZE				(5000000)
#define	ROW_SIZE				(5000000)

#define	VAL_RANGE_START		(1)
#define	VAL_RANGE_END			(10)

#define	COL_PER_VAL			(7)

namespace	test
{

Test::Test			(	void	)
{
	matrixA	=	matrix::SparseMatrix(COL_SIZE, ROW_SIZE);
	matrixB1	=	matrix::SparseMatrix(COL_SIZE, ROW_SIZE);;
	matrixB2	=	matrix::SparseMatrix(COL_SIZE, 1);;
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

void	Test::initRandomVal		(	void	)
{
	srand(time(NULL));
}

matrix::elem_t
		Test::getRandomVal		(	size_t		start,
										size_t		end
									)
{
	matrix::elem_t	val		=	(matrix::elem_t)((rand() % (end - start + 1)) + start);

	return	val;
}

void	Test::inpData			(	void	)
{
	printf("\n\n"
			"데이터 입력\n"
			"==========\n");

	printf("행렬 A1에 데이터 입력 중...");
	fflush(stdout);

	startMeasure();
	// 랜덤 값으로 A 행렬에 값 넣기
	for(size_t cnt=0;cnt<matrixA.getCol();cnt++)
	{
		for(size_t cnt2=0;cnt2<COL_PER_VAL;cnt2++)
		{
			matrixA.setElem(cnt,getRandomVal(0,matrixA.getRow()-1),getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
		}
	}
	endMeasure();
	printf("완료\n");
	ptrMeasure();

	printf("행렬 B1에 데이터 입력 중...");
	fflush(stdout);

	startMeasure();
	// 랜덤 값으로 B1 행렬에 값 넣기
	for(size_t cnt=0;cnt<matrixB1.getCol();cnt++)
	{
		for(size_t cnt2=0;cnt2<COL_PER_VAL;cnt2++)
		{
			matrixB1.setElem(cnt,getRandomVal(0,matrixB1.getRow()-1),getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
		}
	}
	endMeasure();
	printf("완료\n");
	ptrMeasure();

	printf("행렬 B2에 데이터 입력 중...");
	fflush(stdout);

	startMeasure();
	// 랜덤 값으로 B2 행렬에 값 넣기
	for(size_t cnt=0;cnt<matrixB2.getCol();cnt++)
	{
		matrixB2.setElem(cnt,0,getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
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
			"%-15s : %ld\n"
			"%-15s : %ld\n",
			"matrixA",
			matrixA.getSize(),
			"matrixB1",
			matrixB1.getSize(),
			"matrixB2",
			matrixB2.getSize(),
			"matrixResult1",
			matrixResult1.getSize(),
			"matrixResult2",
			matrixResult2.getSize());
}

void	Test::testEqual		(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"대입 시험\n"
			"=========\n");

	printf("(1) 단일 쓰레드 대입 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1.equal(matrixA);

	endMeasure();

	if( matrixResult1 == matrixA )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("통과"):("문제 있음"));

	ptrMeasure();

	printf("(2) 멀티 쓰레드 대입 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1	=	matrixA;

	endMeasure();

	if( matrixResult1 == matrixA )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("통과"):("문제 있음"));

	ptrMeasure();
}

void	Test::testCompare		(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"비교 시험\n"
			"=========\n");
	printf("시험 준비 중...");
	fflush(stdout);

	// matrixResult에 matrixA 내용 넣기
	matrixResult1	=	matrixA;

	printf("완료\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	if( matrixResult1.compare(matrixA) )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	endMeasure();

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	if( matrixResult1 == matrixA )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	endMeasure();

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

void	Test::testAdd			(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"덧셈 시험\n"
			"=========\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.add(matrixB1);

	endMeasure();

	printf("완료\n");

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult2		=	matrixA + matrixB1;

	endMeasure();

	printf("완료\n");
	ptrMeasure();

	printf("결과 비교 중...");
	fflush(stdout);

	if( matrixResult1 == matrixResult2 )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

void	Test::testSub			(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"뺄셈 시험\n"
			"=========\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.sub(matrixB1);

	endMeasure();

	printf("완료\n");

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult2		=	matrixA - matrixB1;

	endMeasure();

	printf("완료\n");
	ptrMeasure();

	printf("결과 비교 중...");
	fflush(stdout);

	if( matrixResult1 == matrixResult2 )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

void	Test::testMul			(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"곱셈 시험\n"
			"=========\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.multiply(matrixB2);

	endMeasure();

	printf("완료\n");

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult2		=	matrixA * matrixB2;

	endMeasure();

	printf("완료\n");
	ptrMeasure();

	printf("결과 비교 중...");
	fflush(stdout);

	if( matrixResult1 == matrixResult2 )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

void	Test::testElmMul		(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"행렬 * 상수 곱셈 시험\n"
			"===================\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.multiply(2);

	endMeasure();

	printf("완료\n");

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult2		=	matrixA * 2;

	endMeasure();

	printf("완료\n");
	ptrMeasure();

	printf("결과 비교 중...");
	fflush(stdout);

	if( matrixResult1 == matrixResult2 )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

void	Test::testTMul		(	void	)
{
	bool	flag	=	false;

	printf("\n\n"
			"전치 행렬 곱셈 시험\n"
			"==================\n");

	printf("(1) 단일 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult1		=	matrixA.tmultiply(matrixB2);

	endMeasure();

	printf("완료\n");

	ptrMeasure();

	printf("(2) 멀티 쓰레드 동작 시험...");
	fflush(stdout);

	startMeasure();

	matrixResult2		=	matrixA.ptmultiply(matrixB2);

	endMeasure();

	printf("완료\n");
	ptrMeasure();

	printf("결과 비교 중...");
	fflush(stdout);

	if( matrixResult1 == matrixResult2 )
	{
		flag	=	true;
	}
	else
	{
		flag	=	false;
	}

	printf("완료\n"
			"결과: %s\n",
			(flag)?("일치함"):("불 일치함"));

	ptrMeasure();
}

};
