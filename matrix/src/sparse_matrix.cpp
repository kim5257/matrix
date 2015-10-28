/*
 * sparse_matrix3.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_NUM					(2)
#define	THREAD_FUNC_THRESHOLD	(THREAD_NUM)

namespace matrix
{

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	void	)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
}

/**
 * 소멸자
 */
SparseMatrix::SparseMatrix		(	size_t		row,	///< 행
											size_t		col		///< 열
										)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
	allocElems(row, col);
}

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	const SparseMatrix&		matrix		///< 행렬
								)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
	allocElems(matrix.getRow(), matrix.getCol());
	copyElems(matrix);
}

/**
 * 소멸자
 */
SparseMatrix::~SparseMatrix		(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return 참조한 행렬 요소 값
 */
elem_t		SparseMatrix::getElem		(	size_t		row,	///< 참조 할 행 위치
												size_t		col		///< 참조 할 열 위치
											) const
{
	return	SparseMatrix::getElem_(mData, row, col);
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix::setElem		(	size_t		row,	///< 설정 할 행 위치
												size_t		col,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	SparseMatrix::setElem_(mData, row, col, elem);
}

void		SparseMatrix::clear		(	void	)
{
	for(size_t row=0;row<getRow();++row)
	{
		clear(row);
	}
}

void		SparseMatrix::clear		(	size_t		row		)
{
	mData[row].mVector.clear();
}

/**
 * 행렬 덧셈
 * @return	행렬 덧셈 결과
 */
SparseMatrix	SparseMatrix::add	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result	=	SparseMatrix(getRow(), getCol());

	result.equal(*this);

	for(size_t row=0;row<operand.getRow();++row)
	{
		std::vector<node_t>&	vec		=	operand.mData[row].mVector;

		for(size_t cnt=0;cnt<vec.size();++cnt)
		{
			size_t	col		=	vec[cnt].mCol;

			result.setElem	(	row,
									col,
									result.getElem(row, col) + vec[cnt].mElem
								);
		}

		//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		//{
		//	result.setElem	(	row,
		//							itor->mCol,
		//							result.getElem(row, itor->mCol) + itor->mElem
		//						);
		//}
	}

	return	result;
}

/**
 * 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix	SparseMatrix::sub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result		=	SparseMatrix(getRow(), getCol());

	result.equal(*this);

	for(size_t row=0;row<operand.getRow();++row)
	{
		std::vector<node_t>&	vec		=	operand.mData[row].mVector;

		for(size_t cnt=0;cnt<vec.size();++cnt)
		{
			size_t	col		=	vec[cnt].mCol;

			result.setElem	(	row,
									col,
									result.getElem(row, col) - vec[cnt].mElem
								);
		}

		//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		//{
		//	result.setElem	(	row,
		//							itor->mCol,
		//							result.getElem(row, itor->mCol) - itor->mElem
		//						);
		//}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply	(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getRow() != operand.getCol() ) &&
		( getCol() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getRow(), operand.getCol());

	for(size_t row=0;row<getRow();++row)
	{
		std::vector<node_t>&	vec		=	mData[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	operand.mData[itor->mCol].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	row,
										itor2->mCol,
										result.getElem(row, itor2->mCol) + (itor->mElem * itor2->mElem)
									);
			}
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getRow(), getCol());

	for(size_t row=0;row<getRow();++row)
	{
		std::vector<node_t>&	vec		=	mData[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	row,
									itor->mCol,
									itor->mElem * operand
								);
		}
	}

	return	result;
}

/**
 * 뒤 전치행렬 변환 후 앞 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::tmultiply		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	if( getCol() != operand.getCol() )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getRow(), operand.getRow());

	for (size_t row = 0; row < getRow(); ++row)
	{
		std::vector<node_t>& vec = mData[row].mVector;

		for (size_t col = 0; col < operand.getRow(); ++col)
		{
			elem_t sum = 0;

			for (elem_vector_itor itor = vec.begin(); itor < vec.end(); ++itor)
			{
				sum += itor->mElem * operand.getElem(col, itor->mCol);
			}

			result.setElem(row, col, sum);
		}
	}

	return	result;
}

/**
 * 앞 전치행렬 변환 후 뒤 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::stmultiply		(	const SparseMatrix&	operand	) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getCol());

	for(size_t row=0;row<getRow();++row)
	{
		std::vector<node_t>&	vec		=	mData[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	operand.mData[row].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	itor->mCol,
										itor2->mCol,
										result.getElem(itor->mCol, itor2->mCol) + (itor->mElem * itor2->mElem)
									);
			}
		}
	}

	return	result;
}

/**
 * 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::equal			(	const SparseMatrix&	operand	///< 피연산자
													)
{
	try
	{
		chkSameSize(operand);
		copyElems(operand);
	}
	catch( ErrMsg*	)
	{
		freeElems();
		allocElems(operand.getRow(), operand.getCol());
		copyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 비교 연산
 * @return 두 행렬이 일치하면 true, 비 일치하면 false
 */
bool		SparseMatrix::compare		(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(size_t row=0;row<getRow();++row)
		{
			std::vector<node_t>&	vec		=	mData[row].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_t	val		=	operand.getElem(row, itor->mCol);
				if( itor->mElem != val )
				{
					ret		=	false;
					break;
				}
			}

			if( ret == false )
			{
				break;
			}
		}
	}
	else
	{
		ret		=	false;
	}

	return	ret;
}

