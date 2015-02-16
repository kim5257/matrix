/*
 * matrix_csr.h
 *
 *  Created on: 2015. 1. 28.
 *      Author: asran
 */

#ifndef INCLUDE_MATRIX_CSR_H_
#define INCLUDE_MATRIX_CSR_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
 * 희소 행렬 표현 클래스
 */
class	MatrixCSR
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
		const MatrixCSR*	operandA;
		const MatrixCSR*	operandB;
		elem_t				elemOperandB;
		MatrixCSR*			result;
		void*				retVal;
	};
private:
	col_t				mCol;			///< 행 크기
	row_t				mRow;			///< 열 크기
	col_t*				mColStart;
	elem_vector_t		mData;

public:
				MatrixCSR		(	void	);
				MatrixCSR		(	col_t		col,
									row_t		row
								);
				MatrixCSR		(	const MatrixCSR&		matrix		);
	virtual		~MatrixCSR		(	void	);
public:
	elem_t		getElem		(	col_t				col,
									row_t				row
								) const;
	void		setElem		(	col_t				col,
								row_t				row,
								elem_t				elem
							);
	MatrixCSR	add			(	const MatrixCSR&	operand	) const;
	MatrixCSR	sub			(	const MatrixCSR&	operand	) const;
	MatrixCSR	multiply	(	const MatrixCSR&	operand	) const;
	MatrixCSR	multiply	(	elem_t		operand	) const;
	MatrixCSR	tmultiply	(	const MatrixCSR&	operand	) const;
	const MatrixCSR&		equal		(	const MatrixCSR&	operand	);
	bool			compare	(	const MatrixCSR&	operand	) const;
	MatrixCSR	solution	(	const MatrixCSR&	operand	);
public:
	inline MatrixCSR		operator+		(	const MatrixCSR&	operand	) const;
	inline MatrixCSR		operator-		(	const MatrixCSR&	operand	) const;
	inline MatrixCSR		operator*		(	const MatrixCSR&	operand	) const;
	inline MatrixCSR		operator*		(	elem_t		operand		) const;
	inline const MatrixCSR&		operator=		(	const MatrixCSR&	operand	);
	inline bool	operator==		(	const MatrixCSR&	operand	) const;
public:
	inline bool	isValid		(	void	);
	inline col_t	getCol		(	void	) const;
	inline row_t	getRow		(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	col_t	col,
									row_t	row
								);
	void		freeElems		(	void	);
	void		copyElems		(	const MatrixCSR&		matrix		);
	void		chkSameSize	(	const MatrixCSR&		matrix		) const;
	void		chkBound		(	col_t		col,
									row_t		row
								) const;
};

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
MatrixCSR		MatrixCSR::operator+		(	const MatrixCSR&	operand	///< 피연산자
													) const
{
	return	add(operand);
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
MatrixCSR		MatrixCSR::operator-		(	const MatrixCSR&	operand	///< 피연산자
													) const
{
	return	sub(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
MatrixCSR		MatrixCSR::operator*		(	const MatrixCSR&	operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
MatrixCSR		MatrixCSR::operator*		(	elem_t		operand	///< 피연산자
													) const
{
	return	multiply(operand);
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const MatrixCSR&		MatrixCSR::operator=		(	const MatrixCSR&	operand	///< 피연산자
															)
{
	return	equal(operand);
}

/**
 * 행렬 비교
 * @return		비교 결과
 */
bool	MatrixCSR::operator==	(	const MatrixCSR&	operand	///< 피연산자
										) const
{
	return	compare(operand);
}

/**
 * 행렬 객체가 유효한지 검사
 * @return		행렬 객체가 유효하면 true, 유효하지 않으면 false
 */
bool	MatrixCSR::isValid		(	void	)
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
col_t	MatrixCSR::getCol		(	void	) const
{
	return	mCol;
}

/**
 * 열 크기 가져오기
 * @return		열 크기
 */
row_t	MatrixCSR::getRow		(	void	) const
{
	return	mRow;
}

/**
 * 행렬 요소 데이터 수 가져오기
 * @return		요소 데이터 크기
 */
size_t	MatrixCSR::getSize		(	void	) const
{
	return	mData.size();
}

}

#endif /* INCLUDE_MATRIX_CSR_H_ */
