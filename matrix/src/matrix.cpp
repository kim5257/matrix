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
 * ����
 */
Matrix::Matrix		(	void	)
:mCol(0),
mRow(0),
mData(NULL)
{
}

/**
 * ����
 */
Matrix::Matrix		(	size_t		col,	///< �� ũ��
							size_t		row		///< �� ũ��
						)
{
	allocElems(col, row);
}

/**
 * ���� ����
 */
Matrix::Matrix		(	const Matrix&		matrix		///< ���� �� ��ü
						)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * �Ҹ���
 */
Matrix::~Matrix		(	void	)
{
	freeElems();
}

/**
 * ��� ��� �� ����
 * @return		������ ��� ��� ��
 */
elem_t		Matrix::getElem		(	size_t		col,	///< ���� �� �� ��ġ
										size_t		row		///< ���� �� �� ��ġ
									) const
{
	chkBound(col, row);
	return	mData[ col * getRow() + row ];
}

/**
 * ��� ��� �� ����
 */
void		Matrix::setElem		(	size_t		col,	///< ���� �� �� ��ġ
										size_t		row,	///< ���� �� �� ��ġ
										elem_t		elem	///< ���� �� ��� ��
									)
{
	chkBound(col, row);
	mData[ col * getRow() + row ]	=	elem;
}

/**
 * ��� ����
 * @return		��� ���� ���
 */
Matrix		Matrix::add				(	const Matrix&	operand	///< �ǿ�����
										) const
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

/**
 * ��� ����
 * @return		��� ���� ���
 */
Matrix		Matrix::sub				(	const Matrix&	operand	///< �ǿ�����
										) const
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

/**
 * ��� ����
 * @return		��� ���� ���
 */
Matrix		Matrix::multiply			(	const Matrix&	operand	///< �ǿ�����
										) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
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

/**
 * ��� ����
 * @return		��� ���� ���
 */
Matrix		Matrix::multiply			(	elem_t		operand	///< �ǿ�����
										) const
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

/**
 * ��ġ ��� ��ȯ
 * @return		��ȯ ���
 */
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

/**
 * ��� ����
 * @return		���� �� ���
 */
const Matrix&		Matrix::equal			(	const Matrix&	operand	///< �ǿ�����
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
		allocElems(operand.getCol(), operand.getRow());
		copyElems(operand);
	}

	return	*this;
}

/**
 * ��� ������ �� ���
 * @return		�� ��� ���
 */
Matrix		Matrix::solution		(	const Matrix&	operand	///< �ǿ�����
									)
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

/**
 * ��� ������ �� �Ҵ�
 * @exception		�޸� �Ҵ� ���� �� ���� �߻�
 */
void		Matrix::allocElems		(	size_t		col,	///< �� ũ��
											size_t		row		///< �� ũ��
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

/**
 * ��� ������ �� �Ҵ� ����
 */
void		Matrix::freeElems			(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

/**
 * ��� ������ ����
 */
void		Matrix::copyElems			(	const Matrix&		matrix		///< ���� �� ���
										)
{
	size_t		length		=	matrix.getCol()
							*	matrix.getRow()
							*	sizeof(elem_t);

	memcpy(mData, matrix.mData, length);
}

/**
 * ���� ũ���� ������� �˻�
 * @exception		����� ���� ũ�Ⱑ �ƴ� ��� ���� �߻�
 */
void		Matrix::chkSameSize		(	const Matrix&		matrix		///< �� �� ���
										) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}
}

/**
 * ��� ��� ���� ���� �˻�
 * @exception		���� ���� ���� ��� ���� �߻�
 */
void		Matrix::chkBound			(	size_t		col,	///< ���� �� �� ��ġ
											size_t		row		///< ���� �� �� ��ġ
										) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

};
