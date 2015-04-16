/*
 * sparse_matrix.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_FUNC_THRESHOLD	(1)
#define	THREAD_NUM					(8)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix::OpInfo		opInfo;
	Operation					func;
	col_t						startCol;
	col_t						endCol;
};

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	void	)
:mCol(0),
mRow(0),
mData(NULL)
{
}

/**
 * 소멸자
 */
SparseMatrix::SparseMatrix		(	col_t		col,	///< 행
										row_t		row		///< 열
									)
:mCol(0),
mRow(0),
mData(NULL)
{
	allocElems(col, row);
}

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	const SparseMatrix&		matrix		///< 행렬
									)
:mCol(0),
mRow(0),
mData(NULL)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * 소멸자
 */
SparseMatrix::~SparseMatrix			(	void	)
{
	freeElems();
}

/**
* 행렬 요소 값 참조
* @return 참조한 행렬 요소 값
*/
elem_t		SparseMatrix::getElem		(	col_t		col,	///< 참조 할 행 위치
												row_t		row		///< 참조 할 열 위치
											) const
{
	chkBound(col, row);

	return	SparseMatrix::getElem_(mData, col, row);
}

/**
* 행렬 요소 값 설정
*/
void		SparseMatrix::setElem		(	col_t		col,	///< 설정 할 행 위치
												row_t		row,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	chkBound(col, row);

	SparseMatrix::setElem_(mData, col, row, elem);
}

/**
* 행렬 전체 값을 0으로 설정
*/
void		SparseMatrix::clear			(	void	)
{
	for(col_t col=0;col<getCol();++col)
	{
		clear(col);
	}
}

/**
* 행렬 한 행 값을 0으로 설정
*/
void		SparseMatrix::clear			(	col_t		col		///< 설정 할 행 위치
											)
{
	mData[col].mVector.clear();
}

/**
* 행렬 덧셈
* @return 행렬 덧셈 결과
*/
SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result		=	SparseMatrix(getCol(), getRow());

	result.equal(*this);

	for(col_t col=0;col<operand.getCol();++col)
	{
		elem_vector_t&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) + itor->mData
								);
		}
	}

	return	result;
}

/**
* 쓰레드 행렬 덧셈
* @return 행렬 덧셈 결과
*/
SparseMatrix	SparseMatrix::padd	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(col_t col=0;col<operand.getCol();++col)
		{
			elem_vector_t&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
										itor->mRow,
										result.getElem(col, itor->mRow) + itor->mData
									);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_ADD, info);
	}

	return	result;
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix	SparseMatrix::sub		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result		=	SparseMatrix(getCol(), getRow());

	result.equal(*this);

	for(col_t col=0;col<operand.getCol();++col)
	{
		elem_vector_t&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) - itor->mData
								);
		}
	}

	return	result;
}

/**
* 행렬 뺄셈
* @return 행렬 뺄셈 결과
*/
SparseMatrix	SparseMatrix::psub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(col_t col=0;col<operand.getCol();++col)
		{
			elem_vector_t&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
										itor->mRow,
										result.getElem(col, itor->mRow) - itor->mData
									);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_SUB, info);
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
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(col_t col=0;col<getCol();++col)
	{
		elem_vector_t&	vec		=	mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_vector_t&	vec2	=	operand.mData[itor->mRow].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	col,
										itor2->mRow,
										result.getElem(col, itor2->mRow) + (itor->mData * itor2->mData)
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
SparseMatrix	SparseMatrix::pmultiply	(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
		{
			elem_vector_t&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_vector_t&	vec2	=	operand.mData[itor->mRow].mVector;

				for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	col,
											itor2->mRow,
											result.getElem(col, itor2->mRow) + (itor->mData * itor2->mData)
										);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_MULTIPLY, info);
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
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	for(col_t col=0;col<getCol();++col)
	{
		elem_vector_t&	vec			=	mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									itor->mData * operand
								);
		}
	}

	return	result;
}

/**
* 쓰레드 행렬 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix	SparseMatrix::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
		{
			elem_vector_t&	vec			=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
										itor->mRow,
										itor->mData * operand
									);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA			=	this;
		info.elemOperandB		=	operand;
		info.result			=	&result;

		doThreadFunc(FUNC_ELEM_MUL, info);
	}

	return	result;
}

/**
* 전치 행렬 변환 후 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix	SparseMatrix::tmultiply	(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(col_t col=0;col<getCol();++col)
	{
		elem_vector_t&	vec		=	mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_vector_t&	vec2	=	operand.mData[col].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	itor->mRow,
										itor2->mRow,
										result.getElem(itor->mRow, itor2->mRow) + (itor->mData * itor2->mData)
									);
			}
		}
	}

	return	result;
}

/**
* 쓰레드 전치 행렬 변환 후 곱셈
* @return 행렬 곱셈 결과
*/
SparseMatrix	SparseMatrix::ptmultiply	(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
		{
			elem_vector_t&	vec		=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_vector_t&	vec2	=	operand.mData[col].mVector;

				for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	itor->mRow,
											itor2->mRow,
											result.getElem(itor->mRow, itor2->mRow) + (itor->mData * itor2->mData)
										);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_PMULTIPLY, info);
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
		allocElems(operand.getCol(), operand.getRow());
		copyElems(operand);
	}

	return	*this;
}

