/*
 * sparse_matrix3.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix3.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_NUM					(8)
#define	THREAD_FUNC_THRESHOLD	(THREAD_NUM)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix3::OpInfo	opInfo;
	Operation					func;
	size_t						startCol;
	size_t						endCol;
};

/**
 * 생성자
 */
SparseMatrix3::SparseMatrix3		(	void	)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
}

/**
 * 소멸자
 */
SparseMatrix3::SparseMatrix3		(	size_t		row,	///< 행
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
SparseMatrix3::SparseMatrix3		(	const SparseMatrix3&		matrix		///< 행렬
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
SparseMatrix3::~SparseMatrix3		(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return 참조한 행렬 요소 값
 */
elem_t		SparseMatrix3::getElem		(	size_t		row,	///< 참조 할 행 위치
												size_t		col		///< 참조 할 열 위치
											) const
{
	return	SparseMatrix3::getElem_(mData, row, col);
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix3::setElem		(	size_t		row,	///< 설정 할 행 위치
												size_t		col,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	SparseMatrix3::setElem_(mData, row, col, elem);
}

void		SparseMatrix3::clear		(	void	)
{
	for(size_t col=0;col<getCol();++col)
	{
		clear(col);
	}
}

void		SparseMatrix3::clear		(	size_t		col		)
{
	mData[col].mVector.clear();
}

void		SparseMatrix3::resize	(	size_t		col,
											size_t		size
										)
{
	mData[col].mVector.reserve(size);
}

/**
 * 행렬 덧셈
 * @return	행렬 덧셈 결과
 */
SparseMatrix3	SparseMatrix3::add	(	const SparseMatrix3&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix3		result	=	SparseMatrix3(getRow(), getCol());

	result.equal(*this);

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	itor->mRow,
									col,
									result.getElem(itor->mRow, col) + itor->mElem
								);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 덧셈
 * @return 행렬 덧셈 결과
 */
SparseMatrix3	SparseMatrix3::padd	(	const SparseMatrix3&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix3	result		=	SparseMatrix3(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	itor->mRow,
										col,
										result.getElem(itor->mRow, col) + itor->mElem
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
SparseMatrix3	SparseMatrix3::sub	(	const SparseMatrix3&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix3		result		=	SparseMatrix3(getRow(), getCol());

	result.equal(*this);

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	itor->mRow,
									col,
									result.getElem(itor->mRow, col) - itor->mElem
								);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix3	SparseMatrix3::psub	(	const SparseMatrix3&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix3	result		=	SparseMatrix3(getRow(), getCol());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	itor->mRow,
										col,
										result.getElem(itor->mRow, col) - itor->mElem
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
SparseMatrix3	SparseMatrix3::multiply	(	const SparseMatrix3&	operand	///< 피연산자
											) const
{
	if( getCol() != operand.getRow() )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix3	result	=	SparseMatrix3(getRow(), operand.getCol());

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t2>&	vec2	=	mData[itor->mRow].mVector;

			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	itor2->mRow,
										col,
										result.getElem(itor2->mRow, col) + (itor->mElem * itor2->mElem)
									);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix3	SparseMatrix3::pmultiply		(	const SparseMatrix3&	operand	///< 피연산자
												) const
{
	if( getCol() != operand.getRow() )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix3	result	=	SparseMatrix3(getRow(), operand.getCol());

	if( operand.getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				std::vector<node_t2>&	vec2	=	mData[itor->mRow].mVector;

				for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	itor2->mRow,
											col,
											result.getElem(itor2->mRow, col) + (itor->mElem * itor2->mElem)
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
SparseMatrix3	SparseMatrix3::multiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix3	result	=	SparseMatrix3(getRow(), getCol());

	for(size_t col=0;col<getCol();++col)
	{
		std::vector<node_t2>&	vec		=	mData[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	itor->mRow,
									col,
									itor->mElem * operand
								);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix3	SparseMatrix3::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix3	result	=	SparseMatrix3(getRow(), getCol());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t2>&	vec		=	mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	itor->mRow,
										col,
										itor->mElem * operand
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
 * 전치행렬 변환 후 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix3	SparseMatrix3::tmultiply		(	const SparseMatrix3&	operand	///< 피연산자
												) const
{
	if( getCol() != operand.getCol() )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix3	result	=	SparseMatrix3(getRow(), operand.getRow());

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t2>&	vec2	=	mData[col].mVector;

			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	itor2->mRow,
										itor->mRow,
										result.getElem(itor2->mRow, itor->mRow) + (itor->mElem * itor2->mElem)
									);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 전치행렬 변환 후 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix3	SparseMatrix3::ptmultiply	(	const SparseMatrix3&	operand	///< 피연산자
												) const
{
	if( getCol() != operand.getCol() )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix3	result	=	SparseMatrix3(getRow(), operand.getRow());

	if( operand.getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t2>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				std::vector<node_t2>&	vec2	=	mData[col].mVector;

				for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	itor2->mRow,
											itor->mRow,
											result.getElem(itor2->mRow, itor->mRow) + (itor->mElem * itor2->mElem)
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

elem_t		SparseMatrix3::square	(	void	)
{
	if( getCol() != 1 &&
		getRow() != 1	)
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	elem_t	result	=	0;

	if( getCol() == 1 )
	{
		std::vector<node_t2>&	vec		=	mData[0].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			result	+=	(itor->mElem * itor->mElem);
		}
	}

	return	result;
}

/**
 * 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix3&		SparseMatrix3::equal		(	const SparseMatrix3&	operand	///< 피연산자
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
 * 쓰레드 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix3&		SparseMatrix3::pequal	(	const SparseMatrix3&	operand	///< 피연산자
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
		allocElems(operand.getRow(), operand.getCol());
		pcopyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 비교 연산
 * @return 두 행렬이 일치하면 true, 비 일치하면 false
 */
bool		SparseMatrix3::compare		(	const SparseMatrix3&	operand	///< 피연산자
											) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t2>&	vec		=	mData[col].mVector;

			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_t	val		=	operand.getElem(itor->mRow, col);
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
 * 쓰레드 행렬 비교 연산
 * @return 두 행렬이 일치하면 true, 비 일치하면 false
 */
bool		SparseMatrix3::pcompare		(	const SparseMatrix3&	operand	///< 피연산자
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

			info.operandA	=	this;
			info.operandB	=	&operand;

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
SparseMatrix3		SparseMatrix3::sol_cg		(	const SparseMatrix3&	operand	///< 피연산자
												)
{
	SparseMatrix3		x			=	SparseMatrix3(this->getCol(), operand.getCol());
	SparseMatrix3		r			=	operand - ( (*this) * x );
	SparseMatrix3		p			=	r;
	//SparseMatrix3		rSold		=	r.ptmultiply(r);
	elem_t				rSold		=	r.square();
	SparseMatrix3		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<32;cnt++)
	{
		SparseMatrix3	ap		=	(*this) * p;

		printf("cnt = %lu\n", cnt);

		elem_t		ptval	=	(p.ptmultiply(ap)).getElem(0,0);

		printf("ptmultiply = %lf\n", ptval);

		//elem_t			alpha	=	rSold.getElem(0,0) / ptval;
		elem_t			alpha	=	rSold / ptval;

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		//SparseMatrix3	rsNew	=	r.ptmultiply(r);
		elem_t		rsNew		=	r.square();

		//elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));
		elem_t		sqrtVal	=	sqrt(rsNew);

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

		//p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		p		=	r + ( p * (rsNew / rSold ) );
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
void		SparseMatrix3::allocElems	(	size_t		row,	///< 행 크기
												size_t		col		///< 열 크기
											)
{
	try
	{
		mRowSize	=	row;
		mColSize	=	col;

		mData	=	new vector_node_t2[col];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		SparseMatrix3::freeElems		(	void	)
{
	delete[]	mData;
	mRowSize	=	0;
	mColSize	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		SparseMatrix3::copyElems		(	const SparseMatrix3&		matrix		///< 복사 할 행렬
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
void		SparseMatrix3::pcopyElems	(	const SparseMatrix3&		matrix		///< 복사 할 행렬
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

		info.operandA	=	this;
		info.operandB	=	&matrix;

		doThreadFunc(FUNC_COPY, info);
	}
}

/**
 * 같은 크기의 행렬인지 검사
 * @exception 행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		SparseMatrix3::chkSameSize	(	const SparseMatrix3&		matrix		///< 비교 할 행렬
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
void		SparseMatrix3::chkBound		(	size_t		row,	///< 참조 할 행 위치
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
 * 쓰레드 연산 시작
 */
void		SparseMatrix3::doThreadFunc	(	FuncKind	kind,	///< 연산 종류
												OpInfo&		info	///< 연산 참조 데이터
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
		orgFuncInfo.func	=	SparseMatrix3::threadAdd;
		break;
	case FUNC_SUB:
		orgFuncInfo.func	=	SparseMatrix3::threadSub;
		break;
	case FUNC_MULTIPLY:
		orgFuncInfo.func	=	SparseMatrix3::threadMultiply;
		break;
	case FUNC_ELEM_MUL:
		orgFuncInfo.func	=	SparseMatrix3::threadElemMul;
		break;
	case FUNC_PMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix3::threadTmultiply;
		break;
	case FUNC_COMPARE:
		orgFuncInfo.func	=	SparseMatrix3::threadCompare;
		break;
	default:
		break;
	}

	if( ( kind == FUNC_MULTIPLY ) ||
		( kind == FUNC_PMULTIPLY ) )
	{
		size_t	threadPerCol	=	info.operandB->getCol() / THREAD_NUM;
		size_t	colMod			=	info.operandB->getCol() % THREAD_NUM;

		for(size_t num=0;num<THREAD_NUM;num++)
		{
			funcInfo[num]	=	orgFuncInfo;

			funcInfo[num].startCol	=	(size_t)(num * threadPerCol);
			funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
		}

		funcInfo[THREAD_NUM-1].endCol	+=	colMod;
	}
	else
	{
		size_t	threadPerCol	=	getCol() / THREAD_NUM;
		size_t	colMod			=	getCol() % THREAD_NUM;

		for(size_t num=0;num<THREAD_NUM;num++)
		{
			funcInfo[num]	=	orgFuncInfo;

			funcInfo[num].startCol	=	(size_t)(num * threadPerCol);
			funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
		}

		funcInfo[THREAD_NUM-1].endCol	+=	colMod;
	}

	// Thread 생성
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix3::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix3::threadFunc,
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
void		SparseMatrix3::doThreadFunc	(	FuncKind	kind,	///< 연산 종류
												OpInfo&		info	///< 연산 참조 데이터
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
		orgFuncInfo.func	=	SparseMatrix3::threadCopy;
		break;
	default:
		break;
	}

	size_t	threadPerCol	=	getCol() / THREAD_NUM;
	size_t	colMod			=	getCol() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	(size_t)(num * threadPerCol);
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread 생성
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix3::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix3::threadFunc,
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

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadFunc			(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

/**
 * 지정한 범위의 행에 대한 행렬 덧셈
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadAdd			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;
	SparseMatrix3&		result		=	*info->opInfo.result;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeB		=	&operandB.mData[start];
	vector_node_t2*		nodeRet		=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t2>&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					elem_t		val		=	itor2->mElem + itor->mElem;

					if( val != 0 )
					{
						itor2->mElem	=	val;
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
				vec2.push_back(node_t2(itor->mRow, itor->mElem));
			}
		}
	}

	return		NULL;
}

/**
 * 지정한 범위의 행에 대한 행렬 뺄셈
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadSub			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;
	SparseMatrix3&		result		=	*info->opInfo.result;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeB		=	&operandB.mData[start];
	vector_node_t2*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t2>&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					elem_t		val		=	itor2->mElem - itor->mElem;

					if( val != 0 )
					{
						itor2->mElem	=	val;
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
				vec2.push_back(node_t2(itor->mRow, -itor->mElem));
			}
		}
	}

	return		NULL;
}

/**
 * 지정한 범위의 행에 대한 행렬 곱셈
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadMultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;
	SparseMatrix3&		result		=	*info->opInfo.result;

	vector_node_t2*		nodeA		=	operandA.mData;
	vector_node_t2*		nodeB		=	&operandB.mData[start];
	vector_node_t2*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t2>&	vec2	=	nodeA[itor->mRow].mVector;
			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				elem_t		val		=	SparseMatrix3::getElem_(nodeRet, itor2->mRow, col) + (itor->mElem * itor2->mElem);

				if( val != 0 )
				{
					SparseMatrix3::setElem_(nodeRet, itor2->mRow, col, val);
				}
				else
				{
					printf("데이터가 0\n");
					SparseMatrix3::delElem_(nodeRet, itor2->mRow, col);
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
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadElemMul		(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	elem_t					operandB	=	info->opInfo.elemOperandB;
	SparseMatrix3&		result		=	*info->opInfo.result;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_t		val			=	itor->mElem * operandB;

			if( val != 0 )
			{
				SparseMatrix3::setElem_(nodeRet, itor->mRow, col, val);
			}
			else
			{
				printf("데이터가 0\n");
				SparseMatrix3::delElem_(nodeRet, itor->mRow, col);
			}
		}
	}

	return	NULL;
}

/**
 * 지정한 범위의 행에 대한 전치행렬 변환 후 곱셈
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadTmultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;
	SparseMatrix3&		result		=	*info->opInfo.result;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeB		=	&operandB.mData[start];
	vector_node_t2*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeB[col].mVector;

		for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t2>&	vec2	=	nodeA[col].mVector;

			for(elem_vector_itor2 itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				// 전치 행렬 곱셈은 분리 된 각 스레드마다
				// nodeRet의 같은 행을 참조 할 수 있어 lock을 사용하여 접근 제어
				LOCK(&nodeRet[itor->mRow - start].mLock);
				elem_t		val		=	getElem_(nodeRet, itor2->mRow, itor->mRow - start);

				setElem_	(	nodeRet,
								itor2->mRow,
								itor->mRow - start,
								val + (itor->mElem * itor2->mElem)
							);
				UNLOCK(&nodeRet[itor->mRow - start].mLock);
			}
		}
	}

	return	NULL;
}

/**
 * 지정한 범위의 행을 복사
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadCopy			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeA[col].mVector	=	nodeB[col].mVector;
	}

	return	NULL;
}

/**
 * 지정한 범위 내 행에 대해서 비교 연산
 * return 비교 결과를 void* 형으로 변환하여 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix3::threadCompare		(	void*	pData	)
{
	bool			flag		=	true;
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix3&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix3&	operandB	=	*info->opInfo.operandB;

	vector_node_t2*		nodeA		=	&operandA.mData[start];
	vector_node_t2*		nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t2>&	vec		=	nodeA[col].mVector;

		if( vec.size() != nodeB[col].mVector.size() )
		{
			flag	=	false;
		}
		else
		{
			for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
			{
				if( itor->mElem != SparseMatrix3::getElem_(nodeB, itor->mRow, col) )
				{
					flag		=	false;
					break;
				}
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
void		SparseMatrix3::delElem_		(	vector_node_t2*	data,	///< vector 객체 배열
												size_t				row,	///< 삭제 할 데이터 행
												size_t				col		///< 삭제 할 데이터 열
											)
{
	std::vector<node_t2>&	vec		=	data[col].mVector;

	elem_vector_itor2 itor	=	find(vec.begin(), vec.end(), row);
	if( itor != vec.end() )
	{
		vec.erase(itor);
	}

	//for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
	//{
	//	if( itor->mRow == row )
	//	{
	//		vec.erase(itor);
	//		break;
	//	}
	//}
}

/**
 * 행렬 데이터 참조
 */
elem_t		SparseMatrix3::getElem_		(	vector_node_t2*	data,	///< vector 객체 배열
												size_t				row,	///< 참조 할 데이터 행
												size_t				col		///< 참조 할 데이터 열
											)
{
	elem_t				value	=	0;
	std::vector<node_t2>&	vec		=	data[col].mVector;

	if( vec.size() != 0 )
	{
		elem_vector_itor2 itor	=	find(vec.begin(), vec.end(), row);
		if( itor != vec.end() )
		{
			value	=	itor->mElem;
		}

		//for(size_t cnt=0;cnt<vec.size();++cnt)
		//{
		//	if( vec[cnt].mRow == row )
		//	{
		//		value	=	vec[cnt].mElem;
		//		break;
		//	}
		//}
	}

	return	value;
}

/**
 * 행렬 데이터 설정
 */
void		SparseMatrix3::setElem_		(	vector_node_t2*	data,	///< vector 객체 배열
												size_t				row,	///< 설정 할 데이터 행
												size_t				col,	///< 설정 할 데이터 열
												elem_t				elem	///< 설정 할 요소 값
											)
{
	std::vector<node_t2>&	vec		=	data[col].mVector;

	elem_vector_itor2 itor	=	find(vec.begin(), vec.end(), row);

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
		vec.push_back(node_t2(row, elem));
	}

	//bool			found	=	false;
	//for(elem_vector_itor2 itor=vec.begin();itor!=vec.end();++itor)
	//{
	//	if( itor->mRow == row )
	//	{
	//		if( elem != 0 )
	//		{
	//			itor->mElem	=	elem;
	//		}
	//		else
	//		{
	//			// 설정 값이 0이라면 vector에서 삭제
	//			// 삭제 후에 itor 값을 vector의 다음 값으로 변경하지 않는 것은
	//			// 직후 break로 반복문으로 빠져나가므로 불필요한 동작이다.
	//			vec.erase(itor);
	//		}
    //
	//		found			=	true;
    //
	//		break;
	//	}
	//}
    //
	//if( ( elem != 0 ) &&
	//	( found == false ) )
	//{
	//	vec.push_back(node_t2(row, elem));
	//}
}

};
