/*
 * sparse_matrix_csr.cpp
 *
 *  Created on: 2015. 1. 28.
 *      Author: asran
 */

#include "matrix_csr.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_FUNC_THRESHOLD	(1)
#define	THREAD_NUM					(4)

namespace matrix
{

typedef	void*(*Operation)(void*);

struct		FuncInfo
{
	MatrixCSR::OpInfo		opInfo;
	Operation				func;
	size_t					startCol;
	size_t					endCol;
};

/**
 * 생성자
 */
MatrixCSR::MatrixCSR			(	void	)
:mCol(0),
mRow(0),
mColStart(NULL)
{
}

/**
 * 생성자
 */
MatrixCSR::MatrixCSR			(	col_t		col,	///< 행 크기
									row_t		row		///< 열 크기
								)
:mCol(0),
mRow(0),
mColStart(NULL)
{
	allocElems(col, row);
}

/**
 * 복사 생성자
 */
MatrixCSR::MatrixCSR			(	const MatrixCSR&		matrix		///< 복사 될 객체
								)
:mCol(0),
mRow(0),
mColStart(NULL)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * 소멸자
 */
MatrixCSR::~MatrixCSR		(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return		참조한 행렬 요소 값
 */
elem_t		MatrixCSR::getElem		(	col_t		col,	///< 참조 할 행 위치
										row_t		row		///< 참조 할 열 위치
									) const
{
	chkBound(col, row);

	elem_t	value	=	0;
	size_t	start	=	mColStart[col];
	size_t	end		=	mColStart[col+1];

	for(size_t cnt=start;cnt<end;cnt++)
	{
		if( mData[cnt].mRow == row )
		{
			value	=	mData[cnt].mData;
			break;
		}
	}

	return	value;
}

/**
 * 행렬 요소 값 설정
 */
void		MatrixCSR::setElem		(	col_t		col,	///< 설정 할 행 위치
										row_t		row,	///< 설정 할 열 위치
										elem_t		elem	///< 설정 할 요소 값
									)
{
	chkBound(col, row);

	bool	found	=	false;
	col_t	start	=	mColStart[col];
	col_t	end		=	mColStart[col+1];

	for(col_t cnt=start;cnt<end;cnt++)
	{
		if( mData[cnt].mRow == row )
		{
			mData[cnt].mData	=	elem;
			found	=	false;
		}
	}

	if( found == false )
	{
		elem_vector_itor	itor	=	mData.begin() + end;
		mData.insert(itor, node_t(row, elem));

		for(size_t cnt=col+1;cnt<mCol+1;cnt++)
		{
			mColStart[cnt]++;
		}
	}
}

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
MatrixCSR		MatrixCSR::add		(	const MatrixCSR&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	MatrixCSR				result		=	MatrixCSR(getCol(), getRow());
	const elem_vector_t&	vec			=	operand.mData;

	result.mData.reserve(mData.size());

	for(col_t col=0;col<getCol();++col)
	{
		row_t	start	=	mColStart[col];
		row_t	end		=	mColStart[col+1];

		for(row_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								mData[cnt].mRow,
								mData[cnt].mData
							);
		}

		start	=	operand.mColStart[col];
		end		=	operand.mColStart[col+1];

		for(row_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
									vec[cnt].mRow,
									result.getElem(col, vec[cnt].mRow) + vec[cnt].mData
								);
		}
	}

