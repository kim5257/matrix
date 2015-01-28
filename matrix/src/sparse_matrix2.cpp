/*
 * sparse_matrix2.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "sparse_matrix2.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_FUNC_THRESHOLD	(1)
#define	THREAD_NUM					(4)

namespace matrix
{

typedef	void*(*Operation)(void*);

struct		FuncInfo
{
	SparseMatrix2::OpInfo	opInfo;
	Operation					func;
	size_t						startCol;
	size_t						endCol;
};

/**
 * 생성자
 */
SparseMatrix2::SparseMatrix2			(	void	)
{
}

/**
 * 생성자
 */
SparseMatrix2::SparseMatrix2			(	size_t		col,	///< 행 크기
												size_t		row		///< 열 크기
											)
{
	allocElems(col, row);
}

/**
 * 복사 생성자
 */
SparseMatrix2::SparseMatrix2			(	const SparseMatrix2&		matrix		///< 복사 될 객체
											)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * 소멸자
 */
SparseMatrix2::~SparseMatrix2			(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return		참조한 행렬 요소 값
 */
elem_t		SparseMatrix2::getElem		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	chkBound(col, row);

	elem_t		value	=	0;

	try
	{
		value	=	mData[col].mMap.at(row);
	}
	catch( std::out_of_range&	exception	)
	{
		value	=	0;
	}

	return	value;
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix2::setElem		(	size_t		col,	///< 설정 할 행 위치
												size_t		row,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	chkBound(col, row);

	if( elem != 0 )
	{
		mData[col].mMap[row]	=	elem;
	}
	else
	{
		mData[col].mMap.erase(row);
	}
}

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix2	SparseMatrix2::add		(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	result		=	*this;

	for(size_t col=0;col<operand.getCol();++col)
	{
		for(elem_map_itor itor=operand.mData[col].mMap.begin();itor!=operand.mData[col].mMap.end();++itor)
		{
			result.setElem	(	col,
									itor->first,
									result.mData[col].mMap[itor->first] + itor->second
								);
		}
	}

	return	result;
}

SparseMatrix2	SparseMatrix2::padd	(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(size_t col=0;col<operand.getCol();++col)
		{
			for(elem_map_itor itor=operand.mData[col].mMap.begin();itor!=operand.mData[col].mMap.end();++itor)
			{
				result.setElem	(	col,
										itor->first,
										result.mData[col].mMap[itor->first] + itor->second
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
SparseMatrix2	SparseMatrix2::sub	(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	result		=	*this;

	for(size_t col=0;col<operand.getCol();++col)
	{
		for(elem_map_itor itor=operand.mData[col].mMap.begin();itor!=operand.mData[col].mMap.end();++itor)
		{
			result.setElem	(	col,
									itor->first,
									result.mData[col].mMap[itor->first] - itor->second
								);
		}
	}

	return	result;
}

SparseMatrix2	SparseMatrix2::psub	(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(size_t col=0;col<operand.getCol();++col)
		{
			for(elem_map_itor itor=operand.mData[col].mMap.begin();itor!=operand.mData[col].mMap.end();++itor)
			{
				result.setElem	(	col,
										itor->first,
										result.mData[col].mMap[itor->first] - itor->second
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
SparseMatrix2	SparseMatrix2::multiply	(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
		{
			for(elem_map_itor itor2=operand.mData[itor->first].mMap.begin();itor2!=operand.mData[itor->first].mMap.end();itor2++)
			{
				result.setElem	(	col,
										itor2->first,
										result.getElem(col, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

SparseMatrix2	SparseMatrix2::pmultiply	(	const SparseMatrix2&	operand
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
			{
				for(elem_map_itor itor2=operand.mData[itor->first].mMap.begin();itor2!=operand.mData[itor->first].mMap.end();itor2++)
				{
					result.setElem	(	col,
											itor2->first,
											result.getElem(col, itor2->first) + (itor->second * itor2->second)
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
SparseMatrix2	SparseMatrix2::multiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getCol(), getRow());

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
		{
			result.setElem	(	col,
									itor->first,
									itor->second * operand
								);
		}
	}

	return	result;
}

SparseMatrix2	SparseMatrix2::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
			{
				result.setElem	(	col,
										itor->first,
										itor->second * operand
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
SparseMatrix2	SparseMatrix2::tmultiply	(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();++col)
	{
		for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
		{
			for(elem_map_itor itor2=operand.mData[col].mMap.begin();itor2!=operand.mData[col].mMap.end();itor2++)
			{
				result.setElem	(	itor->first,
										itor2->first,
										result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

SparseMatrix2	SparseMatrix2::ptmultiply	(	const SparseMatrix2&	operand	///< 피연산자
												) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
			{
				for(elem_map_itor itor2=operand.mData[col].mMap.begin();itor2!=operand.mData[col].mMap.end();itor2++)
				{
					result.setElem	(	itor->first,
											itor2->first,
											result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
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
const SparseMatrix2&		SparseMatrix2::equal			(	const SparseMatrix2&	operand	///< 피연산자
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

const SparseMatrix2&		SparseMatrix2::pequal		(	const SparseMatrix2&	operand	///< 피연산자
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

bool			SparseMatrix2::compare		(	const SparseMatrix2&	operand
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(size_t col=0;col<getCol();++col)
		{
			for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
			{
				if( itor->second != operand.getElem(col, itor->first) )
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

bool			SparseMatrix2::pcompare		(	const SparseMatrix2&	operand
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		if( getCol() < THREAD_FUNC_THRESHOLD )
		{
			for(size_t col=0;col<getCol();++col)
			{
				for(elem_map_itor itor=mData[col].mMap.begin();itor!=mData[col].mMap.end();++itor)
				{
					if( itor->second != operand.getElem(col, itor->first) )
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
SparseMatrix2		SparseMatrix2::solution		(	const SparseMatrix2&	operand	///< 피연산자
													)
{
	SparseMatrix2		x			=	SparseMatrix2(this->getRow(), operand.getRow());
	SparseMatrix2		r			=	operand - ( (*this) * x );
	SparseMatrix2		p			=	r;
	SparseMatrix2		rSold		=	r.tmultiply(r);
	SparseMatrix2		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<1000000;cnt++)
	{
		SparseMatrix2	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.tmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix2	rsNew	=	r.tmultiply(r);

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
void		SparseMatrix2::allocElems		(	size_t		col,	///< 행 크기
													size_t		row		///< 열 크기
												)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mData	=	new map_data_t[col];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		SparseMatrix2::freeElems		(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		SparseMatrix2::copyElems		(	const SparseMatrix2&		matrix		///< 복사 할 행렬
											)
{
	for(size_t col=0;col<getCol();++col)
	{
		mData[col].mMap.clear();
		mData[col]		=	matrix.mData[col];
	}
}

void		SparseMatrix2::pcopyElems		(	const SparseMatrix2&		matrix		///< 복사 할 행렬
												)
{
	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t col=0;col<getCol();++col)
		{
			mData[col].mMap.clear();
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
void		SparseMatrix2::chkSameSize	(	const SparseMatrix2&		matrix		///< 비교 할 행렬
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
void		SparseMatrix2::chkBound		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

void		SparseMatrix2::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
											) const
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];
	pthread_t		id[THREAD_NUM];

	switch( kind )
	{
	case FUNC_ADD:
		orgFuncInfo.func	=	SparseMatrix2::threadAdd;
		break;
	case FUNC_SUB:
		orgFuncInfo.func	=	SparseMatrix2::threadSub;
		break;
	case FUNC_MULTIPLY:
		orgFuncInfo.func	=	SparseMatrix2::threadMultiply;
		break;
	case FUNC_ELEM_MUL:
		orgFuncInfo.func	=	SparseMatrix2::threadElemMul;
		break;
	case FUNC_PMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix2::threadTmultiply;
		break;
	case FUNC_COMPARE:
		orgFuncInfo.func	=	SparseMatrix2::threadCompare;
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
								SparseMatrix2::threadFunc,
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

void		SparseMatrix2::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
											)
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];
	pthread_t		id[THREAD_NUM];

	switch( kind )
	{
	case FUNC_COPY:
		orgFuncInfo.func	=	SparseMatrix2::threadCopy;
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
								SparseMatrix2::threadFunc,
								&funcInfo[num]
							);
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		pthread_join(id[num], NULL);
	}
}

void*		SparseMatrix2::threadFunc		(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

void*		SparseMatrix2::threadAdd			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&		result		=	*info->opInfo.result;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];
	map_data_t*			nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			nodeRet[col].mMap[itor->first]	=	itor->second;
		}
	}

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeB[col].mMap.begin();itor!=nodeB[col].mMap.end();++itor)
		{
			elem_t		val		=	nodeRet[col].mMap[itor->first] + itor->second;

			if( val != 0 )
			{
				nodeRet[col].mMap[itor->first]	=	val;
			}
			else
			{
				nodeRet[col].mMap.erase(itor->first);
			}
		}
	}

	return		NULL;
}

void*		SparseMatrix2::threadSub			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&		result		=	*info->opInfo.result;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];
	map_data_t*			nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			nodeRet[col].mMap[itor->first]	=	itor->second;
		}
	}

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeB[col].mMap.begin();itor!=nodeB[col].mMap.end();++itor)
		{
			elem_t		val		=	nodeRet[col].mMap[itor->first] - itor->second;

			if( val != 0 )
			{
				nodeRet[col].mMap[itor->first]	=	val;
			}
			else
			{
				nodeRet[col].mMap.erase(itor->first);
			}
		}
	}

	return		NULL;
}

void*		SparseMatrix2::threadMultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&		result		=	*info->opInfo.result;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	operandB.mData;
	map_data_t*			nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[itor->first].mMap.begin();itor2!=nodeB[itor->first].mMap.end();itor2++)
			{
				try
				{
					elem_t		val		=	nodeRet[col].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[col].mMap[itor2->first]	=	val;
					}
					else
					{
						printf("데이터가 0\n");
						nodeRet[col].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	exception	)
				{
					printf("범위 초과\n");
				}
			}
		}
	}

	return	NULL;
}

void*		SparseMatrix2::threadElemMul		(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	elem_t					operandB	=	info->opInfo.elemOperandB;
	SparseMatrix2&		result		=	*info->opInfo.result;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeRet	=	&result.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			try
			{
				elem_t		val			=	itor->second * operandB;

				if( val != 0 )
				{
					nodeRet[col].mMap[itor->first]	=	val;
				}
				else
				{
					printf("데이터가 0\n");
					nodeRet[col].mMap.erase(itor->first);
				}
			}
			catch( std::out_of_range&	exception	)
			{
				printf("범위 초과\n");
			}
		}
	}

	return	NULL;
}

void*		SparseMatrix2::threadTmultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&		result		=	*info->opInfo.result;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];
	map_data_t*			nodeRet	=	result.mData;

	for(size_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[col].mMap.begin();itor2!=nodeB[col].mMap.end();itor2++)
			{
				pthread_mutex_lock(&nodeRet[itor->first].mLock);

				try
				{
					elem_t		val		=	nodeRet[itor->first].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[itor->first].mMap[itor2->first]	=	val;
					}
					else
					{
						printf("데이터가 0\n");
						nodeRet[itor->first].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	exception	)
				{
					printf("범위 초과\n");
				}

				pthread_mutex_unlock(&nodeRet[itor->first].mLock);
			}
		}
	}

	return	NULL;
}

void*		SparseMatrix2::threadCopy			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];

	for(size_t col=0;col<=range;++col)
	{
		nodeA[col].mMap	=	nodeB[col].mMap;
	}

	return	NULL;
}

void*		SparseMatrix2::threadCompare		(	void*	pData	)
{
	bool			flag		=	true;
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];

	try
	{
		for(size_t col=0;col<=range;++col)
		{
			for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
			{
				if( itor->second != nodeB[col].mMap.at(itor->first) )
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
	}
	catch( std::out_of_range&	exception	)
	{
		flag	=	false;
	}

	return	(void*)flag;
}

};
