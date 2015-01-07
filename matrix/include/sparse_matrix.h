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
#include "matrix_typedef.h"

namespace	matrix
{

typedef	std::map<row_t, elem_t>			elem_node_t;		///< 한 개 행 데이터 형식
typedef	elem_node_t::const_iterator		elem_node_itor;	///< 한 개 행 데이터 참조자

/**
 * 희소 행렬 표현 클래스
 */
class	SparseMatrix
{
private:
	size_t			mCol	=	0;			///< 행 크기
	size_t			mRow	=	0;			///< 열 크기
	elem_node_t*	mData	=	NULL;		///< 행렬 데이터
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

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix		SparseMatrix::operator+		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	add(operand);
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
SparseMatrix		SparseMatrix::operator-		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	sub(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix		SparseMatrix::operator*		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix		SparseMatrix::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::operator=		(	const SparseMatrix&	operand	///< 피연산자
															)
{
	return	equal(operand);
}

/**
 * 행렬 객체가 유효한지 검사
 * @return		행렬 객체가 유효하면 true, 유효하지 않으면 false
 */
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

/**
 * 행 크기 가져오기
 * @return		행 크기
 */
size_t	SparseMatrix::getCol			(	void	) const
{
	return	mCol;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
size_t	SparseMatrix::getRow			(	void	) const
{
	return	mRow;
}

/**
 * 행렬 요소 데이터 수 가져오기
 * @return		요소 데이터 크기
 */
size_t	SparseMatrix::getSize		(	void	) const
{
	size_t		sum		=	0;

	for(size_t cnt=0;cnt<getCol();cnt++)
	{
		sum		+=	mData[cnt].size();
	}

	return	sum;
}

}

#endif /* SPARSE_MATRIX_H_ */