	return	result;
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
MatrixCSR		MatrixCSR::sub		(	const MatrixCSR&	operand	///< 피연산자
									) const
{
	chkSameSize(operand);

	MatrixCSR				result		=	MatrixCSR(getCol(), getRow());
	const elem_vector_t&	vec			=	operand.mData;

	result.mData.reserve(mData.size());

	for(col_t col=0;col<getCol();++col)
	{
		row_t		start	=	mColStart[col];
		row_t		end		=	mColStart[col+1];

		for(row_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								mData[cnt].mRow,
								mData[cnt].mData
							);
		}

		start	=	operand.mColStart[col];
		end		=	operand.mColStart[col+1];

		for(row_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
									vec[cnt].mRow,
									result.getElem(col, vec[cnt].mRow) - vec[cnt].mData
								);
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
MatrixCSR		MatrixCSR::multiply	(	const MatrixCSR&	operand	///< 피연산자
										) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	MatrixCSR				result		=	MatrixCSR(getCol(), operand.getRow());
	const elem_vector_t&	vec			=	operand.mData;

	for(col_t col=0;col<getCol();++col)
	{
		row_t	start	=	mColStart[col];
		row_t	end		=	mColStart[col+1];

		for(row_t cnt=start;cnt<end;++cnt)
		{
			row_t	start2	=	operand.mColStart[mData[cnt].mRow];
			row_t	end2	=	operand.mColStart[mData[cnt].mRow+1];

			for(row_t cnt2=start2;cnt2<end2;++cnt2)
			{
				result.setElem	(	col,
									vec[cnt2].mRow,
									result.getElem(col, vec[cnt2].mRow) + (mData[cnt].mData * vec[cnt2].mData)
								);
			}
		}
	}

	return	result;
}


/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
MatrixCSR		MatrixCSR::multiply	(	elem_t		operand	///< 피연산자
										) const
{
	MatrixCSR	result				=	MatrixCSR(getCol(), getRow());

	for(col_t col=0;col<getCol();++col)
	{
		row_t		start	=	mColStart[col];
		row_t		end		=	mColStart[col+1];

		for(row_t cnt=start;cnt!=end;++cnt)
		{
			result.setElem	(	col,
									mData[cnt].mRow,
									mData[cnt].mData * operand
								);
		}
	}

	return	result;
}

/**
 * 전치 행렬 변환 후 곱셈
 * @return		행렬 곱셈 결과
 */
MatrixCSR		MatrixCSR::tmultiply		(	const MatrixCSR&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	MatrixCSR				result		=	MatrixCSR(getCol(), operand.getRow());
	const elem_vector_t&	vec			=	operand.mData;

	for(col_t col=0;col<getCol();++col)
	{
		row_t		start	=	mColStart[col];
		row_t		end		=	mColStart[col+1];

		for(row_t cnt=start;cnt!=end;++cnt)
		{
			row_t		start2	=	operand.mColStart[col];
			row_t		end2	=	operand.mColStart[col+1];

			for(row_t cnt2=start2;cnt2!=end2;++cnt2)
			{
				result.setElem	(	mData[cnt].mRow,
										vec[cnt2].mRow,
										result.getElem(mData[cnt].mRow, vec[cnt2].mRow) + (mData[cnt].mData * vec[cnt2].mData)
									);
			}
		}
	}

	return	result;
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const MatrixCSR&		MatrixCSR::equal		(	const MatrixCSR&	operand	///< 피연산자
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
		allocElems(operand.getCol(), operand.getRow());
		copyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 비교 연산
 * @return		두 행렬이 일치하면 true, 비 일치하면 false
 */
bool			MatrixCSR::compare			(	const MatrixCSR&	operand
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(col_t col=0;col<getCol();++col)
		{
			row_t	start	=	mColStart[col];
			row_t	end		=	mColStart[col+1];

			for(row_t cnt=start;cnt<end;++cnt)
			{
				elem_t	val		=	operand.getElem(col, mData[cnt].mRow);
				if( mData[cnt].mData != val )
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
 * 행렬 데이터 공간 할당
 * @exception		메모리 할당 실패 시 에러 발생
 */
void		MatrixCSR::allocElems		(	col_t		col,	///< 행 크기
											row_t		row		///< 열 크기
										)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mColStart	=	new col_t[col+1];
		memset(mColStart, 0, sizeof(col_t) * (col+1));
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		MatrixCSR::freeElems		(	void	)
{
	delete[]	mColStart;
	mCol	=	0;
	mRow	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		MatrixCSR::copyElems		(	const MatrixCSR&		matrix		///< 복사 할 행렬
										)
{
	memcpy(mColStart, matrix.mColStart, sizeof(col_t) * (mCol + 1));
	mData	=	matrix.mData;
}


/**
 * 같은 크기의 행렬인지 검사
 * @exception		행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		MatrixCSR::chkSameSize	(	const MatrixCSR&		matrix		///< 비교 할 행렬
										) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

/**
 * 행렬 요소 참조 범위 검사
 * @exception		참조 범위 밖일 경우 예외 발생
 */
void		MatrixCSR::chkBound		(	col_t		col,	///< 참조 할 행 위치
										row_t		row		///< 참조 할 열 위치
									) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

}
