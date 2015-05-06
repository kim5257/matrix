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
:mColSize(0),
mRowSize(0),
mColStart(NULL)
{
}

/**
 * 생성자
 */
MatrixCSR::MatrixCSR			(	size_t		col,	///< 행 크기
									size_t		row		///< 열 크기
								)
:mColSize(0),
mRowSize(0),
mColStart(NULL)
{
	allocElems(col, row);
}

/**
 * 복사 생성자
 */
MatrixCSR::MatrixCSR			(	const MatrixCSR&		matrix		///< 복사 될 객체
								)
:mColSize(0),
mRowSize(0),
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
elem_t		MatrixCSR::getElem		(	size_t		col,	///< 참조 할 행 위치
										size_t		row		///< 참조 할 열 위치
									) const
{
	//chkBound(col, row);

	elem_t	value	=	0;
	size_t	start	=	mColStart[col];
	size_t	end		=	mColStart[col+1];

	for(size_t cnt=start;cnt<end;cnt++)
	{
		if( mData[cnt].mCol == row )
		{
			value	=	mData[cnt].mElem;
			break;
		}
	}

	return	value;
}

/**
 * 행렬 요소 값 설정
 */
void		MatrixCSR::setElem		(	size_t		col,	///< 설정 할 행 위치
										size_t		row,	///< 설정 할 열 위치
										elem_t		elem	///< 설정 할 요소 값
									)
{
	//chkBound(col, row);

	bool	found	=	false;
	size_t	start	=	mColStart[col];
	size_t	end		=	mColStart[col+1];

	for(size_t cnt=start;cnt<end;cnt++)
	{
		if( mData[cnt].mCol == row )
		{
			mData[cnt].mElem	=	elem;
			found	=	false;
		}
	}

	if( found == false )
	{
		elem_vector_itor	itor	=	mData.begin() + end;
		mData.insert(itor, node_t(row, elem));

		for(size_t cnt=col+1;cnt<mColSize+1;cnt++)
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
	const std::vector<node_t>&	vec			=	operand.mData;

	result.mData.reserve(mData.size());

	for(size_t col=0;col<getCol();++col)
	{
		size_t	start	=	mColStart[col];
		size_t	end		=	mColStart[col+1];

		for(size_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								mData[cnt].mCol,
								mData[cnt].mElem
							);
		}

		start	=	operand.mColStart[col];
		end		=	operand.mColStart[col+1];

		for(size_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								vec[cnt].mCol,
								result.getElem(col, vec[cnt].mCol) + vec[cnt].mElem
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
	const std::vector<node_t>&	vec			=	operand.mData;

	result.mData.reserve(mData.size());

	for(size_t col=0;col<getCol();++col)
	{
		size_t		start	=	mColStart[col];
		size_t		end		=	mColStart[col+1];

		for(size_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								mData[cnt].mCol,
								mData[cnt].mElem
							);
		}

		start	=	operand.mColStart[col];
		end		=	operand.mColStart[col+1];

		for(size_t cnt=start;cnt<end;++cnt)
		{
			result.setElem	(	col,
								vec[cnt].mCol,
								result.getElem(col, vec[cnt].mCol) - vec[cnt].mElem
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
	const std::vector<node_t>&	vec			=	operand.mData;

	for(size_t col=0;col<getCol();++col)
	{
		size_t	start	=	mColStart[col];
		size_t	end		=	mColStart[col+1];

		for(size_t cnt=start;cnt<end;++cnt)
		{
			size_t	start2	=	operand.mColStart[mData[cnt].mCol];
			size_t	end2	=	operand.mColStart[mData[cnt].mCol+1];

			for(size_t cnt2=start2;cnt2<end2;++cnt2)
			{
				result.setElem	(	col,
									vec[cnt2].mCol,
									result.getElem(col, vec[cnt2].mCol) + (mData[cnt].mElem * vec[cnt2].mElem)
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

	for(size_t col=0;col<getCol();++col)
	{
		size_t		start	=	mColStart[col];
		size_t		end		=	mColStart[col+1];

		for(size_t cnt=start;cnt!=end;++cnt)
		{
			result.setElem	(	col,
								mData[cnt].mCol,
								mData[cnt].mElem * operand
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
	const std::vector<node_t>&	vec			=	operand.mData;

	for(size_t col=0;col<getCol();++col)
	{
		size_t		start	=	mColStart[col];
		size_t		end		=	mColStart[col+1];

		for(size_t cnt=start;cnt!=end;++cnt)
		{
			size_t		start2	=	operand.mColStart[col];
			size_t		end2	=	operand.mColStart[col+1];

			for(size_t cnt2=start2;cnt2!=end2;++cnt2)
			{
				result.setElem	(	mData[cnt].mCol,
									vec[cnt2].mCol,
									result.getElem(mData[cnt].mCol, vec[cnt2].mCol) + (mData[cnt].mElem * vec[cnt2].mElem)
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
		for(size_t col=0;col<getCol();++col)
		{
			size_t	start	=	mColStart[col];
			size_t	end		=	mColStart[col+1];

			for(size_t cnt=start;cnt<end;++cnt)
			{
				elem_t	val		=	operand.getElem(col, mData[cnt].mCol);
				if( mData[cnt].mElem != val )
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
void		MatrixCSR::allocElems		(	size_t		col,	///< 행 크기
											size_t		row		///< 열 크기
										)
{
	try
	{
		mColSize	=	col;
		mRowSize	=	row;

		mColStart	=	new size_t[col+1];
		memset(mColStart, 0, sizeof(size_t) * (col+1));
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
	mColSize	=	0;
	mRowSize	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		MatrixCSR::copyElems		(	const MatrixCSR&		matrix		///< 복사 할 행렬
										)
{
	memcpy(mColStart, matrix.mColStart, sizeof(size_t) * (mColSize + 1));
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
void		MatrixCSR::chkBound		(	size_t		col,	///< 참조 할 행 위치
										size_t		row		///< 참조 할 열 위치
									) const
{
	if( ( col >= mColSize ) ||
		( row >= mRowSize ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

}
