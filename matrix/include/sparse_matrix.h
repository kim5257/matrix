/*
 * sparse_matrix.h
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX2_H_
#define SPARSE_MATRIX2_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
 * 희소 행렬 표현 클래스
 */
class	SparseMatrix
{
public:
	enum	FuncKind
	{
		FUNC_ADD,			///< 덧셈
		FUNC_SUB,			///< 뺄셈
		FUNC_MULTIPLY,		///< 곱셈
		FUNC_ELEM_MUL,		///< 행렬 x 단일 값
		FUNC_PMULTIPLY,		///< 전치 행렬 곱셈
		FUNC_COPY,			///< 행렬 복사
		FUNC_COMPARE,		///< 행렬 비교
	};
	struct		OpInfo
	{
		const SparseMatrix*		operandA;
		const SparseMatrix*		operandB;
		elem_t					elemOperandB;
		SparseMatrix*			result;
		THREAD_RETURN_TYPE		retVal;
	};
private:
	col_t			mCol;		///< 행 크기
	row_t			mRow;		///< 열 크기
	vector_data_t*	mData;		///< 행렬 데이터
public:
				SparseMatrix		(	void	);
				SparseMatrix		(	col_t		col,
										row_t		row
									);
				SparseMatrix		(	const SparseMatrix&		matrix		);
	virtual		~SparseMatrix		(	void	);
public:
	elem_t		getElem		(	col_t				col,
								row_t				row
							) const;
	void		setElem		(	col_t				col,
								row_t				row,
								elem_t				elem
							);
	SparseMatrix	add			(	const SparseMatrix&	operand	) const;
	SparseMatrix	padd		(	const SparseMatrix&	operand	) const;
	SparseMatrix	sub			(	const SparseMatrix&	operand	) const;
	SparseMatrix	psub		(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply	(	const SparseMatrix&	operand	) const;
	SparseMatrix	pmultiply	(	const SparseMatrix&	operand	) const;
	SparseMatrix	multiply	(	elem_t		operand	) const;
	SparseMatrix	pmultiply	(	elem_t		operand	) const;
	SparseMatrix	tmultiply	(	const SparseMatrix&	operand	) const;
	SparseMatrix	ptmultiply	(	const SparseMatrix&	operand	) const;
	const SparseMatrix&		equal		(	const SparseMatrix&	operand	);
	const SparseMatrix&		pequal		(	const SparseMatrix&	operand	);
	bool			compare	(	const SparseMatrix&	operand	) const;
	bool			pcompare	(	const SparseMatrix&	operand	) const;
	SparseMatrix	solution	(	const SparseMatrix&	operand	);
public:
	inline SparseMatrix		operator+		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator-		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	const SparseMatrix&	operand	) const;
	inline SparseMatrix		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix&		operator=		(	const SparseMatrix&	operand	);
	inline bool	operator==		(	const SparseMatrix&	operand	) const;
public:
	inline bool	isValid		(	void	);
	inline col_t	getCol			(	void	) const;
	inline row_t	getRow			(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	col_t		col,
									row_t		row
								);
	void		freeElems		(	void	);
	void		copyElems		(	const SparseMatrix&		matrix		);
	void		pcopyElems		(	const SparseMatrix&		matrix		);
	void		chkSameSize	(	const SparseMatrix&		matrix		) const;
	void		chkBound		(	col_t		col,
									row_t		row
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
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
	static void		delElem_		(	vector_data_t*	data,
										col_t			col,
										row_t			row
									);
	static elem_t	getElem_		(	vector_data_t*	data,
										col_t			col,
										row_t			row
									);
	static void		setElem_		(	vector_data_t*	data,
										col_t			col,
										row_t			row,
										elem_t			elem
									);
};

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix		SparseMatrix::operator+		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	padd(operand);
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
SparseMatrix		SparseMatrix::operator-		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	psub(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix		SparseMatrix::operator*		(	const SparseMatrix&	operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix		SparseMatrix::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::operator=		(	const SparseMatrix&	operand	///< 피연산자
															)
{
	return	pequal(operand);
}

/**
 * 행렬 비교
 * @return		비교 결과
 */
bool	SparseMatrix::operator==	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	return	pcompare(operand);
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
col_t	SparseMatrix::getCol		(	void	) const
{
	return	mCol;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
row_t	SparseMatrix::getRow		(	void	) const
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
		sum		+=	mData[cnt].mVector.size();
	}

	return	sum;
}

}



#endif /* SPARSE_MATRIX2_H_ */