/**
* 쓰레드 행렬 대입
* @return 대입 할 행렬
*/
const SparseMatrix&		SparseMatrix::pequal			(	const SparseMatrix&	operand	///< 피연산자
															)
{
	try
	{
		chkSameSize(operand);
		pcopyElems(operand);
	}
	catch( ErrMsg*	)
	{
		freeElems();
		allocElems(operand.getCol(), operand.getRow());
		pcopyElems(operand);
	}

	return	*this;
}

/**
* 행렬 비교 연산
* @return 두 행렬이 일치하면 true, 비 일치하면 false
*/
bool			SparseMatrix::compare		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(col_t col=0;col<getCol();++col)
		{
			elem_vector_t&	vec		=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_t	val		=	operand.getElem(col, itor->mRow);
				if( itor->mData != val )
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
* 쓰레드 행렬 비교 연산
* @return 두 행렬이 일치하면 true, 비 일치하면 false
*/
bool			SparseMatrix::pcompare		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		if( getCol() < THREAD_FUNC_THRESHOLD )
		{
			ret		=	compare(operand);
		}
		else
		{
			OpInfo		info;

			info.operandA		=	this;
			info.operandB		=	&operand;

			doThreadFunc(FUNC_COMPARE, info);

			ret		=	(info.retVal == 0)?(false):(true);
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
SparseMatrix		SparseMatrix::solution		(	const SparseMatrix&	operand	///< 피연산자
													)
{
	SparseMatrix		x			=	SparseMatrix(this->getRow(), operand.getRow());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.ptmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<32;cnt++)
	{
		SparseMatrix	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.ptmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.ptmultiply(r);

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.00001 )
		{
			foundFlag	=	true;
			break;
		}

		p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		rSold	=	rsNew;
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
void		SparseMatrix::allocElems		(	col_t		col,	///< 행 크기
												row_t		row		///< 열 크기
											)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mData	=	new vector_data_t[col];
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
	mCol	=	0;
	mRow	=	0;
}

/**
* 행렬 데이터 복사
*/
void		SparseMatrix::copyElems		(	const SparseMatrix&		matrix		///< 복사 할 행렬
											)
{
	for(size_t col=0;col<getCol();++col)
	{
		mData[col].mVector	=	matrix.mData[col].mVector;
	}
}

/**
* 쓰레드 행렬 데이터 복사
*/
void		SparseMatrix::pcopyElems		(	const SparseMatrix&		matrix		///< 복사 할 행렬
											)
{
	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			mData[col].mVector.clear();
			mData[col]		=	matrix.mData[col];
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&matrix;

		doThreadFunc(FUNC_COPY, info);
	}
}

/**
* 같은 크기의 행렬인지 검사
* @exception 행렬이 같은 크기가 아닐 경우 예외 발생
*/
void		SparseMatrix::chkSameSize	(	const SparseMatrix&		matrix		///< 비교 할 행렬
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
* @exception 참조 범위 밖일 경우 예외 발생
*/
void		SparseMatrix::chkBound		(	col_t		col,	///< 참조 할 행 위치
												row_t		row		///< 참조 할 열 위치
											) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

/**
* 쓰레드 연산 시작
*/
void		SparseMatrix::doThreadFunc	(	FuncKind	kind,	///< 연산 종류
												OpInfo&	info	///< 연산 참조 데이터
											) const
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];

#if(PLATFORM == PLATFORM_WINDOWS)

	HANDLE			id[THREAD_NUM];

#elif(PLATFORM == PLATFORM_LINUX)

	pthread_t		id[THREAD_NUM];

#endif

	switch( kind )
	{
	case FUNC_ADD:
		orgFuncInfo.func	=	SparseMatrix::threadAdd;
		break;
	case FUNC_SUB:
		orgFuncInfo.func	=	SparseMatrix::threadSub;
		break;
	case FUNC_MULTIPLY:
		orgFuncInfo.func	=	SparseMatrix::threadMultiply;
		break;
	case FUNC_ELEM_MUL:
		orgFuncInfo.func	=	SparseMatrix::threadElemMul;
		break;
	case FUNC_PMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix::threadTmultiply;
		break;
	case FUNC_COMPARE:
		orgFuncInfo.func	=	SparseMatrix::threadCompare;
		break;
	default:
		break;
	}

	col_t	threadPerCol	=	getCol() / THREAD_NUM;
	col_t	colMod			=	getCol() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	(col_t)(num * threadPerCol);
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread ��
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix::threadFunc,
								&funcInfo[num]
							);

