/*
 * sparse_matrix.h
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX3_H_
#define SPARSE_MATRIX3_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
* 희소 행렬 표현 클래스 (Array + Vector)
*/
class	SparseMatrix3
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
		const SparseMatrix3*		operandA;
		const SparseMatrix3*		operandB;
		elem_t						elemOperandB;
		SparseMatrix3*			result;
		THREAD_RETURN_TYPE		retVal;
	};
private:
	size_t				mRowSize;		///< 행 크기
	size_t				mColSize;		///< 열 크기
	vector_node_t2*	mData;			///< 행렬 데이터
public:
				SparseMatrix3		(	void	);
				SparseMatrix3		(	size_t		row,
										size_t		col
									);
				SparseMatrix3		(	const SparseMatrix3&		matrix		);
	virtual	~SparseMatrix3	(	void	);
public:
	elem_t		getElem		(	size_t		row,
									size_t		col
								) const;
	void		setElem		(	size_t		row,
									size_t		col,
									elem_t		elem
								);
	void		clear			(	void	);
	void		clear			(	size_t		col		);
	void		resize			(	size_t		col,
									size_t		size
								);
	SparseMatrix3	add			(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	padd		(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	sub			(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	psub		(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	multiply	(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	pmultiply	(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	multiply	(	elem_t		operand	) const;
	SparseMatrix3	pmultiply	(	elem_t		operand	) const;
	SparseMatrix3	tmultiply	(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	ptmultiply	(	const SparseMatrix3&	operand	) const;
	elem_t		square		(	void	);
	const SparseMatrix3&		equal		(	const SparseMatrix3&	operand	);
	const SparseMatrix3&		pequal		(	const SparseMatrix3&	operand	);
	bool	compare	(	const SparseMatrix3&	operand	) const;
	bool	pcompare	(	const SparseMatrix3&	operand	) const;
	SparseMatrix3	sol_cg	(	const SparseMatrix3&	operand	);
public:
	inline SparseMatrix3		operator+		(	const SparseMatrix3&	operand	) const;
	inline SparseMatrix3		operator-		(	const SparseMatrix3&	operand	) const;
	inline SparseMatrix3		operator*		(	const SparseMatrix3&	operand	) const;
	inline SparseMatrix3		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix3&		operator=		(	const SparseMatrix3&	operand	);
	inline bool	operator==		(	const SparseMatrix3&	operand	) const;
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
	void		copyElems		(	const SparseMatrix3&		matrix		);
	void		pcopyElems		(	const SparseMatrix3&		matrix		);
	void		chkSameSize	(	const SparseMatrix3&		matrix		) const;
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
	static void		delElem_		(	vector_node_t2*	data,
											size_t				row,
											size_t				col
										);
	static elem_t		getElem_		(	vector_node_t2*	data,
											size_t				row,
											size_t				col
										);
	static void		setElem_		(	vector_node_t2*	data,
											size_t				row,
											size_t				col,
											elem_t				elem
										);
};

/**
* 행렬 덧셈
* @return 행렬 덧셈 결과
*/
SparseMatrix3		SparseMatrix3::operator+		(	const SparseMatrix3&	operand	///< 피연산자
													) const
{
	return	padd(operand);
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix3		SparseMatrix3::operator-		(	const SparseMatrix3&	operand	///< 피연산자
												) const
{
	return	psub(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix3		SparseMatrix3::operator*		(	const SparseMatrix3&	operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix3		SparseMatrix3::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 대입
* @return 대입 할 행렬
*/
const SparseMatrix3&		SparseMatrix3::operator=		(	const SparseMatrix3&	operand	///< 피연산자
															)
{
	return	pequal(operand);
}

/**
* 행렬 비교
* @return 비교 결과
*/
bool	SparseMatrix3::operator==	(	const SparseMatrix3&	operand	///< 피연산자
										) const
{
	return	pcompare(operand);
}

/**
* 행렬 객체가 유효한지 검사
* @return 행렬 객체가 유효하면 true, 유효하지 않으면 false
*/
bool	SparseMatrix3::isValid		(	void	)
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
size_t	SparseMatrix3::getRow		(	void	) const
{
	return	mRowSize;
}

/**
* 열 크기 가져오기
* @return 열 크기
*/
size_t	SparseMatrix3::getCol		(	void	) const
{
	return	mColSize;
}

/**
* 행렬 요소 데이터 수 가져오기
* @return 요소 데이터 크기
*/
size_t	SparseMatrix3::getSize		(	void	) const
{
	size_t		sum		=	0;

	for(size_t cnt=0;cnt<getCol();++cnt)
	{
		sum		+=	mData[cnt].mVector.size();
	}

	return	sum;
}

}

#endif /* SPARSE_MATRIX3_H_ */
