/*
 * sparse_matrix.h
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_

#include <stdio.h>
#include <list>
#include <array>
#include <unordered_map>
#include <bits/unordered_map.h>
#include <map>
#include <numeric>

typedef	double						elem_t;
typedef	std::array<size_t, 2>	index_t;

namespace std
{

template <>
class	hash<index_t>:
		public	std::unary_function<index_t, size_t>
{
public:
	size_t		operator()		(	const	index_t&	index	) const
	{
		return	std::accumulate(index.begin(), index.end(), 0);
	}
};

}

typedef	std::unordered_map<index_t, elem_t>	elem_node_t;

namespace	matrix
{

class	SparseMatrix
{
private:
	elem_node_t	mData;
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
	SparseMatrix	add		(	const SparseMatrix&	operand	) const;
	SparseMatrix	sub		(	const SparseMatrix&	operand	) const;
public:
	inline bool	isValid		(	void	);
	inline size_t	getCol			(	void	) const;
	inline size_t	getRow			(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	size_t		col,
									size_t		row
								);
	void		chkSameSize	(	const SparseMatrix&		matrix		) const;
	void		chkBound		(	size_t		col,
									size_t		row
								) const;
};

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
	return	mData.size();
}

}

#endif /* SPARSE_MATRIX_H_ */