#endif
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		switch( kind )
		{
		case FUNC_COMPARE:
			{
				THREAD_RETURN_TYPE	retVal		=	(THREAD_RETURN_TYPE)FALSE;

				info.retVal		=	(THREAD_RETURN_TYPE)TRUE;

#if(PLATFORM == PLATFORM_WINDOWS)

				::WaitForSingleObjectEx	(	id[num],
											INFINITE,
											FALSE
										);

				::GetExitCodeThread(id[num], (LPDWORD)&retVal);

				::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

				pthread_join(id[num], &retVal);

#endif

				info.retVal	=	(THREAD_RETURN_TYPE)((unsigned long)info.retVal & (unsigned long)retVal);
			}
			break;
		default:

#if(PLATFORM == PLATFORM_WINDOWS)

			::WaitForSingleObjectEx	(	id[num],
										INFINITE,
										FALSE
									);
			::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

			pthread_join(id[num], NULL);

#endif
			break;
		}
	}
}

/**
* 쓰레드 연산 시작
*/
void		SparseMatrix::doThreadFunc	(	FuncKind	kind,	///< 연산 종류
												OpInfo&	info	///< 연산 참조 데이터
											)
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];

#if(PLATFORM == PLATFORM_WINDOWS)

	HANDLE			id[THREAD_NUM];

#elif(PLATFORM == PLATFORM_LINUX)

	pthread_t		id[THREAD_NUM];

#endif

	switch( kind )
	{
	case FUNC_COPY:
		orgFuncInfo.func	=	SparseMatrix::threadCopy;
		break;
	default:
		break;
	}

	col_t	threadPerCol	=	getCol() / THREAD_NUM;
	col_t	colMod			=	getCol() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	(col_t)(num * threadPerCol);
		funcInfo[num].endCol	=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread ��
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix::threadFunc,
								&funcInfo[num]
							);

#endif
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		::WaitForSingleObjectEx	(	id[num],
									INFINITE,
									FALSE
								);
		::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_join(id[num], NULL);

