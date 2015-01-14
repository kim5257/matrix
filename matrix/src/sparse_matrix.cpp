/*
 * sparse_matrix.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>
#include <pthread.h>

#define	THREAD_FUNC_THRESHOLD	(1)
#define	THREAD_NUM					(4)

namespace matrix
{

typedef	void*(*Operation)(void*);

struct		FuncInfo
{
	SparseMatrix::OpInfo	opInfo;
	Operation					func;
	size_t						startCol;
	size_t						endCol;
};

/**
 * 생성자
 */
SparseMatrix::SparseMatrix			(	void	)
{
}

/**
 * 생성자
 */
SparseMatrix::SparseMatrix			(	size_t		col,	///< 행 크기
											size_t		row		///< 열 크기
										)
{
	allocElems(col, row);
}

/**
 * 복사 생성자
 */
SparseMatrix::SparseMatrix			(	const SparseMatrix&		matrix		///< 복사 될 객체
										)
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
 * @return		참조한 행렬 요소 값
 */
elem_t		SparseMatrix::getElem		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	chkBound(col, row);

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
	chkBound(col, row);

	SparseMatrix::setElem_(mData, col, row, elem);
}

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix	SparseMatrix::add	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	result.equal(*this);

	for(size_t col=0;col<operand.getCol();++col)
	{
		for(elem_vector_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) + itor->mData
								);
		}
	}

	return	result;
}

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
			for(elem_vector_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();++itor)
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
 * @return		행렬 뺄셈 결과
 */
SparseMatrix	SparseMatrix::sub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	result.equal(*this);

	for(size_t col=0;col<operand.getCol();++col)
	{
		for(elem_vector_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									result.getElem(col, itor->mRow) - itor->mData
								);
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::psub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t col=0;col<operand.getCol();++col)
		{
			for(elem_vector_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();++itor)
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
 * @return		행렬 곱셈 결과
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

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
		{
			for(elem_vector_itor itor2=operand.mData[itor->mRow].begin();itor2!=operand.mData[itor->mRow].end();itor2++)
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

SparseMatrix	SparseMatrix::pmultiply	(	const SparseMatrix&	operand
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
			for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
			{
				for(elem_vector_itor itor2=operand.mData[itor->mRow].begin();itor2!=operand.mData[itor->mRow].end();itor2++)
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
 * @return		행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
		{
			result.setElem	(	col,
									itor->mRow,
									itor->mData * operand
								);
		}
	}

	return	result;
}

SparseMatrix	SparseMatrix::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
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
 * @return		행렬 곱셈 결과
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

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
		{
			for(elem_vector_itor itor2=operand.mData[col].begin();itor2!=operand.mData[col].end();itor2++)
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
			for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
			{
				for(elem_vector_itor itor2=operand.mData[col].begin();itor2!=operand.mData[col].end();itor2++)
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
 * @return		대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::equal			(	const SparseMatrix&	operand	///< 피연산자
															)
{
	try
	{
		chkSameSize(operand);
		copyElems(operand);
	}
	catch( ErrMsg*	exception	)
	{
		freeElems();
		allocElems(operand.getCol(), operand.getRow());
		copyElems(operand);
	}

	return	*this;
}

const SparseMatrix&		SparseMatrix::pequal			(	const SparseMatrix&	operand	///< 피연산자
															)
{
	try
	{
		chkSameSize(operand);
		pcopyElems(operand);
	}
	catch( ErrMsg*	exception	)
	{
		freeElems();
		allocElems(operand.getCol(), operand.getRow());
		pcopyElems(operand);
	}

	return	*this;
}

bool			SparseMatrix::compare		(	const SparseMatrix&	operand
												) const
{
	bool	ret		=	true;

	if( 1 )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_vector_itor itor=mData[col].begin();itor!=mData[col].end();++itor)
			{
				if( itor->mData != operand.getElem(col, itor->mRow) )
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

bool			SparseMatrix::pcompare		(	const SparseMatrix&	operand
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

			ret		=	(bool)info.retVal;
		}
	}
	else
	{
		ret		=	false;
	}

	return	ret;
}

/**
 * 행렬 방정식 해 계산
 * @return		해 계산 결과
 */
SparseMatrix		SparseMatrix::solution		(	const SparseMatrix&	operand	///< 피연산자
													)
{
	SparseMatrix		x			=	SparseMatrix(this->getRow(), operand.getRow());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.tmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<3;cnt++)
	{
		SparseMatrix	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.tmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.tmultiply(r);

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.001 )
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
 * @exception		메모리 할당 실패 시 에러 발생
 */
void		SparseMatrix::allocElems	(	size_t		col,	///< 행 크기
												size_t		row		///< 열 크기
											)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mData	=	new elem_vector_t[col];
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
		mData[col].clear();
		mData[col]		=	matrix.mData[col];
	}
}

void		SparseMatrix::pcopyElems		(	const SparseMatrix&		matrix		///< 복사 할 행렬
											)
{
	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			mData[col].clear();
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
 * @exception		행렬이 같은 크기가 아닐 경우 예외 발생
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
 * @exception		참조 범위 밖일 경우 예외 발생
 */
void		SparseMatrix::chkBound		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

void		SparseMatrix::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
											) const
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];
	pthread_t		id[THREAD_NUM];

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

	size_t		threadPerCol	=	getCol() / THREAD_NUM;
	size_t		colMod			=	getCol() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	num * threadPerCol;
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread 생성
	for(size_t num=0;num<THREAD_NUM;num++)
	{
		pthread_create	(	&id[num],
								NULL,
								SparseMatrix::threadFunc,
								&funcInfo[num]
							);
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		switch( kind )
		{
		case FUNC_COMPARE:
			{
				void*	retVal		=	NULL;

				info.retVal		=	(void*)1;
				pthread_join(id[num], &retVal);

				info.retVal	=	(void*)((unsigned long)info.retVal & (unsigned long)retVal);
			}
			break;
		default:
			pthread_join(id[num], NULL);
			break;
		}
	}
}

