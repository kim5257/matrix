/*
 * sparse_matrix2.h
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX2_H_
#define SPARSE_MATRIX2_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
* 희소 행렬 표현 클래스 (Array + Map)
*/
class	SparseMatrix2
{
public:
	enum	FuncKind
	{
		FUNC_ADD,			///< 덧셈
		FUNC_SUB, 			///< 뺄셈
		FUNC_MULTIPLY,		///< 곱셈
		FUNC_ELEM_MUL,		///< 행렬 x 단일 값
		FUNC_TMULTIPLY,		///< 전치 행렬 곱셈
		FUNC_STMULTIPLY,		///< 앞 전치 행렬 곱셈
		FUNC_COPY,			///< 행렬 복사
		FUNC_COMPARE,		///< 행렬 비교
	};
	struct		OpInfo
	{
		const SparseMatrix2*	operandA;
		const SparseMatrix2*	operandB;
		elem_t					elemOperandB;
		SparseMatrix2*			result;
		THREAD_RETURN_TYPE		retVal;
	};
private:
	size_t			mRowSize;		///< 행 크기
	size_t			mColSize;		///< 열 크기
	map_node_t*		mData;		///< 행렬 데이터
public:
				SparseMatrix2		(	void	);
				SparseMatrix2		(	size_t		row,
										size_t		col
									);
				SparseMatrix2		(	const SparseMatrix2&		matrix		);
	virtual		~SparseMatrix2		(	void	);
public:
	elem_t		getElem		(	size_t		row,
								size_t		col
							) const;
	void		setElem		(	size_t		row,
								size_t		col,
								elem_t		elem
							);
	SparseMatrix2	add			(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	padd		(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	sub			(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	psub		(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	multiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	pmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	multiply	(	elem_t		operand	) const;
	SparseMatrix2	pmultiply	(	elem_t		operand	) const;
	SparseMatrix2	tmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	ptmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	stmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	pstmultiply	(	const SparseMatrix2&	operand	) const;
	const SparseMatrix2&		equal		(	const SparseMatrix2&	operand	);
	const SparseMatrix2&		pequal		(	const SparseMatrix2&	operand	);
	bool			compare		(	const SparseMatrix2&	operand	) const;
	bool			pcompare	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	sol_cg	(	const SparseMatrix2&	operand	);
public:
	inline SparseMatrix2		operator+		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator-		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix2&		operator=		(	const SparseMatrix2&	operand	);
	inline bool	operator==		(	const SparseMatrix2&	operand	) const;
public:
	inline bool		isValid		(	void	);
	inline size_t	getRow		(	void	) const;
	inline size_t	getCol		(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	size_t		row,
									size_t		col
								);
	void		freeElems		(	void	);
	void		copyElems		(	const SparseMatrix2&		matrix		);
	void		pcopyElems		(	const SparseMatrix2&		matrix		);
	void		chkSameSize		(	const SparseMatrix2&		matrix		) const;
	void		chkBound		(	size_t		row,
									size_t		col
								) const;
	void		doThreadFunc	(	FuncKind	kind,
									OpInfo&		info
								) const;
	void		doThreadFunc	(	FuncKind	kind,
									OpInfo&		info
								);
private:
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadFunc			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadAdd			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadSub			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadMultiply	(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadElemMul		(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadTmultiply	(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadStmultiply	(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCopy			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCompare		(	void*	pData	);
};

/**
* 행렬 덧셈
* @return 행렬 덧셈 결과
*/
SparseMatrix2		SparseMatrix2::operator+		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	padd(operand);
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix2		SparseMatrix2::operator-		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	psub(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix2		SparseMatrix2::operator*		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix2		SparseMatrix2::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
* 행렬 대입
* @return 대입 할 행렬
*/
const SparseMatrix2&		SparseMatrix2::operator=		(	const SparseMatrix2&	operand	///< 피연산자
															)
{
	return	pequal(operand);
}

/**
* 행렬 비교
* @return 비교 결과
*/
bool	SparseMatrix2::operator==	(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	return	pcompare(operand);
}

/**
* 행렬 객체가 유효한지 검사
* @return 행렬 객체가 유효하면 true, 유효하지 않으면 false
*/
bool	SparseMatrix2::isValid		(	void	)
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
* @return 열 크기
*/
size_t	SparseMatrix2::getRow		(	void	) const
{
	return	mRowSize;
}

/**
* 행 크기 가져오기
* @return 행 크기
*/
size_t	SparseMatrix2::getCol		(	void	) const
{
	return	mColSize;
}

/**
* 행렬 요소 데이터 수 가져오기
* @return 요소 데이터 크기
*/
size_t	SparseMatrix2::getSize		(	void	) const
{
	size_t		sum		=	0;

	for(size_t cnt=0;cnt<getRow();cnt++)
	{
		sum		+=	mData[cnt].mMap.size();
	}

	return	sum;
}

}

#endif /* SPARSE_MATRIX2_H_ */
