/*
 * matrix.h
 *
 *  Created on: 2014. 12. 26.
 *      Author: asran
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace matrix
{

/**
 * 행렬 표현 클래스
 */
class	Matrix
{
private:
	size_t		mRowSize;	///< 행 크기
	size_t		mColSize;	///< 열 크기
	elem_t*		mData;		///< 행렬 데이터
public:
				Matrix			(	void	);
				Matrix			(	size_t		row,
									size_t		col
								);
				Matrix			(	const Matrix&		matrix		);
	virtual	~Matrix		(	void	);
public:
	elem_t		getElem		(	size_t		row,
									size_t		col
								) const;
	void		setElem		(	size_t		row,
									size_t		col,
									elem_t		elem
								);
	Matrix		add				(	const Matrix&	operand	) const;
	Matrix		sub				(	const Matrix&	operand	) const;
	Matrix		multiply		(	const Matrix&	operand	) const;
	Matrix		multiply		(	elem_t		operand	) const;
	Matrix		transpose		(	void	) const;
	Matrix		stmultiply		(	const Matrix&	operand	) const;
	const Matrix&		equal			(	const Matrix&	operand	);
	Matrix		sol_cg		(	const Matrix&	operand	);
public:
	inline Matrix		operator+		(	const Matrix&	operand	) const;
	inline Matrix		operator-		(	const Matrix&	operand	) const;
	inline Matrix		operator*		(	const Matrix&	operand	) const;
	inline Matrix		operator*		(	elem_t		operand		) const;
	inline const Matrix&		operator=		(	const Matrix&	operand	);
public:
	inline bool	isValid		(	void	);
	inline size_t	getRow			(	void	) const;
	inline size_t	getCol			(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	size_t		row,
									size_t		col
								);
	void		freeElems		(	void	);
	void		copyElems		(	const Matrix&		matrix		);
	void		chkSameSize	(	const Matrix&		matrix		) const;
	void		chkBound		(	size_t		row,
									size_t		col
								) const;
};

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
Matrix		Matrix::operator+		(	const Matrix&	operand	) const
{
	return	add(operand);
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
Matrix		Matrix::operator-		(	const Matrix&	operand	) const
{
	return	sub(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
Matrix		Matrix::operator*		(	const Matrix&	operand	) const
{
	return	multiply(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
Matrix		Matrix::operator*		(	elem_t		operand		) const
{
	return	multiply(operand);
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const Matrix&		Matrix::operator=		(	const Matrix&	operand	)
{
	return	equal(operand);
}

/**
 * 행렬 객체가 유효한지 검사
 * @return		행렬 객체가 유효하면 true, 유효하지 않으면 false
 */
bool	Matrix::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mRowSize != 0) &&
		(mColSize != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
size_t	Matrix::getRow			(	void	) const
{
	return	mRowSize;
}

/**
 * 행 크기 가져오기
 * @return		행 크기
 */
size_t	Matrix::getCol			(	void	) const
{
	return	mColSize;
}

size_t	Matrix::getSize		(	void	) const
{
	return	getRow() * getCol();
}

};

#endif /* MATRIX_H_ */
