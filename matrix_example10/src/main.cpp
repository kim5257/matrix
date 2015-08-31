/*
 * @file		main.cpp
 * @author		asran
 * @created	2014. 12. 23.
 * @brief		matix.txt로부터 A 행렬 값을 읽고, B 행렬에는 sine vector를 입력한 후
 * 				Ax = B 방정식의 x 값을 구하는 연산을 수행한다.
 * 				이 때 각 데이터의 메모리 사용량과 방정식 해를 구하는데 소요 된 시간을 측정한다.
 */

#include <matrix_error.h>
#include <cmath>
#include <unistd.h>
#include <stdio.h>
#include "setting.h"
#include "test.h"

//#define	MEASURE_CG

#define	MATRIX_FILE_NAME		"matrix(100X100)_Cond(1.1942).txt"
#define	MATRIX_SIZE			(100)

#define	CG_LOOP				(1000)

matrix_t	sol_cg		(	const matrix_t&		matrixA,
							const matrix_t&		matrixB
						);

int		main	(	int		argc,
					char*	argv[]
				)
{
	try
	{
		size_t		initUsage	=	0;
		size_t		dataAUsage	=	0;
		size_t		dataBUsage	=	0;

		// 객체 선언
		printf("Matrix data file open - %s %s %s\n", argv[1], argv[2], argv[3]);
		FILE*	matrixFile		=	fopen(argv[1], "r");			// A data file
		size_t	matrixSize		=	atol(argv[2]);
		size_t	loopNum		=	atol(argv[3]);

		// matrix 데이터 파일 여는 데 실패하면 예외 던짐.
		if( matrixFile == NULL )
		{
			throw	matrix::ErrMsg::createErrMsg("파일 열기 실패");
		}

		initUsage	=	test::Test::calcMemUsage();
		printf("init  : %09lu\n", initUsage);

		matrix_t		matrixA(matrixSize, matrixSize);		// A data matrix

		// 데이터 넣기
		printf("Input matrix data from matrix data file...\n");
		for(size_t row=0;row<matrixSize;++row)
		{
			for(size_t col=0;col<matrixSize;++col)
			{
				double	value	=	0.0;

				if( fscanf(matrixFile, "%lf", &value) != 0 )
				{
					if( value != 0.0 )
					{
						matrixA.setElem(row, col, value);
					}
				}
			}
		}

		dataAUsage	=	test::Test::calcMemUsage() - initUsage;
		printf("data A size : %lu\n", matrixA.getSize());
		printf("dataA : %09lu\n", dataAUsage);

		matrix_t		matrixB(matrixSize, 1);					// B data matrix

		printf("Calculate sine data, and input this data in B matrix...\n");
		for(size_t row=0;row<matrixSize;++row)
		{
			double	value	=	-sin(double(row+1)*2*M_PI/(matrixSize+1));
			matrixB.setElem(row, 0, value);
		}

		dataBUsage	=	test::Test::calcMemUsage() - dataAUsage - initUsage;

		printf("data B size : %lu\n", matrixB.getSize());
		printf("dataB : %09lu\n", dataBUsage);
		// 연산
		printf("Calculate solution by using CG. - 1000\n");

		for(size_t loop=0;loop<loopNum;++loop)
		{
			printf("[%04lu]", loop);
			matrix_t		matrixX	=	sol_cg(matrixA, matrixB);		// x data matrix
		}
	}
	catch(	matrix::ErrMsg*	err	)
	{
		printf("%s", err->getErrString());
		matrix::ErrMsg::destroyErrMsg(err);
	}

	return	0;
}

#ifdef	MEASURE_CG

matrix_t	sol_cg		(	const matrix_t&		matrixA,
							const matrix_t&		matrixB
						)
{
	test::Test		testObj;

	testObj.startMeasure();

	size_t			cnt			=	0;
	matrix_t		x			=	matrix_t(matrixA.getCol(), matrixB.getCol());
	matrix_t		r			=	matrixB.sub(matrixA.multiply(x));
	matrix_t		p			=	r;
	matrix_t		rSold		=	r.stmultiply(r);
	matrix_t		result		=	x;
	matrix::elem_t	min			=	1000;
	bool	foundFlag	=	false;

	for(cnt=0;cnt<100000;cnt++)
	{
		testObj.startMeasure();
		matrix_t	ap		=	matrixA.multiply(p);
		testObj.endMeasure();
		printf("1. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		matrix::elem_t	ptval	=	(p.stmultiply(ap)).getElem(0,0);
		testObj.endMeasure();
		printf("2. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		matrix::elem_t	alpha	=	rSold.getElem(0,0) / ptval;
		testObj.endMeasure();
		printf("3. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		x	=	x.add(p.multiply(alpha));
		testObj.endMeasure();
		printf("4. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		r	=	r.sub(ap.multiply(alpha));
		testObj.endMeasure();
		printf("5. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		matrix_t	rsNew	=	r.stmultiply(r);
		testObj.endMeasure();
		printf("6. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		matrix::elem_t	sqrtVal	=	sqrt(rsNew.getElem(0,0));
		testObj.endMeasure();
		printf("7. ");
		testObj.ptrMeasure();

		if( min > sqrtVal )
		{
			min		=	sqrtVal;

			testObj.startMeasure();
			result	=	x;
			testObj.endMeasure();
			printf("8. ");
			testObj.ptrMeasure();
		}

		if( sqrtVal < 0.1e-16 )
		{
			foundFlag	=	true;
			break;
		}

		testObj.startMeasure();
		p		=	r.add( p.multiply( (rsNew.getElem(0,0) / rSold.getElem(0,0)) ) );
		testObj.endMeasure();
		printf("9. ");
		testObj.ptrMeasure();

		testObj.startMeasure();
		rSold	=	rsNew;
		testObj.endMeasure();
		printf("10. ");
		testObj.ptrMeasure();
	}

	testObj.endMeasure();

	printf("done - %lu\n", cnt);
	testObj.ptrMeasure();

	if( foundFlag != true )
	{
		x	=	result;
	}

	return	x;
}

#else

matrix_t	sol_cg		(	const matrix_t&		matrixA,
							const matrix_t&		matrixB
						)
{
	test::Test		testObj;

	testObj.startMeasure();

	size_t			cnt			=	0;
	matrix_t		x			=	matrix_t(matrixA.getCol(), matrixB.getCol());
	matrix_t		r			=	matrixB.sub(matrixA.multiply(x));
	matrix_t		p			=	r;
	matrix_t		rSold		=	r.stmultiply(r);
	matrix_t		result		=	x;
	matrix::elem_t	min			=	1000;
	bool	foundFlag	=	false;

	for(cnt=0;cnt<100000;cnt++)
	{
		matrix_t	ap		=	matrixA.multiply(p);
		matrix::elem_t	ptval	=	(p.stmultiply(ap)).getElem(0,0);

		matrix::elem_t	alpha	=	rSold.getElem(0,0) / ptval;

		x	=	x.add(p.multiply(alpha));
		r	=	r.sub(ap.multiply(alpha));

		matrix_t	rsNew	=	r.stmultiply(r);
		matrix::elem_t	sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.1e-16 )
		{
			foundFlag	=	true;
			break;
		}

		p		=	r.add( p.multiply( (rsNew.getElem(0,0) / rSold.getElem(0,0)) ) );
		rSold	=	rsNew;
	}

	if( foundFlag != true )
	{
		x	=	result;
	}

	testObj.endMeasure();
	printf(" cnt = %lu ", cnt);
	testObj.ptrMeasure();

	return	x;
}

#endif
