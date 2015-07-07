/*
 * @file		main.cpp
 * @author		asran
 * @created	2014. 12. 23.
 * @brief		SparseMatrix 클래스를 사용하여 4x4 행렬을 생성하고\n
 * 				기본 연산을 수행하는 예제 프로그램이다.
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <stdio.h>

int		main	(	void	)
{
	try
	{
		matrix::SparseMatrix	matrixA	=	matrix::SparseMatrix(4,4);
		matrix::SparseMatrix	matrixB	=	matrix::SparseMatrix(4,4);
		matrix::SparseMatrix	matrixD	=	matrix::SparseMatrix(4,4);
		matrix::SparseMatrix	matrixC;

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

		// B 행렬 데이터 넣기
		matrixD.setElem(0,0,16);
		matrixD.setElem(1,0,15);
		matrixD.setElem(2,0,14);
		matrixD.setElem(3,0,13);
		matrixD.setElem(0,1,12);
		matrixD.setElem(1,1,11);
		matrixD.setElem(2,1,10);
		matrixD.setElem(3,1,9);
		matrixD.setElem(0,2,8);
		matrixD.setElem(1,2,7);
		matrixD.setElem(2,2,6);
		matrixD.setElem(3,2,5);
		matrixD.setElem(0,3,4);
		matrixD.setElem(1,3,3);
		matrixD.setElem(2,3,2);
		matrixD.setElem(3,3,1);

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

		printf("C = A + B\n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA - matrixB;

		printf("C = A - B\n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA * matrixB;

		printf("C = A * B\n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA * 2;

		printf("C = A * 2\n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA.ptmultiply(matrixB);

		printf("C = A * B^-1\n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%6.2f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		matrixC	=	matrixA.stmultiply(matrixB);

		printf("C = A^-1 * B\n");
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
