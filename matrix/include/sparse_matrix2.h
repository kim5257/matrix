/*
 * sparse_matrix2.h
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
class	SparseMatrix2
{
public:
	enum	FuncKind
	{
		FUNC_ADD,
		FUNC_SUB,
		FUNC_MULTIPLY,
		FUNC_ELEM_MUL,
		FUNC_PMULTIPLY,
		FUNC_COPY,
		FUNC_COMPARE,
	};
	struct		OpInfo
	{
		const SparseMatrix2*	operandA;
		const SparseMatrix2*	operandB;
		elem_t					elemOperandB;
		SparseMatrix2*			result;
		void*					retVal;
	};
private:
	size_t			mCol	=	0;			///< 행 크기
	size_t			mRow	=	0;			///< 열 크기
	elem_node_t*	mData	=	NULL;		///< 행렬 데이터
public:
				SparseMatrix2			(	void	);
				SparseMatrix2			(	size_t		col,
											size_t		row
										);
				SparseMatrix2			(	const SparseMatrix2&		matrix		);
	virtual	~SparseMatrix2			(	void	);
public:
	elem_t		getElem		(	size_t		col,
									size_t		row
								) const;
	void		setElem		(	size_t		col,
									size_t		row,
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
	const SparseMatrix2&		equal		(	const SparseMatrix2&	operand	);
	const SparseMatrix2&		pequal		(	const SparseMatrix2&	operand	);
	bool			compare	(	const SparseMatrix2&	operand	) const;
	bool			pcompare	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	solution	(	const SparseMatrix2&	operand	);
public:
	inline SparseMatrix2		operator+		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator-		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix2&		operator=		(	const SparseMatrix2&	operand	);
	inline bool	operator==		(	const SparseMatrix2&	operand	) const;
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
	void		copyElems		(	const SparseMatrix2&		matrix		);
	void		pcopyElems		(	const SparseMatrix2&		matrix		);
	void		chkSameSize	(	const SparseMatrix2&		matrix		) const;
	void		chkBound		(	size_t		col,
									size_t		row
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								);
private:
	static void*	threadFunc			(	void*	pData	);
	static void*	threadAdd			(	void*	pData	);
	static void*	threadSub			(	void*	pData	);
	static void*	threadMultiply	(	void*	pData	);
	static void*	threadElemMul		(	void*	pData	);
	static void*	threadTmultiply	(	void*	pData	);
	static void*	threadCopy			(	void*	pData	);
	static void*	threadCompare		(	void*	pData	);
};

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix2		SparseMatrix2::operator+		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	add(operand);
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
SparseMatrix2		SparseMatrix2::operator-		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	sub(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix2		SparseMatrix2::operator*		(	const SparseMatrix2&	operand	///< 피연산자
													) const
{
	return	pmultiply(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix2		SparseMatrix2::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const SparseMatrix2&		SparseMatrix2::operator=		(	const SparseMatrix2&	operand	///< 피연산자
															)
{
	return	equal(operand);
}

/**
 * 행렬 비교
 * @return		비교 결과
 */
bool	SparseMatrix2::operator==		(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	return	compare(operand);
}

/**
 * 행렬 객체가 유효한지 검사
 * @return		행렬 객체가 유효하면 true, 유효하지 않으면 false
 */
bool	SparseMatrix2::isValid		(	void	)
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
size_t	SparseMatrix2::getCol			(	void	) const
{
	return	mCol;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
size_t	SparseMatrix2::getRow			(	void	) const
{
	return	mRow;
}

/**
 * 행렬 요소 데이터 수 가져오기
 * @return		요소 데이터 크기
 */
size_t	SparseMatrix2::getSize		(	void	) const
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