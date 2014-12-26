/*
 * main.cpp
 *
 *  Created on: 2014. 12. 23.
 *      Author: asran
 */

#include "matrix.h"
#include <stdio.h>

int		main	(	int		argc,
					char*	argv[]
				)
{
	Matrix*	matrixA		=	Matrix::createMatrix(3, 3);
	Matrix*	matrixC		=	Matrix::createMatrix(3, 3);

	for(size_t col=0;col<matrixA->getCol();col++)
	{
		for(size_t row=0;row<matrixA->getRow();row++)
		{
			matrixA->setValue(col, row, (col * matrixA->getRow()) + row);
		}
	}

	for(size_t col=0;col<matrixA->getCol();col++)
	{
		for(size_t row=0;row<matrixA->getRow();row++)
		{
			printf("%3d ", matrixA->getValue(col, row));
		}
		printf("\n");
	}

	printf("\n");

	Matrix*	matrixB	=	Matrix::createMatrix(matrixA);

	for(size_t col=0;col<matrixB->getCol();col++)
	{
		for(size_t row=0;row<matrixB->getRow();row++)
		{
			printf("%3d ", matrixB->getValue(col, row));
		}
		printf("\n");
	}

	printf("\n");

	matrixA->transpose(matrixC);

	for(size_t col=0;col<matrixC->getCol();col++)
	{
		for(size_t row=0;row<matrixC->getRow();row++)
		{
			printf("%3d ", matrixC->getValue(col, row));
		}
		printf("\n");
	}

	Matrix::destroyMatrix(matrixA);
	Matrix::destroyMatrix(matrixB);
	Matrix::destroyMatrix(matrixC);

	return	0;
}
