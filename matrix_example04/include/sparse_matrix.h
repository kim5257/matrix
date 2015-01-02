/*
 * sparse_matrix.h
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_

#include <stdio.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <bits/unordered_map.h>
#include <map>
#include <numeric>

namespace	matrix
{

typedef	double		elem_t;
typedef	size_t		col_t;
typedef	size_t		row_t;

typedef	std::map<row_t, elem_t>			elem_node_t;
typedef	elem_node_t::const_iterator		elem_node_itor;

class	SparseMatrix
{
private:
	elem_node_t*	mData;
	size_t			mCol;
	size_t			mRow;
public:
				SparseMatrix			(	void	);
				SparseMatrix			(	size_t		col,
											size_t		row
										);
				SparseMatrix			(	const SparseMatrix&		matrix		);
	virtual	~SparseMatrix			(	void	);
public:
	elem_t		getElem		(	size_t		col,
									size_t		row
								) const;
	void		setElem		(	size_t		col,
									size_t		row,
									elem_t		elem
								);
	SparseMatrix	add			(	const SparseMatrix&	operand	) const;
	SparseMatrix	sub			(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply	(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply	(	elem_t		operand	) const;
	SparseMatrix	tmultiply	(	const SparseMatrix&	operand	) const;
	const SparseMatrix&		equal		(	const SparseMatrix&	operand	);
	SparseMatrix	solution	(	const SparseMatrix&	operand	);
public:
	inline SparseMatrix		operator+		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator-		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix&		operator=		(	const SparseMatrix&	operand	);
public:
	inline bool	isValid		(	void	);
	inline size_t	getCol			(	void	) const;
	inline size_t	getRow			(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	size_t		col,
									size_t		row
								);
	void		freeElems		(	void	);
	void		copyElems		(	const SparseMatrix&		matrix		);
	void		chkSameSize	(	const SparseMatrix&		matrix		) const;
	void		chkBound		(	size_t		col,
									size_t		row
								) const;
};

SparseMatrix		SparseMatrix::operator+		(	const SparseMatrix&	operand	) const
{
	return	add(operand);
}

SparseMatrix		SparseMatrix::operator-		(	const SparseMatrix&	operand	) const
{
	return	sub(operand);
}

SparseMatrix		SparseMatrix::operator*		(	const SparseMatrix&	operand	) const
{
	return	multiply(operand);
}

SparseMatrix		SparseMatrix::operator*		(	elem_t		operand		) const
{
	return	multiply(operand);
}

const SparseMatrix&		SparseMatrix::operator=		(	const SparseMatrix&	operand	)
{
	return	equal(operand);
}

bool	SparseMatrix::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mCol != 0) &&
		(mRow != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

size_t	SparseMatrix::getCol			(	void	) const
{
	return	mCol;
}

size_t	SparseMatrix::getRow			(	void	) const
{
	return	mRow;
}

size_t	SparseMatrix::getSize		(	void	) const
{
	return	0;
}

}

#endif /* SPARSE_MATRIX_H_ */
