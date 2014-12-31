/*
 * sparse_matrix.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"

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
:mCol(0),
 mRow(0)
{
	allocElems(matrix.getCol(), matrix.getRow());
	this->mData	=	matrix.mData;
}

SparseMatrix::~SparseMatrix			(	void	)
{

}

elem_t		SparseMatrix::getElem		(	size_t		col,
												size_t		row
											) const
{
	chkBound(col, row);

	elem_t		value	=	0;

	try
	{
		index_t	index;
		index[0]	=	col;
		index[1]	=	row;

		value	=	mData.at(index);
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

	index_t	index;
	index[0]	=	col;
	index[1]	=	row;

	mData[index]	=	elem;
}

SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(elem_node_t::const_iterator	itor=mData.begin();itor!=mData.end();itor++)
	{
		index_t	index	=	itor->first;

		result.setElem	(	index[0],
								index[1],
								itor->second
							);
	}

	for(elem_node_t::const_iterator	itor=operand.mData.begin();itor!=operand.mData.end();itor++)
	{
		index_t	index	=	itor->first;

		result.setElem	(	index[0],
								index[1],
								result.getElem(index[0], index[1]) + itor->second
							);
	}

	return	result;
}

SparseMatrix	SparseMatrix::sub		(	const SparseMatrix&	operand	) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(elem_node_t::const_iterator	itor=mData.begin();itor!=mData.end();itor++)
	{
		index_t	index	=	itor->first;

		result.setElem	(	index[0],
								index[1],
								itor->second
							);
	}

	for(elem_node_t::const_iterator	itor=operand.mData.begin();itor!=operand.mData.end();itor++)
	{
		index_t	index	=	itor->first;

		result.setElem	(	index[0],
								index[1],
								result.getElem(index[0], index[1]) - itor->second
							);
	}

	return	result;
}

void		SparseMatrix::allocElems		(	size_t		col,
												size_t		row
											)
{
	try
	{
		mCol	=	col;
		mRow	=	row;
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
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
