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
		matrix::SparseMatrix		matrixA	=	matrix::SparseMatrix(100,100);
		matrix::SparseMatrix		matrixB	=	matrix::SparseMatrix(100,100);

		matrixA.setElem(0,0,1);
		matrixA.setElem(0,1,2);
		matrixA.setElem(1,0,8);
		matrixA.setElem(1,1,9);

		matrixB.setElem(2,0,1);
		matrixB.setElem(2,1,2);
		matrixB.setElem(3,0,8);
		matrixB.setElem(3,1,9);

		matrix::SparseMatrix		matrixC	=	matrixA.sub(matrixB);

		printf("C = \n");
		for(size_t col=0;col<matrixC.getCol();col++)
		{
			for(size_t row=0;row<matrixC.getRow();row++)
			{
				printf("%3.5f ", matrixC.getElem(col, row));
			}
			printf("\n");
		}

		printf("A size = %ld\n", matrixA.getSize());
		printf("B size = %ld\n", matrixB.getSize());
		printf("C size = %ld\n", matrixC.getSize());
	}
	catch( matrix::ErrMsg*	exception	)
	{
		fprintf(stderr, "%s\n", exception->getErrString());

		matrix::ErrMsg::destroyErrMsg(exception);
	}

	return	0;
}
