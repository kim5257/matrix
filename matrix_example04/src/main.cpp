/*
 * main.cpp
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <stdio.h>

int		main	(	int		argc,
					char*	argv[]
				)
{
	try
	{
		matrix::SparseMatrix	matrixA	=	matrix::SparseMatrix(4,4);
		matrix::SparseMatrix	matrixB	=	matrix::SparseMatrix(4,1);

		matrixA.setElem(0,0,999);
		matrixA.setElem(0,1,0);
		matrixA.setElem(1,0,100);
		matrixA.setElem(1,1,6);

		matrixB.setElem(0,0,15);
		matrixB.setElem(1,0,16);

		matrix::SparseMatrix	matrixC	=	matrixA * matrixB;

		printf("A = \n");
		for(size_t col=0;col<matrixA.getCol();col++)
		{
			for(size_t row=0;row<matrixA.getRow();row++)
			{
				printf("%3.5f ", matrixA.getElem(col, row));
			}
			printf("\n");
		}

		printf("b = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%3.5f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrix::SparseMatrix	result		=	matrixA.solution(matrixC);

		printf("result = \n");
		for(size_t col=0;col<result.getCol();col++)
		{
			for(size_t row=0;row<result.getRow();row++)
			{
				printf("%3.5f ", result.getElem(col, row));
			}
			printf("\n");
		}

		matrixB	=	matrixA * result;

		printf("check = \n");
		for(size_t col=0;col<matrixB.getCol();col++)
		{
			for(size_t row=0;row<matrixB.getRow();row++)
			{
				printf("%3.5f ", matrixB.getElem(col, row));
			}
			printf("\n");
		}
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());

		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
