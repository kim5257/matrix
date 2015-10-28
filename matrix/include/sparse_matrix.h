/*
 * sparse_matrix.h
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
* 희소 행렬 표현 클래스 (Array + Vector)
*/
class	SparseMatrix
{
public:
	enum	CG_LimitType
	{
		ABSOLUTE,
		RELATIVE,
	};
private:
	size_t				mRowSize;		///< 행 크기
	size_t				mColSize;		///< 열 크기
	vector_node_t*	mData;			///< 행렬 데이터
public:
				SparseMatrix		(	void	);
				SparseMatrix		(	size_t		row,
										size_t		col
									);
				SparseMatrix		(	const SparseMatrix&		matrix		);
	virtual	~SparseMatrix	(	void	);
public:
	elem_t		getElem		(	size_t		row,
									size_t		col
								) const;
	void		setElem		(	size_t		row,
									size_t		col,
									elem_t		elem
								);
	void		clear			(	void	);
	void		clear			(	size_t		row		);
	SparseMatrix	add				(	const SparseMatrix&	operand	) const;
	SparseMatrix	sub				(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply		(	elem_t		operand	) const;
	SparseMatrix	tmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	stmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	square			(	void	);
	const SparseMatrix&		equal		(	const SparseMatrix&	operand	);
	bool	compare	(	const SparseMatrix&	operand	) const;
	SparseMatrix	sol_cg	(	const SparseMatrix&	operand	);
	SparseMatrix	sol_cg	(	const SparseMatrix&	operand,
								const SparseMatrix&	init,
								uint32_t				iteration,
								elem_t					limit,
								CG_LimitType			limitType,
								elem_t&				rangeResult
							);
public:
	inline SparseMatrix		operator+		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator-		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix&		operator=		(	const SparseMatrix&	operand	);
	inline bool	operator==		(	const SparseMatrix&	operand	) const;
public:
	inline bool		isValid		(	void	);
	inline size_t		getCol			(	void	) const;
	inline size_t		getRow			(	void	) const;
	inline size_t		getSize		(	void	) const;
private:
	void		allocElems		(	size_t		row,
									size_t		col
								);
	void		freeElems		(	void	);
	void		copyElems		(	const SparseMatrix&		matrix		);
	void		pcopyElems		(	const SparseMatrix&		matrix		);
	void		chkSameSize	(	const SparseMatrix&		matrix		) const;
	void		chkBound		(	size_t		row,
									size_t		col
								) const;
private:
	static void		delElem_		(	vector_node_t*	data,
											size_t				row,
											size_t				col
										);
	static elem_t		getElem_		(	vector_node_t*	data,
											size_t				row,
											size_t				col
										);
	static void		setElem_		(	vector_node_t*	data,
											size_t				row,
											size_t				col,
											elem_t				elem
										);
};

/**
* 행렬 덧셈
* @return 행렬 덧셈 결과
*/
SparseMatrix		SparseMatrix::operator+		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	add(operand);
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix		SparseMatrix::operator-		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	return	sub(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix		SparseMatrix::operator*		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix		SparseMatrix::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
* 행렬 대입
* @return 대입 할 행렬
*/
const SparseMatrix&		SparseMatrix::operator=		(	const SparseMatrix&	operand	///< 피연산자
															)
{
	return	equal(operand);
}

/**
* 행렬 비교
* @return 비교 결과
*/
bool	SparseMatrix::operator==	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	return	compare(operand);
}

/**
* 행렬 객체가 유효한지 검사
* @return 행렬 객체가 유효하면 true, 유효하지 않으면 false
*/
bool	SparseMatrix::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mColSize != 0) &&
		(mRowSize != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

/**
* 행 크기 가져오기
* @return 행 크기
*/
size_t	SparseMatrix::getRow		(	void	) const
{
	return	mRowSize;
}

/**
* 열 크기 가져오기
* @return 열 크기
*/
size_t	SparseMatrix::getCol		(	void	) const
{
	return	mColSize;
}

/**
* 행렬 요소 데이터 수 가져오기
* @return 요소 데이터 크기
*/
size_t	SparseMatrix::getSize		(	void	) const
{
	size_t		sum		=	0;

	for(size_t cnt=0;cnt<getRow();cnt++)
	{
		sum		+=	mData[cnt].mVector.size();
	}

	return	sum;
}

}

#endif /* SPARSE_MATRIX_H_ */
