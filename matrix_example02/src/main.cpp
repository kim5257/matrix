/*
 * main.cpp
 *
 *  Created on: 2014. 12. 23.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_csr.h"
#include "matrix_error.h"
#include <stdio.h>

int		main	(	void	)
{
	try
	{
		matrix::MatrixCSR	matrixA	=	matrix::MatrixCSR(4,4);
		matrix::MatrixCSR	matrixB	=	matrix::MatrixCSR(4,4);
		matrix::MatrixCSR	matrixC;

		// A 행렬 데이터 넣기
		matrixA.setElem(0,0,1);
		matrixA.setElem(0,1,2);
		matrixA.setElem(0,2,3);
		matrixA.setElem(0,3,4);
		matrixA.setElem(1,0,5);
		matrixA.setElem(1,1,6);
		matrixA.setElem(1,2,7);
		matrixA.setElem(1,3,8);
		matrixA.setElem(2,0,9);
		matrixA.setElem(2,1,10);
		matrixA.setElem(2,2,11);
		matrixA.setElem(2,3,12);
		matrixA.setElem(3,0,13);
		matrixA.setElem(3,1,14);
		matrixA.setElem(3,2,15);
		matrixA.setElem(3,3,16);

		// B 행렬 데이터 넣기
		matrixB.setElem(0,0,16);
		matrixB.setElem(0,1,15);
		matrixB.setElem(0,2,14);
		matrixB.setElem(0,3,13);
		matrixB.setElem(1,0,12);
		matrixB.setElem(1,1,11);
		matrixB.setElem(1,2,10);
		matrixB.setElem(1,3,9);
		matrixB.setElem(2,0,8);
		matrixB.setElem(2,1,7);
		matrixB.setElem(2,2,6);
		matrixB.setElem(2,3,5);
		matrixB.setElem(3,0,4);
		matrixB.setElem(3,1,3);
		matrixB.setElem(3,2,2);
		matrixB.setElem(3,3,1);

		printf("A = \n");
		for(size_t col=0;col<matrixA.getCol();col++)
		{
			for(size_t row=0;row<matrixA.getRow();row++)
			{
				printf("%6.2f ", matrixA.getElem(col, row));
			}
			printf("\n");
		}

		printf("B = \n");
		for(size_t col=0;col<matrixB.getCol();col++)
		{
			for(size_t row=0;row<matrixB.getRow();row++)
			{
				printf("%6.2f ", matrixB.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA + matrixB;

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA - matrixB;

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA.multiply(matrixB);

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA.multiply(2);

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA.tmultiply(matrixB);

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}
	}
	catch( matrix::ErrMsg*	exception	)
	{
		printf("Exception: %s\n", exception->getErrString());

		matrix::ErrMsg::destroyErrMsg(exception);
	}


	return	0;
}
