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
	enum	FuncKind
	{
		FUNC_ADD,			///< 덧셈
		FUNC_SUB, 			///< 뺄셈
		FUNC_MULTIPLY,	///< 곱셈
		FUNC_ELEM_MUL,	///< 행렬 x 단일 값
		FUNC_PMULTIPLY,	///< 전치 행렬 곱셈
		FUNC_COPY,			///< 행렬 복사
		FUNC_COMPARE,		///< 행렬 비교
	};
	struct		OpInfo
	{
		const SparseMatrix*		operandA;
		const SparseMatrix*		operandB;
		elem_t						elemOperandB;
		SparseMatrix*			result;
		THREAD_RETURN_TYPE		retVal;
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
	SparseMatrix	padd			(	const SparseMatrix&	operand	) const;
	SparseMatrix	sub				(	const SparseMatrix&	operand	) const;
	SparseMatrix	psub			(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	pmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply		(	elem_t		operand	) const;
	SparseMatrix	pmultiply		(	elem_t		operand	) const;
	SparseMatrix	tmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	ptmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	stmultiply		(	const SparseMatrix&	operand	) const;
	SparseMatrix	pstmultiply	(	const SparseMatrix&	operand	) const;
	SparseMatrix	square			(	void	);
	SparseMatrix	psquare		(	void	);
	const SparseMatrix&		equal		(	const SparseMatrix&	operand	);
	const SparseMatrix&		pequal		(	const SparseMatrix&	operand	);
	bool	compare	(	const SparseMatrix&	operand	) const;
	bool	pcompare	(	const SparseMatrix&	operand	) const;
	SparseMatrix	sol_cg	(	const SparseMatrix&	operand	);
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
	void		doThreadFunc	(	FuncKind	kind,
									OpInfo&	info
								) const;
	void		doThreadFunc	(	FuncKind	kind,
									OpInfo&	info
								);
private:
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadFunc			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadAdd			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadSub			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadMultiply		(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadElemMul		(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadTmultiply		(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCopy			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCompare		(	void*	pData	);
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
	return	padd(operand);
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix		SparseMatrix::operator-		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	return	psub(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix		SparseMatrix::operator*		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix		SparseMatrix::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 대입
* @return 대입 할 행렬
*/
const SparseMatrix&		SparseMatrix::operator=		(	const SparseMatrix&	operand	///< 피연산자
															)
{
	return	pequal(operand);
}

/**
* 행렬 비교
* @return 비교 결과
*/
bool	SparseMatrix::operator==	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	return	pcompare(operand);
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
