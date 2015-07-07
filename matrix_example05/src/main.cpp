/*
 * @file		main.cpp
 * @author		asran
 * @created	2015. 06. 16.
 * @brief		희소행렬 객체의 메모리 사용량을 측정한다.
 */

#include <stdio.h>
#include <unistd.h>
#include <sparse_matrix2.h>
#include "test.h"

#define	ROW_SIZE		(5000000)
#define	COL_SIZE		(6)

int		main	(	int		argc,
					char*	argv[]
				)
{
	size_t		initMemUsage	=	0;
	size_t		dataMemUsage	=	0;
	size_t		freeMemUsage	=	0;

	printf("메모리 측정을 시작합니다\n");

	initMemUsage	=	test::Test::calcMemUsage();

	getchar();

	{
		matrix::SparseMatrix2*	matrixA	=	new matrix::SparseMatrix2(ROW_SIZE, COL_SIZE);

		for(size_t row=0;row<ROW_SIZE;++row)
		{
			for(size_t col=0;col<COL_SIZE;++col)
			{
				matrixA->setElem(row, col, 5);
			}
		}

		dataMemUsage	=	test::Test::calcMemUsage();

		getchar();

		delete	matrixA;
	}

	freeMemUsage	=	test::Test::calcMemUsage();

	printf("init : %09lu\n", initMemUsage);
	printf("data : %09lu\n", dataMemUsage);
	printf("free : %09lu\n", freeMemUsage);
	printf("allo : %09lu MB\n", (dataMemUsage - initMemUsage) / 1024 / 1024);

	return	0;
}
