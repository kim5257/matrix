/*
 * matrix.h
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>

typedef	double		elem_t;

namespace matrix
{

class	Matrix
{
private:
	elem_t*	mData;		///< 행렬 데이터
	size_t		mCol;		///< 세로열
	size_t		mRow;		///< 가로열
public:
				Matrix			(	void	);
				Matrix			(	size_t		col,
									size_t		row
								);
				Matrix			(	const Matrix&		matrix		);
	virtual	~Matrix		(	void	);
public:
	elem_t		getElem		(	size_t		col,
									size_t		row
								) const;
	void		setElem		(	size_t		col,
									size_t		row,
									elem_t		elem
								);
	Matrix		add				(	const Matrix&	operand	) const;
	Matrix		sub				(	const Matrix&	operand	) const;
	Matrix		multiply		(	const Matrix&	operand	) const;
	Matrix		multiply		(	elem_t		operand	) const;
	Matrix		transpose		(	void	) const;
	const Matrix&		equal			(	const Matrix&	operand	);
	Matrix		solution		(	const Matrix&	operand	);
public:
	inline Matrix		operator+		(	const Matrix&	operand	) const;
	inline Matrix		operator-		(	const Matrix&	operand	) const;
	inline Matrix		operator*		(	const Matrix&	operand	) const;
	inline Matrix		operator*		(	elem_t		operand		) const;
	inline const Matrix&		operator=		(	const Matrix&	operand	);
public:
	inline bool	isValid		(	void	);
	inline size_t	getCol			(	void	) const;
	inline size_t	getRow			(	void	) const;
private:
	void		allocElems		(	size_t		col,
									size_t		row
								);
	void		freeElems		(	void	);
	void		copyElems		(	const Matrix&		matrix		);
	void		chkSameSize	(	const Matrix&		matrix		) const;
	void		chkBound		(	size_t		col,
									size_t		row
								) const;
};

Matrix		Matrix::operator+		(	const Matrix&	operand	) const
{
	return	add(operand);
}

Matrix		Matrix::operator-		(	const Matrix&	operand	) const
{
	return	sub(operand);
}

Matrix		Matrix::operator*		(	const Matrix&	operand	) const
{
	return	multiply(operand);
}

Matrix		Matrix::operator*		(	elem_t		operand		) const
{
	return	multiply(operand);
}

const Matrix&		Matrix::operator=		(	const Matrix&	operand	)
{
	return	equal(operand);
}

bool	Matrix::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mCol != 0) &&
		(mRow != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

size_t	Matrix::getCol			(	void	) const
{
	return	mCol;
}

size_t	Matrix::getRow			(	void	) const
{
	return	mRow;
}

};

#endif /* MATRIX_H_ */
