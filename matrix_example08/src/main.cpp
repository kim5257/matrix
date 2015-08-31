/*
 * main.cpp
 *
 *  Created on: 2015. 7. 15.
 *      Author: asran
 */

#include <sparse_matrix.h>
#include <matrix_error.h>
#include <string.h>
#include <stdio.h>
#include <cmath>

#define	MATRIX_FILE_NAME		"matrix(1000)_Cond(5.2746E04).txt"
#define	MATRIX_SIZE			(100)

int		main	(	int		argc,
					char*	argv[]
				)
{
	try
	{
		FILE*	matrixFile		=	fopen(MATRIX_FILE_NAME, "r");

		matrix::SparseMatrix		matrixA(MATRIX_SIZE, MATRIX_SIZE);
		matrix::SparseMatrix		matrixB(MATRIX_SIZE, 1);
		matrix::SparseMatrix		matrixX;
		matrix::SparseMatrix		matrixC;

		if( matrixFile == NULL )
		{
			throw	matrix::ErrMsg::createErrMsg("파일 열기 실패");
		}

		for(size_t row=0;row<MATRIX_SIZE;++row)
		{
			for(size_t col=0;col<MATRIX_SIZE;++col)
			{
				double	value	=	0.0;

				fscanf(matrixFile, "%lf", &value);
				if( value != 0.0 )
				{
					matrixA.setElem(row, col, value);
				}
			}
		}
		printf("Read done.\n");

		for(size_t row=0;row<MATRIX_SIZE;++row)
		{
			double	value	=	-sin(double(row+1)*2*M_PI/(MATRIX_SIZE+1));
			matrixB.setElem(row, 0, value);
		}

		printf("Make matrixB done.\n");

		matrixX	=	matrixA.sol_cg(matrixB);

		printf("Calc CG done\n");

		for(size_t row=0;row<MATRIX_SIZE;++row)
		{
			printf("%0.4lf\n", matrixX.getElem(row, 0));
		}

		printf("MatrixC = MatrixA * MatrixX\n");
		matrixC	=	matrixA * matrixX;
		for(size_t row=0;row<MATRIX_SIZE;++row)
		{
			printf("%0.4lf\n", matrixC.getElem(row, 0));
		}

		printf("MatrixB\n");
		for(size_t row=0;row<MATRIX_SIZE;++row)
		{
			printf("%0.4lf\n", matrixB.getElem(row, 0));
		}

		if( matrixB == matrixC )
		{
			printf("match\n");
		}
		else
		{
			printf("not match\n");
		}
	}
	catch(	matrix::ErrMsg*	err	)
	{
		printf("%s", err->getErrString());
		matrix::ErrMsg::destroyErrMsg(err);
	}

	return	0;
}