#endif
	}
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadFunc			(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

/**
* 지정한 범위의 행에 대한 행렬 덧셈
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadAdd			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	&operandB.mData[start];
	vector_data_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			elem_vector_t&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					elem_t		val		=	itor2->mData + itor->mData;

					if( val != 0 )
					{
						itor2->mData	=	val;
					}
					else
					{
						// 설정 값이 0이라면 vector에서 삭제
						// 삭제 후에 itor 값을 vector의 다음 값으로 변경하지 않는 것은
						// 직후 break로 반복문으로 빠져나가므로 불필요한 동작이다.
						vec2.erase(itor2);
					}

					found			=	true;
					break;
				}
			}

			if( found == false )
			{
				vec2.push_back(node_t(itor->mRow, itor->mData));
			}
		}
	}

	return		NULL;
}

/**
* 지정한 범위의 행에 대한 행렬 뺄셈
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadSub			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	&operandB.mData[start];
	vector_data_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			elem_vector_t&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					elem_t		val		=	itor2->mData - itor->mData;

					if( val != 0 )
					{
						itor2->mData	=	val;
					}
					else
					{
						// 설정 값이 0이라면 vector에서 삭제
						// 삭제 후에 itor 값을 vector의 다음 값으로 변경하지 않는 것은
						// 직후 break로 반복문으로 빠져나가므로 불필요한 동작이다.
						vec2.erase(itor2);
					}

					found			=	true;
					break;
				}
			}

			if( found == false )
			{
				vec2.push_back(node_t(itor->mRow, -itor->mData));
			}
		}
	}

	return		NULL;
}

/**
* 지정한 범위의 행에 대한 행렬 곱셈
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadMultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	col_t			start		=	info->startCol;
	col_t			end			=	info->endCol;
	col_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	operandB.mData;
	vector_data_t*		nodeRet	=	&result.mData[start];

	for(col_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_vector_t&	vec2	=	nodeB[itor->mRow].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				elem_t		val		=	SparseMatrix::getElem_(nodeRet, col, itor2->mRow) + (itor->mData * itor2->mData);

				if( val != 0 )
				{
					SparseMatrix::setElem_(nodeRet, col, itor2->mRow, val);
				}
				else
				{
					printf("데이터가 0\n");
					SparseMatrix::delElem_(nodeRet, col, itor2->mRow);
				}
			}
		}
	}

	return	NULL;
}

/**
* 지정한 범위의 행에 대한 행렬 곱셈
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadElemMul		(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	col_t			start		=	info->startCol;
	col_t			end			=	info->endCol;
	col_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	elem_t					operandB	=	info->opInfo.elemOperandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeRet	=	&result.mData[start];

	for(col_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_t		val			=	itor->mData * operandB;

			if( val != 0 )
			{
				SparseMatrix::setElem_(nodeRet, col, itor->mRow, val);
			}
			else
			{
				printf("데이터가 0\n");
				SparseMatrix::delElem_(nodeRet, col, itor->mRow);
			}
		}
	}

	return	NULL;
}

/**
* 지정한 범위의 행에 대한 전치행렬 변환 후 곱셈
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadTmultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	col_t			start		=	info->startCol;
	col_t			end			=	info->endCol;
	col_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	&operandB.mData[start];
	vector_data_t*		nodeRet	=	result.mData;

	for(col_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeA[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_vector_t&	vec2	=	nodeB[col].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				// 전치 행렬 곱셈은 분리 된 각 스레드마다
				// nodeRet의 같은 행을 참조 할 수 있어 lock을 사용하여 접근 제어
				LOCK(&nodeRet[itor->mRow].mLock);
				elem_t		val		=	getElem_(nodeRet, itor->mRow, itor2->mRow);

				setElem_	(	nodeRet,
								itor->mRow,
								itor2->mRow,
								val + (itor->mData * itor2->mData)
							);
				UNLOCK(&nodeRet[itor->mRow].mLock);
			}
		}
	}

	return	NULL;
}

/**
* 지정한 범위의 행을 복사
* return 항상 NULL을 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadCopy			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	col_t			start		=	info->startCol;
	col_t			end			=	info->endCol;
	col_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	&operandB.mData[start];

	for(col_t col=0;col<=range;++col)
	{
		nodeA[col].mVector	=	nodeB[col].mVector;
	}

	return	NULL;
}

/**
* 지정한 범위 내 행에 대해서 비교 연산
* return 비교 결과를 void* 형으로 변환하여 리턴
*/
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadCompare		(	void*	pData	)
{
	bool			flag		=	true;
	FuncInfo*		info		=	(FuncInfo*)pData;
	col_t			start		=	info->startCol;
	col_t			end			=	info->endCol;
	col_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_data_t*		nodeA		=	&operandA.mData[start];
	vector_data_t*		nodeB		=	&operandB.mData[start];

	for(col_t col=0;col<=range;++col)
	{
		elem_vector_t&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			if( itor->mData != SparseMatrix::getElem_(nodeB, col, itor->mRow) )
			{
				flag		=	false;
				break;
			}
		}

		if( flag == false )
		{
			break;
		}
	}

	return	(THREAD_RETURN_TYPE)flag;
}

/**
* 행렬 데이터 제거
*/
void		SparseMatrix::delElem_		(	vector_data_t*	data,	///< vector 객체 배열
												col_t				col,	///< 제거 할 데이터 행
												row_t				row		///< 제거 할 데이터 열
											)
{
	elem_vector_t&	vec		=	data[col].mVector;

	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	{
		if( itor->mRow == row )
		{
			vec.erase(itor);
			break;
		}
	}
}

/**
* 행렬 데이터 참조
*/
elem_t		SparseMatrix::getElem_		(	vector_data_t*	data,	///< vector 객체 배열
												col_t				col,	///< 참조 할 데이터 행
												row_t				row		///< 참조 할 데이터 열
											)
{
	elem_t				value	=	0;
	elem_vector_t&	vec		=	data[col].mVector;

	if( vec.size() != 0 )
	{
		for(size_t cnt=0;cnt<vec.size();++cnt)
		{
			if( vec[cnt].mRow == row )
			{
				value	=	vec[cnt].mData;
				break;
			}
		}
	}

	return	value;
}

/**
* 행렬 데이터 설정
*/
void		SparseMatrix::setElem_		(	vector_data_t*	data,	///< vector 객체 배열
												col_t				col,	///< 추가 할 데이터 행
												row_t				row,	///< 추가 할 데이터 열
												elem_t				elem	///< 추가 할 데이터
											)
{
	bool			found	=	false;
	elem_vector_t&	vec		=	data[col].mVector;

	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	{
		if( itor->mRow == row )
		{
			if( elem != 0 )
			{
				itor->mData	=	elem;
			}
			else
			{
				// 설정 값이 0이라면 vector에서 삭제
				// 삭제 후에 itor 값을 vector의 다음 값으로 변경하지 않는 것은
				// 직후 break로 반복문으로 빠져나가므로 불필요한 동작이다.
				vec.erase(itor);
			}

			found			=	true;

			break;
		}
	}

	if( ( elem != 0 ) &&
		( found == false ) )
	{
		vec.push_back(node_t(row, elem));
	}
}


};