void		SparseMatrix::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
											)
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];
	pthread_t		id[THREAD_NUM];

	switch( kind )
	{
	case FUNC_COPY:
		orgFuncInfo.func	=	SparseMatrix::threadCopy;
		break;
	default:
		break;
	}

	size_t		threadPerCol	=	getCol() / THREAD_NUM;
	size_t		colMod			=	getCol() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	num * threadPerCol;
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread 생성
	for(size_t num=0;num<THREAD_NUM;num++)
	{
		pthread_create	(	&id[num],
								NULL,
								SparseMatrix::threadFunc,
								&funcInfo[num]
							);
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		pthread_join(id[num], NULL);
	}
}

void*		SparseMatrix::threadFunc			(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

void*		SparseMatrix::threadAdd			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	&operandB.mData[start];
	elem_vector_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		for(elem_vector_itor itor=nodeB[col].begin();itor!=nodeB[col].end();++itor)
		{
			bool	found	=	false;

			for(elem_vector_itor itor2=nodeRet[col].begin();itor2!=nodeRet[col].end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					itor2->mData	=	itor->mData + itor2->mData;
					found			=	true;
					break;
				}
			}

			if( found == false )
			{
				nodeRet[col].push_back(node_t(itor->mRow, itor->mData));
			}
		}
	}

	return		NULL;
}

void*		SparseMatrix::threadSub			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	&operandB.mData[start];
	elem_vector_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeRet[col]	=	nodeA[col];
	}

	for(size_t col=0;col<=range;++col)
	{
		for(elem_vector_itor itor=nodeB[col].begin();itor!=nodeB[col].end();++itor)
		{
			bool	found	=	false;

			for(elem_vector_itor itor2=nodeRet[col].begin();itor2!=nodeRet[col].end();++itor2)
			{
				if( itor2->mRow == itor->mRow )
				{
					itor2->mData	=	itor->mData - itor2->mData;
					found			=	true;
					break;
				}
			}

			if( found == false )
			{
				nodeRet[col].push_back(node_t(itor->mRow, itor->mData));
			}
		}
	}

	return		NULL;
}

void*		SparseMatrix::threadMultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	operandB.mData;
	elem_vector_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_vector_itor itor=nodeA[col].begin();itor!=nodeA[col].end();++itor)
		{
			for(elem_vector_itor itor2=nodeB[itor->mRow].begin();itor2!=nodeB[itor->mRow].end();itor2++)
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

void*		SparseMatrix::threadElemMul		(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	elem_t					operandB	=	info->opInfo.elemOperandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_vector_itor itor=nodeA[col].begin();itor!=nodeA[col].end();++itor)
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

void*		SparseMatrix::threadTmultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	operandB.mData;
	elem_vector_t*		nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_vector_itor itor=nodeA[col].begin();itor!=nodeA[col].end();++itor)
		{
			for(elem_vector_itor itor2=nodeB[col].begin();itor2!=nodeB[col].end();itor2++)
			{
				elem_t		val		=	SparseMatrix::getElem_(nodeRet, itor->mRow, itor2->mRow) + (itor->mData * itor2->mData);

				if( val != 0 )
				{
					SparseMatrix::setElem_(nodeRet, itor->mRow, itor2->mRow, val);
				}
				else
				{
					printf("데이터가 0\n");
					SparseMatrix::delElem_(nodeRet, itor->mRow, itor2->mRow);
				}
			}
		}
	}

	return	NULL;
}

void*		SparseMatrix::threadCopy			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeA[col].clear();
		nodeA[col]		=	nodeB[col];
	}

	return	NULL;
}

void*		SparseMatrix::threadCompare		(	void*	pData	)
{
	bool			flag		=	true;
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	elem_vector_t*		nodeA		=	&operandA.mData[start];
	elem_vector_t*		nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<range;++col)
	{
		for(elem_vector_itor itor=nodeA[col].begin();itor!=nodeA[col].end();++itor)
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

	return	(void*)flag;
}

void		SparseMatrix::delElem_		(	elem_vector_t*	data,
												size_t				col,
												size_t				row
											)
{
	elem_vector_t&	vec		=	data[col];

	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	{
		if( itor->mRow == row )
		{
			vec.erase(itor);
			break;
		}
	}
}

elem_t		SparseMatrix::getElem_		(	elem_vector_t*	data,
												size_t				col,
												size_t				row
											)
{
	elem_t				value	=	0;
	elem_vector_t&	vec		=	data[col];

	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	{
		if( itor->mRow == row )
		{
			value	=	itor->mData;
			break;
		}
	}

	return	value;
}

void		SparseMatrix::setElem_		(	elem_vector_t*	data,
												size_t				col,
												size_t				row,
												elem_t				elem
											)
{
	bool				found	=	false;
	elem_vector_t&	vec		=	data[col];

	for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	{
		if( itor->mRow == row )
		{
			if( elem != 0 )
			{
				itor->mData	=	elem;
				found			=	true;
			}
			else
			{
				vec.erase(itor);
			}
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
