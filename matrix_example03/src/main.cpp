/*
 * main.cpp
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define	COL_SIZE				(5000000)
#define	ROW_SIZE				(5000000)

#define	VAL_RANGE_START		(1)
#define	VAL_RANGE_END			(10)

#define	COL_PER_VAL			(6)

void	initRandomVal		(	void	)
{
	srand(time(NULL));
}

matrix::elem_t	getRandomVal		(	size_t		start,
											size_t		end
										)
{
	matrix::elem_t	val		=	(matrix::elem_t)((rand() % (end - start + 1)) + start);

	return	val;
}

int		main	(	int		argc,
					char*	argv[]
				)
{
	try
	{
		matrix::SparseMatrix		matrixA	=	matrix::SparseMatrix(COL_SIZE,ROW_SIZE);
		matrix::SparseMatrix		matrixComp;
		matrix::SparseMatrix		matrixB	=	matrix::SparseMatrix(COL_SIZE,1);

		timeval	startTime;
		timeval	endTime;
		timeval	diffTime;

		initRandomVal();

		printf("데이터 입력중...\n");

		gettimeofday(&startTime, NULL);

		// 랜덤 값으로 대각선 열에 값 넣기
		for(size_t cnt=0;cnt<matrixA.getCol();cnt++)
		{
			matrixA.setElem(cnt,cnt,getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
		}

		// 랜덤 값으로 나머지 열에 값 넣기
		for(size_t cnt=0;cnt<matrixA.getCol();cnt++)
		{
			for(size_t cnt2=0;cnt2<COL_PER_VAL;cnt2++)
			{
				matrixA.setElem(cnt,getRandomVal(0,matrixA.getRow()-1),getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
			}
		}

		// 랜덤 값으로 B 행렬에 값 넣기
		for(size_t cnt=0;cnt<matrixB.getCol();cnt++)
		{
			matrixB.setElem(cnt,0,getRandomVal(VAL_RANGE_START, VAL_RANGE_END));
		}

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);

		printf("대입 중...\n");

		gettimeofday(&startTime, NULL);

		matrixComp		=	matrixA;

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);

		printf("비교 중...\n");

		gettimeofday(&startTime, NULL);

		if( matrixComp == matrixA )
		{
			printf("비교 결과 일치\n");
		}
		else
		{
			printf("비교 결과 불일치\n");
		}

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);

		printf("matrixA 크기: %ld\n"
				"matrixB 크기: %ld\n",
				matrixA.getSize(),
				matrixB.getSize());

		printf("곱셈 중...\n");

		gettimeofday(&startTime, NULL);

		matrix::SparseMatrix		matrixC1	=	matrixA.multiply(matrixB);

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);

		printf("쓰레드 곱셈 중...\n");

		gettimeofday(&startTime, NULL);

		matrix::SparseMatrix		matrixC2	=	matrixA * matrixB;

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);

		printf("비교 중...\n");

		gettimeofday(&startTime, NULL);

		if( matrixC1 == matrixC2 )
		{
			printf("비교 결과 일치\n");
		}
		else
		{
			printf("비교 결과 불일치\n");
		}

		gettimeofday(&endTime, NULL);
		timersub(&endTime, &startTime, &diffTime);

		printf("완료 - %ld:%06ld\n", diffTime.tv_sec, diffTime.tv_usec);
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());

		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
