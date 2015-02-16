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
	size_t		mCol;		///< 행 크기
	size_t		mRow;		///< 열 크기
	elem_t*		mData;		///< 행렬 데이터
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

	if( (mCol != 0) &&
		(mRow != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

/**
 * 행 크기 가져오기
 * @return		행 크기
 */
size_t	Matrix::getCol			(	void	) const
{
	return	mCol;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
size_t	Matrix::getRow			(	void	) const
{
	return	mRow;
}

};

#endif /* MATRIX_H_ */
