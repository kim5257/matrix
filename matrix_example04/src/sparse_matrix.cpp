/*
 * sparse_matrix.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

namespace matrix
{

SparseMatrix::SparseMatrix			(	void	)
:mCol(0),
 mRow(0)
{
}

SparseMatrix::SparseMatrix			(	size_t		col,
											size_t		row
										)
:mCol(0),
 mRow(0)
{
	allocElems(col, row);
}

SparseMatrix::SparseMatrix			(	const SparseMatrix&		matrix		)
:mCol(matrix.getCol()),
 mRow(matrix.getRow())
{
	this->mData	=	matrix.mData;
}

SparseMatrix::~SparseMatrix			(	void	)
{
	freeElems();
}

elem_t		SparseMatrix::getElem		(	size_t		col,
												size_t		row
											) const
{
	chkBound(col, row);

	elem_t		value	=	0;

	try
	{
		value	=	mData[col].at(row);
	}
	catch( std::out_of_range&	exception	)
	{
		value	=	0;
	}

	return	value;
}

void		SparseMatrix::setElem		(	size_t		col,
												size_t		row,
												elem_t		elem
											)
{
	chkBound(col, row);

	mData[col][row]	=	elem;
}

SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second
								);
		}
	}

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									result.getElem(col, itor->first) + itor->second
								);
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::sub		(	const SparseMatrix&	operand	) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second
								);
		}
	}

	for(size_t col=0;col<operand.getCol();col++)
	{
		for(elem_node_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									result.getElem(col, itor->first) - itor->second
								);
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::multiply	(	const SparseMatrix&	operand	) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			for(elem_node_itor itor2=operand.mData[itor->first].begin();itor2!=operand.mData[itor->first].end();itor2++)
			{
				result.setElem	(	col,
										itor2->first,
										result.getElem(col, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::multiply	(	elem_t		operand	) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second * operand
								);
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::tmultiply	(	const SparseMatrix&	operand	) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			for(elem_node_itor itor2=operand.mData[col].begin();itor2!=operand.mData[col].end();itor2++)
			{
				result.setElem	(	itor->first,
										itor2->first,
										result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

const SparseMatrix&		SparseMatrix::equal			(	const SparseMatrix&	operand	)
{
	chkSameSize(operand);
	copyElems(operand);

	return	*this;
}

SparseMatrix		SparseMatrix::solution		(	const SparseMatrix&	operand	)
{
	SparseMatrix		x			=	SparseMatrix(this->getRow(), operand.getRow());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.tmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<1000;cnt++)
	{
		SparseMatrix	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.tmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.tmultiply(r);

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

void		SparseMatrix::allocElems		(	size_t		col,
												size_t		row
											)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mData	=	new elem_node_t[col];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

void		SparseMatrix::freeElems		(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

void		SparseMatrix::copyElems		(	const SparseMatrix&		matrix		)
{
	for(size_t col=0;col<getCol();col++)
	{
		mData[col].clear();
		mData[col]		=	matrix.mData[col];
	}
}

void		SparseMatrix::chkSameSize	(	const SparseMatrix&		matrix		) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

void		SparseMatrix::chkBound		(	size_t		col,
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
