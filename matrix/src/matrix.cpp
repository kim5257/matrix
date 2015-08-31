/*
 * matrix.cpp
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#include "matrix.h"
#include "matrix_error.h"
#include <string.h>
#include <new>
#include <math.h>

namespace matrix
{

/**
 * 생성자
 */
Matrix::Matrix		(	void	)
:mRowSize(0),
 mColSize(0),
mData(NULL)
{
}

/**
 * 생성자
 */
Matrix::Matrix		(	size_t		row,	///< 행 크기
						size_t		col		///< 열 크기
					)
{
	allocElems(row, col);
}

/**
 * 복사 생성자
 */
Matrix::Matrix		(	const Matrix&		matrix		///< 복사 될 객체
					)
{
	allocElems(matrix.getRow(), matrix.getCol());
	copyElems(matrix);
}

/**
 * 소멸자
 */
Matrix::~Matrix		(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return 참조한 행렬 요소 값
 */
elem_t		Matrix::getElem		(	size_t		row,	///< 참조 할 행 위치
									size_t		col		///< 참조 할 열 위치
								) const
{
	//chkBound(row, col);
	return	mData[ row * getCol() + col ];
}

/**
 * 행렬 요소 값 설정
 */
void		Matrix::setElem		(	size_t		row,	///< 설정 할 행 위치
									size_t		col,	///< 설정 할 열 위치
									elem_t		elem	///< 설정 할 요소 값
								)
{
	//chkBound(row, col);
	mData[ row * getCol() + col ]	=	elem;
}

/**
 * 행렬 덧셈
 * @return	행렬 덧셈 결과
 */
Matrix		Matrix::add				(	const Matrix&	operand	///< 피연산자
									) const
{
	chkSameSize(operand);

	Matrix	result		=	Matrix(getRow(), getCol());

	for(size_t row=0;row<getRow();row++)
	{
		for(size_t col=0;col<getCol();col++)
		{
			result.setElem	(	row,
								col,
								getElem(row, col) + operand.getElem(row, col)
							);
		}
	}

	return	result;
}

/**
 * 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
Matrix		Matrix::sub				(	const Matrix&	operand	///< 피연산자
									) const
{
	chkSameSize(operand);

	Matrix	result		=	Matrix(getRow(), getCol());

	for(size_t row=0;row<getRow();row++)
	{
		for(size_t col=0;col<getCol();col++)
		{
			result.setElem	(	row,
									col,
									getElem(row, col) - operand.getElem(row, col)
								);
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
Matrix		Matrix::multiply		(	const Matrix&	operand	///< 피연산자
									) const
{
	if( ( getRow() != operand.getCol() ) &&
		( getCol() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	Matrix	result		=	Matrix(getRow(), operand.getCol());

	for(size_t row=0;row<result.getRow();row++)
	{
		for(size_t col=0;col<result.getCol();col++)
		{
			elem_t	 value	=	0;
			for(size_t x=0;x<getCol();x++)
			{
				value	+=	getElem(row, x) * operand.getElem(x, col);
			}
			result.setElem(row, col, value);
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
Matrix		Matrix::multiply		(	elem_t		operand	///< 피연산자
									) const
{
	Matrix	result		=	Matrix(getRow(), getCol());

	for(size_t row=0;row<getRow();row++)
	{
		for(size_t col=0;col<getCol();col++)
		{
			result.setElem	(	row,
									col,
									getElem(row, col) * operand
								);
		}
	}

	return	result;
}

/**
 * 전치 행렬 변환
 * @return 전치 행렬 변환 결과
 */
Matrix		Matrix::transpose		(	void	) const
{
	Matrix	result		=	Matrix(getCol(), getRow());

	for(size_t row=0;row<getRow();row++)
	{
		for(size_t col=0;col<getCol();col++)
		{
			result.setElem	(	col,
									row,
									getElem(row, col)
								);
		}
	}

	return	result;
}

Matrix		Matrix::stmultiply		(	const Matrix&	operand	) const
{
	Matrix	result		=	this->transpose() * operand;

	return	result;
}

/**
 * 행렬 대입
 * @return 대입 할 행렬
 */
const Matrix&		Matrix::equal			(	const Matrix&	operand	///< 피연산자
											)
{
	try
	{
		chkSameSize(operand);
		copyElems(operand);
	}
	catch( ErrMsg*	)
	{
		freeElems();
		allocElems(operand.getRow(), operand.getCol());
		copyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 방정식 해 구하기
 * @return		해 행렬 리턴
 */
Matrix		Matrix::sol_cg		(	const Matrix&	operand	///< 피연산자
									)
{
	Matrix		x			=	Matrix(this->getCol(), operand.getCol());
	Matrix		r			=	operand - ( (*this) * x );
	Matrix		p			=	r;
	Matrix		rSold		=	r.transpose() * r;
	Matrix		result		=	x;
	elem_t		min			=	1;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<1000000;cnt++)
	{
		Matrix		ap			=	(*this) * p;
		elem_t		alpha		=	rSold.getElem(0,0) / (p.transpose() * ap).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		Matrix		rsNew		=	r.transpose() * r;

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.0001 )
		{
			foundFlag	=	true;
			break;
		}

		p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		rSold	=	rsNew;
	}

	if( foundFlag != true )
	{
		x	=	result;
	}

	return	x;
}

/**
 * 행렬 데이터 공간 할당
 * @exception 메모리 할당 실패 시 에러 발생
 */
void		Matrix::allocElems			(	size_t		row,	///< 행 크기
											size_t		col		///< 열 크기
										)
{
	try
	{
		mData	=	new elem_t[row * col];
		mRowSize	=	row;
		mColSize	=	col;
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		Matrix::freeElems			(	void	)
{
	delete[]	mData;
	mRowSize	=	0;
	mColSize	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		Matrix::copyElems			(	const Matrix&		matrix		///< 복사 할 행렬
										)
{
	size_t		length		=	matrix.getRow()
							*	matrix.getCol()
							*	sizeof(elem_t);

	memcpy(mData, matrix.mData, length);
}

/**
 * 같은 크기의 행렬인지 검사
 * @exception 행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		Matrix::chkSameSize		(	const Matrix&		matrix		///< 비교 할 행렬
										) const
{
	if( ( getRow() != matrix.getRow() ) ||
		( getCol() != matrix.getCol() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

/**
 * 행렬 요소 참조 범위 검사
 * @exception 참조 범위 밖일 경우 예외 발생
 */
void		Matrix::chkBound			(	size_t		row,	///< 참조 할 행 위치
											size_t		col		///< 참조 할 열 위치
										) const
{
	if( ( row >= mRowSize ) ||
		( col >= mColSize ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

};
