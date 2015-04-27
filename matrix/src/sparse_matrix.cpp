/*
 * sparse_matrix.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_NUM				(8)
#define	THREAD_FUNC_THRESHOLD	(THREAD_NUM)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix::OpInfo	opInfo;
	Operation				func;
	size_t					startCol;
	size_t					endCol;
};

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	void	)
:mColSize(0),
mRowSize(0),
mData(NULL)
{
}

/**
 * 소멸자
 */
SparseMatrix::SparseMatrix		(	size_t		col,	///< 행
									size_t		row		///< 열
								)
:mColSize(0),
mRowSize(0),
mData(NULL)
{
	allocElems(col, row);
}

/**
 * 생성자
 */
SparseMatrix::SparseMatrix		(	const SparseMatrix&		matrix		///< 행렬
								)
:mColSize(0),
mRowSize(0),
mData(NULL)
{
	allocElems(matrix.getCol(), matrix.getRow());
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
elem_t		SparseMatrix::getElem		(	size_t		col,	///< 참조 할 행 위치
											size_t		row		///< 참조 할 열 위치
										) const
{
	//chkBound(col, row);

	return	SparseMatrix::getElem_(mData, col, row);
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix::setElem		(	size_t		col,	///< 설정 할 행 위치
											size_t		row,	///< 설정 할 열 위치
											elem_t		elem	///< 설정 할 요소 값
										)
{
	//chkBound(col, row);

	SparseMatrix::setElem_(mData, col, row, elem);
}

void		SparseMatrix::clear		(	void	)
{
	for(size_t col=0;col<getCol();++col)
	{
		clear(col);
	}
}

void		SparseMatrix::clear		(	size_t		col		)
{
	mData[col].mVector.clear();
}

/**
 * 행렬 덧셈
 * @return	행렬 덧셈 결과
 */
SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result	=	SparseMatrix(getCol(), getRow());

	result.equal(*this);

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
								itor->mRow,
								result.getElem(col, itor->mRow) + itor->mElem
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

		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) + itor->mElem
								);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
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

	for(size_t col=0;col<operand.getCol();++col)
	{
		std::vector<node_t>&	vec		=	operand.mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
								itor->mRow,
								result.getElem(col, itor->mRow) - itor->mElem
							);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix	SparseMatrix::psub		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t col=0;col<operand.getCol();++col)
		{
			std::vector<node_t>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) - itor->mElem
								);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_SUB, info);
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply		(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();++col)
	{
		std::vector<node_t>&	vec		=	mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	operand.mData[itor->mRow].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				result.setElem	(	col,
									itor2->mRow,
									result.getElem(col, itor2->mRow) + (itor->mElem * itor2->mElem)
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
SparseMatrix	SparseMatrix::pmultiply		(	const SparseMatrix&	operand	///< 피연산자
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
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t>&	vec		=	operand.mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				std::vector<node_t>&	vec2	=	operand.mData[itor->mRow].mVector;

				for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	col,
										itor2->mRow,
										result.getElem(col, itor2->mRow) + (itor->mElem * itor2->mElem)
									);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
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

	for(size_t col=0;col<getCol();++col)
	{
		std::vector<node_t>&	vec		=	mData[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	col,
								itor->mRow,
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
SparseMatrix	SparseMatrix::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t>&	vec		=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	col,
									itor->mRow,
									itor->mElem * operand
								);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.elemOperandB	=	operand;
		info.result			=	&result;

		doThreadFunc(FUNC_ELEM_MUL, info);
	}

	return	result;
}

/**
 * 전치행렬 변환 후 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::tmultiply		(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());


	for(size_t col=0;col<getCol();++col)
	{
		std::vector<node_t>&	vec		=	mData[col].mVector;

		if( (col % 100) == 0 )
		{
			printf("col = %lu\n", col);
		}

		for(size_t col2=0;col2<operand.getCol();++col2)
		{
			elem_t		sum		=	0;

			std::vector<node_t>&	vec2	=	operand.mData[col2].mVector;

			size_t itor	=	0;
			size_t itor2	=	0;

			while( (itor<vec.size()) && (itor2<vec2.size()) )
			{
				if( vec[itor].mRow > vec2[itor2].mRow )
				{
					++itor2;
				}
				else if( vec[itor].mRow < vec2[itor2].mRow )
				{
					++itor;
				}
				else
				{
					sum		+=		(vec[itor].mElem * vec2[itor2].mElem);

					++itor;
					++itor2;
				}
			}

			//elem_vector_itor itor	=	vec.begin();
			//elem_vector_itor itor2	=	vec2.begin();
            //
			//while( (itor!=vec.end()) && (itor2!=vec2.end()) )
			//{
			//	if( itor->mRow > itor2->mRow )
			//	{
			//		++itor2;
			//	}
			//	else if( itor->mRow < itor2->mRow )
			//	{
			//		++itor;
			//	}
			//	else
			//	{
			//		sum		+=		(itor->mElem * itor2->mElem);
            //
			//		++itor;
			//		++itor2;
			//	}
			//}

			//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			//{
			//	sum		+=	(itor->mElem * operand.getElem(col2, itor->mRow));
			//}

			if( sum != 0 )
			{
				result.setElem(col, col2, sum);
			}
		}
	}

	//for(size_t col=0;col<getCol();++col)
	//{
	//	std::vector<node_t>&	vec		=	mData[col].mVector;
    //
	//	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	//	{
	//		std::vector<node_t>&	vec2	=	operand.mData[col].mVector;
    //
	//		for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
	//		{
	//			result.setElem	(	itor->mRow,
	//								itor2->mRow,
	//								result.getElem(itor->mRow, itor2->mRow) + (itor->mElem * itor2->mElem)
	//							);
	//		}
	//	}
	//}

	return	result;
}

/**
 * 쓰레드 전치행렬 변환 후 행렬 곱셈
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
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t>&	vec		=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				std::vector<node_t>&	vec2	=	operand.mData[col].mVector;

				for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
				{
					result.setElem	(	itor->mRow,
										itor2->mRow,
										result.getElem(itor->mRow, itor2->mRow) + (itor->mElem * itor2->mElem)
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
const SparseMatrix&		SparseMatrix::pequal		(	const SparseMatrix&	operand	///< 피연산자
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
		for(size_t col=0;col<getCol();++col)
		{
			std::vector<node_t>&	vec		=	mData[col].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				elem_t	val		=	operand.getElem(col, itor->mRow);
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
SparseMatrix		SparseMatrix::sol_cg		(	const SparseMatrix&	operand	///< 피연산자
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

		elem_t		ptval	=	(p.ptmultiply(ap)).getElem(0,0);

		printf("ptmultiply = %lf\n", ptval);

		elem_t			alpha	=	rSold.getElem(0,0) / ptval;

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
void		SparseMatrix::allocElems		(	size_t		col,	///< 행 크기
												size_t		row		///< 열 크기
											)
{
	try
	{
		mColSize	=	col;
		mRowSize	=	row;

		mData	=	new vector_node_t[col];
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
	mColSize	=	0;
	mRowSize	=	0;
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
void		SparseMatrix::pcopyElems	(	const SparseMatrix&		matrix		///< 복사 할 행렬
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
void		SparseMatrix::chkBound		(	size_t		col,	///< 참조 할 행 위치
											size_t		row		///< 참조 할 열 위치
										) const
{
	if( ( col >= mColSize ) ||
		( row >= mRowSize ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

/**
 * 쓰레드 연산 시작
 */
void		SparseMatrix::doThreadFunc	(	FuncKind	kind,	///< 연산 종류
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
		orgFuncInfo.func	=	SparseMatrix::threadCopy;
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
		funcInfo[num].endCol	=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread 생성
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

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet		=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t>&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
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
				vec2.push_back(node_t(itor->mRow, itor->mElem));
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
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeB[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t>&	vec2	=	nodeRet[col].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
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
				vec2.push_back(node_t(itor->mRow, -itor->mElem));
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
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	operandB.mData;
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	nodeB[itor->mRow].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				elem_t		val		=	SparseMatrix::getElem_(nodeRet, col, itor2->mRow) + (itor->mElem * itor2->mElem);

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
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	elem_t				operandB	=	info->opInfo.elemOperandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeA[col].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_t		val			=	itor->mElem * operandB;

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
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet		=	result.mData;

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeA[col].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	nodeB[col].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				// 전치 행렬 곱셈은 분리 된 각 스레드마다
				// nodeRet의 같은 행을 참조 할 수 있어 lock을 사용하여 접근 제어
				LOCK(&nodeRet[itor->mRow].mLock);
				elem_t		val		=	getElem_(nodeRet, itor->mRow, itor2->mRow);

				setElem_	(	nodeRet,
								itor->mRow,
								itor2->mRow,
								val + (itor->mElem * itor2->mElem)
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
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];

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
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadCompare		(	void*	pData	)
{
	bool			flag		=	true;
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		std::vector<node_t>&	vec		=	nodeA[col].mVector;

		if( vec.size() != nodeB[col].mVector.size() )
		{
			flag	=	false;
		}
		else
		{
			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				if( itor->mElem != SparseMatrix::getElem_(nodeB, col, itor->mRow) )
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
void		SparseMatrix::delElem_		(	vector_node_t*	data,	///< vector 객체 배열
											size_t			col,	///< 삭제 할 데이터 행
											size_t			row		///< 삭제 할 데이터 열
										)
{
	std::vector<node_t>&	vec		=	data[col].mVector;

	elem_vector_itor itor	=	find(vec.begin(), vec.end(), row);
	if( itor != vec.end() )
	{
		vec.erase(itor);
	}

	//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
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
elem_t		SparseMatrix::getElem_		(	vector_node_t*	data,	///< vector 객체 배열
											size_t			col,	///< 참조 할 데이터 행
											size_t			row		///< 참조 할 데이터 열
										)
{
	elem_t				value	=	0;
	std::vector<node_t>&	vec		=	data[col].mVector;

	if( vec.size() != 0 )
	{
		elem_vector_itor itor	=	find(vec.begin(), vec.end(), row);
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
void		SparseMatrix::setElem_		(	vector_node_t*	data,	///< vector 객체 배열
											size_t			col,	///< 설정 할 데이터 행
											size_t			row,	///< 설정 할 데이터 열
											elem_t			elem	///< 설정 할 요소 값
										)
{
	std::vector<node_t>&	vec		=	data[col].mVector;

	elem_vector_itor itor	=	find(vec.begin(), vec.end(), row);

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
		vec.push_back(node_t(row, elem));
	}

	//bool			found	=	false;
	//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
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
	//	vec.push_back(node_t(row, elem));
	//}
}

};