/**
 * 행렬 방정식 해 구하기
 * @return		해 행렬 리턴
 */
SparseMatrix		SparseMatrix::sol_cg		(	const SparseMatrix&	operand	///< 피연산자
												)
{
	elem_t	rangeResult = 0.0;

	return sol_cg	(	operand,
						SparseMatrix(this->getCol(), operand.getCol()),
						32,
						0.0001,
						ABSOLUTE,
						rangeResult
					);
}

SparseMatrix		SparseMatrix::sol_cg		(	const SparseMatrix&	operand,
													const SparseMatrix&	init,
													uint32_t				iteration,
													elem_t					limit,
													CG_LimitType			limitType,
													elem_t&				rangeResult
												)
{
	SparseMatrix		x			=	init;
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.stmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	elem_t		preVal		=	0;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<iteration;++cnt)
	{
		SparseMatrix	ap		=	(*this) * p;

		elem_t			ptval	=	(p.stmultiply(ap)).getElem(0,0);
		elem_t			alpha	=	rSold.getElem(0,0) / ptval;

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.stmultiply(r);

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min				=	sqrtVal;
			rangeResult	=	sqrtVal;
			result			=	x;
		}

		switch( limitType )
		{
		case ABSOLUTE:
			if( sqrtVal < limit )
			{
				rangeResult	=	sqrtVal;
				foundFlag		=	true;
				break;
			}
			break;
		case RELATIVE:
			if( (preVal - sqrtVal) < limit )
			{
				rangeResult	=	sqrtVal;
				foundFlag		=	true;
				break;
			}
			break;
		}

		p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		rSold	=	rsNew;

		preVal = sqrtVal;
	}

	if( foundFlag != true )
	{
		x	=	result;
	}

	return	x;
}

/**
 * 행렬 데이터 공간 할당
 * @exception 메모리 할당 실패 시 에러 발생
 */
void		SparseMatrix::allocElems	(	size_t		row,	///< 행 크기
												size_t		col		///< 열 크기
											)
{
	try
	{
		mRowSize	=	row;
		mColSize	=	col;

		mData	=	new vector_node_t[row];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		SparseMatrix::freeElems		(	void	)
{
	delete[]	mData;
	mRowSize	=	0;
	mColSize	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		SparseMatrix::copyElems		(	const SparseMatrix&		matrix		///< 복사 할 행렬
										)
{
	for(size_t row=0;row<getRow();++row)
	{
		mData[row].mVector	=	matrix.mData[row].mVector;
	}
}

/**
 * 같은 크기의 행렬인지 검사
 * @exception 행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		SparseMatrix::chkSameSize	(	const SparseMatrix&		matrix		///< 비교 할 행렬
										) const
{
	if( ( getRow() != matrix.getRow() ) ||
		( getCol() != matrix.getCol() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

/**
 * 행렬 요소 참조 범위 검사
 * @exception 참조 범위 밖일 경우 예외 발생
 */
void		SparseMatrix::chkBound		(	size_t		row,	///< 참조 할 행 위치
												size_t		col		///< 참조 할 열 위치
											) const
{
	if( ( row >= mRowSize ) ||
		( col >= mColSize ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

/**
 * 행렬 데이터 제거
 */
void		SparseMatrix::delElem_		(	vector_node_t*	data,	///< vector 객체 배열
												size_t				row,	///< 삭제 할 데이터 행
												size_t				col		///< 삭제 할 데이터 열
											)
{
	std::vector<node_t>&	vec		=	data[row].mVector;

	elem_vector_itor itor	=	find(vec.begin(), vec.end(), col);
	if( itor != vec.end() )
	{
		vec.erase(itor);
	}

	//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	//{
	//	if( itor->mCol == row )
	//	{
	//		vec.erase(itor);
	//		break;
	//	}
	//}
}

/**
 * 행렬 데이터 참조
 */
elem_t		SparseMatrix::getElem_		(	vector_node_t*	data,	///< vector 객체 배열
												size_t				row,	///< 참조 할 데이터 행
												size_t				col		///< 참조 할 데이터 열
											)
{
	elem_t				value	=	0;
	std::vector<node_t>&	vec		=	data[row].mVector;

	if( vec.size() != 0 )
	{
		elem_vector_itor itor	=	find(vec.begin(), vec.end(), col);
		if( itor != vec.end() )
		{
			value	=	itor->mElem;
		}
	}

	return	value;
}

/**
 * 행렬 데이터 설정
 */
void		SparseMatrix::setElem_		(	vector_node_t*	data,	///< vector 객체 배열
												size_t				row,	///< 설정 할 데이터 행
												size_t				col,	///< 설정 할 데이터 열
												elem_t				elem	///< 설정 할 요소 값
											)
{
	std::vector<node_t>&	vec		=	data[row].mVector;

	elem_vector_itor itor	=	find(vec.begin(), vec.end(), col);

	if( itor != vec.end() )
	{
		if( elem != 0 )
		{
			itor->mElem	=	elem;
		}
		else
		{
			// 설정 값이 0이라면 vector에서 삭제
			// 삭제 후에 itor 값을 vector의 다음 값으로 변경하지 않는 것은
			// 직후 break로 반복문으로 빠져나가므로 불필요한 동작이다.
			vec.erase(itor);
		}
	}
	else if( elem != 0 )
	{
		vec.push_back(node_t(col, elem));
	}
}

};
