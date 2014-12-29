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

Matrix::Matrix		(	void	)
:mData(NULL),
 mCol(0),
 mRow(0)
{
	//fprintf(stdout, "[%p] create matrix object.\n", this);
}

Matrix::Matrix		(	size_t		col,
							size_t		row
						)
:mData(NULL),
 mCol(0),
 mRow(0)
{
	//fprintf(stdout, "[%p] create matrix object.\n", this);
	allocElems(col, row);
}

Matrix::Matrix		(	const Matrix&		matrix		)
{
	//fprintf(stdout, "[%p] create matrix object.\n", this);

	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

Matrix::~Matrix		(	void	)
{
	//fprintf(stdout, "[%p] delete matrix object.\n", this);
	freeElems();
}

elem_t		Matrix::getElem		(	size_t		col,
										size_t		row
									) const
{
	chkBound(col, row);
	return	mData[ col * getRow() + row ];
}

void		Matrix::setElem		(	size_t		col,
										size_t		row,
										elem_t		elem
									)
{
	chkBound(col, row);
	mData[ col * getRow() + row ]	=	elem;
}

Matrix		Matrix::add				(	const Matrix&	operand	) const
{
	chkSameSize(operand);

	Matrix	result		=	Matrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result.setElem	(	col,
									row,
									getElem(col, row) + operand.getElem(col, row)
								);
		}
	}

	return	result;
}

Matrix		Matrix::sub				(	const Matrix&	operand	) const
{
	chkSameSize(operand);

	Matrix	result		=	Matrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result.setElem	(	col,
									row,
									getElem(col, row) - operand.getElem(col, row)
								);
		}
	}

	return	result;
}

Matrix		Matrix::multiply			(	const Matrix&	operand	) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	Matrix	result		=	Matrix(getCol(), operand.getRow());

	for(size_t col=0;col<result.getCol();col++)
	{
		for(size_t row=0;row<result.getRow();row++)
		{
			elem_t	 value	=	0;
			for(size_t x=0;x<getRow();x++)
			{
				value	+=	getElem(col, x) * operand.getElem(x, row);
			}
			result.setElem(col, row, value);
		}
	}

	return	result;
}

Matrix		Matrix::multiply			(	elem_t		operand	) const
{
	Matrix	result		=	Matrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result.setElem	(	col,
									row,
									getElem(col, row) * operand
								);
		}
	}

	return	result;
}

Matrix		Matrix::transpose			(	void	) const
{
	Matrix	result		=	Matrix(getRow(), getCol());

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result.setElem	(	row,
									col,
									getElem(col, row)
								);
		}
	}

	return	result;
}

const Matrix&		Matrix::equal			(	const Matrix&	operand	)
{
	chkSameSize(operand);
	copyElems(operand);

	return	*this;
}

Matrix		Matrix::solution		(	const Matrix&	operand	)
{
	Matrix		x			=	Matrix(this->getRow(), operand.getRow());
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

void		Matrix::allocElems		(	size_t		col,
											size_t		row
										)
{
	try
	{
		mData	=	new elem_t[col * row];
		mCol	=	col;
		mRow	=	row;
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

void		Matrix::freeElems			(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

void		Matrix::copyElems			(	const Matrix&		matrix		)
{
	size_t		length		=	matrix.getCol()
							*	matrix.getRow()
							*	sizeof(elem_t);

	memcpy(mData, matrix.mData, length);
}

void		Matrix::chkSameSize		(	const Matrix&		matrix		) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

void		Matrix::chkBound			(	size_t		col,
											size_t		row
										) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

};
