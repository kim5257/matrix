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
#define	THREAD_NUM				(8)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix2::OpInfo	opInfo;
	Operation					func;
	size_t						startCol;
	size_t						endCol;
};

/**
 * ����
 */
SparseMatrix2::SparseMatrix2			(	void	)
:mCol(0),
mRow(0),
mData(NULL)
{
}

/**
 * ����
 */
SparseMatrix2::SparseMatrix2			(	col_t		col,	///< �� ũ��
											row_t		row		///< �� ũ��
										)
:mCol(0),
mRow(0),
mData(NULL)
{
	allocElems(col, row);
}

/**
 * ���� ����
 */
SparseMatrix2::SparseMatrix2			(	const SparseMatrix2&		matrix		///< ���� �� ��ü
											)
:mCol(0),
mRow(0),
mData(NULL)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * �Ҹ���
 */
SparseMatrix2::~SparseMatrix2			(	void	)
{
	freeElems();
}

/**
 * ��� ��� �� ����
 * @return		������ ��� ��� ��
 */
elem_t		SparseMatrix2::getElem		(	col_t		col,	///< ���� �� �� ��ġ
												row_t		row		///< ���� �� �� ��ġ
											) const
{
	chkBound(col, row);

	elem_t		value	=	0;

	try
	{
		value	=	mData[col].mMap.at((unsigned int)row);
	}
	catch( std::out_of_range&	)
	{
		value	=	0;
	}

	return	value;
}

/**
 * ��� ��� �� ����
 */
void		SparseMatrix2::setElem			(	col_t		col,	///< ���� �� �� ��ġ
												row_t		row,	///< ���� �� �� ��ġ
												elem_t		elem	///< ���� �� ��� ��
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix2	SparseMatrix2::add		(	const SparseMatrix2&	operand	///< �ǿ�����
											) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	result		=	*this;

	for(col_t col=0;col<operand.getCol();++col)
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

SparseMatrix2	SparseMatrix2::padd	(	const SparseMatrix2&	operand	///< �ǿ�����
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(col_t col=0;col<operand.getCol();++col)
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix2	SparseMatrix2::sub	(	const SparseMatrix2&	operand	///< �ǿ�����
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	result		=	*this;

	for(col_t col=0;col<operand.getCol();++col)
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

SparseMatrix2	SparseMatrix2::psub	(	const SparseMatrix2&	operand	///< �ǿ�����
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(col_t col=0;col<operand.getCol();++col)
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix2	SparseMatrix2::multiply	(	const SparseMatrix2&	operand	///< �ǿ�����
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	for(col_t col=0;col<getCol();++col)
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
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix2	SparseMatrix2::multiply	(	elem_t		operand	///< �ǿ�����
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getCol(), getRow());

	for(col_t col=0;col<getCol();++col)
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

SparseMatrix2	SparseMatrix2::pmultiply	(	elem_t		operand	///< �ǿ�����
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getCol(), getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
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
 * ��ġ ��� ��ȯ �� ����
 * @return		��� ���� ���
 */
SparseMatrix2	SparseMatrix2::tmultiply	(	const SparseMatrix2&	operand	///< �ǿ�����
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	for(col_t col=0;col<getCol();++col)
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

SparseMatrix2	SparseMatrix2::ptmultiply	(	const SparseMatrix2&	operand	///< �ǿ�����
												) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getCol(), operand.getRow());

	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
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
 * ��� ����
 * @return		���� �� ���
 */
const SparseMatrix2&		SparseMatrix2::equal			(	const SparseMatrix2&	operand	///< �ǿ�����
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

const SparseMatrix2&		SparseMatrix2::pequal		(	const SparseMatrix2&	operand	///< �ǿ�����
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

bool			SparseMatrix2::compare		(	const SparseMatrix2&	operand
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(col_t col=0;col<getCol();++col)
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
			for(col_t col=0;col<getCol();++col)
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
 * ��� ������ �� ���
 * @return		�� ��� ���
 */
SparseMatrix2		SparseMatrix2::solution		(	const SparseMatrix2&	operand	///< �ǿ�����
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
 * ��� ������ �� �Ҵ�
 * @exception		�޸� �Ҵ� ���� �� ���� �߻�
 */
void		SparseMatrix2::allocElems		(	col_t		col,	///< �� ũ��
													row_t		row		///< �� ũ��
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
 * ��� ������ �� �Ҵ� ����
 */
void		SparseMatrix2::freeElems		(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

/**
 * ��� ������ ����
 */
void		SparseMatrix2::copyElems		(	const SparseMatrix2&		matrix		///< ���� �� ���
											)
{
	for(col_t col=0;col<getCol();++col)
	{
		mData[col].mMap.clear();
		mData[col]		=	matrix.mData[col];
	}
}

void		SparseMatrix2::pcopyElems		(	const SparseMatrix2&		matrix		///< ���� �� ���
												)
{
	if( getCol() < THREAD_FUNC_THRESHOLD )
	{
		for(col_t col=0;col<getCol();++col)
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
 * ���� ũ���� ������� �˻�
 * @exception		����� ���� ũ�Ⱑ �ƴ� ��� ���� �߻�
 */
void		SparseMatrix2::chkSameSize	(	const SparseMatrix2&		matrix		///< �� �� ���
											) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
	}
}

/**
 * ��� ��� ���� ���� �˻�
 * @exception		���� ���� ���� ��� ���� �߻�
 */
void		SparseMatrix2::chkBound		(	size_t		col,	///< ���� �� �� ��ġ
												size_t		row		///< ���� �� �� ��ġ
											) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("������ �Ѿ�� �����Դϴ�.");
	}
}

void		SparseMatrix2::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
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

	// Thread ��
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix2::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix2::threadFunc,
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

void		SparseMatrix2::doThreadFunc	(	FuncKind		kind,
												OpInfo&		info
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

	// Thread ��
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix2::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix2::threadFunc,
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

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadFunc		(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadAdd			(	void*	pData	)
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

	for(col_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			nodeRet[col].mMap[itor->first]	=	itor->second;
		}
	}

	for(col_t col=0;col<=range;++col)
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

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadSub			(	void*	pData	)
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

	for(col_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			nodeRet[col].mMap[itor->first]	=	itor->second;
		}
	}

	for(col_t col=0;col<=range;++col)
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

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadMultiply	(	void*	pData	)
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

	for(col_t col=0;col<=range;++col)
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
						printf("�����Ͱ� 0\n");
						nodeRet[col].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	)
				{
					printf("���� �ʰ�\n");
				}
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadElemMul		(	void*	pData	)
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

	for(col_t col=0;col<=range;++col)
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
					printf("�����Ͱ� 0\n");
					nodeRet[col].mMap.erase(itor->first);
				}
			}
			catch( std::out_of_range&	)
			{
				printf("���� �ʰ�\n");
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadTmultiply	(	void*	pData	)
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

	for(col_t col=0;col<=range;++col)
	{
		for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[col].mMap.begin();itor2!=nodeB[col].mMap.end();itor2++)
			{
				LOCK(&nodeRet[itor->first].mLock);

				try
				{
					elem_t		val		=	nodeRet[itor->first].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[itor->first].mMap[itor2->first]	=	val;
					}
					else
					{
						printf("�����Ͱ� 0\n");
						nodeRet[itor->first].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	)
				{
					printf("���� �ʰ�\n");
				}

				UNLOCK(&nodeRet[itor->first].mLock);
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadCopy			(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startCol;
	size_t			end			=	info->endCol;
	size_t			range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;

	map_data_t*			nodeA		=	&operandA.mData[start];
	map_data_t*			nodeB		=	&operandB.mData[start];

	for(col_t col=0;col<=range;++col)
	{
		nodeA[col].mMap	=	nodeB[col].mMap;
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadCompare		(	void*	pData	)
{
	THREAD_RETURN_TYPE	flag		=	(THREAD_RETURN_TYPE)TRUE;
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
		for(col_t col=0;col<=range;++col)
		{
			for(elem_map_itor itor=nodeA[col].mMap.begin();itor!=nodeA[col].mMap.end();++itor)
			{
				if( itor->second != nodeB[col].mMap.at(itor->first) )
				{
					flag		=	(THREAD_RETURN_TYPE)FALSE;
					break;
				}
			}

			if( flag == false )
			{
				break;
			}
		}
	}
	catch( std::out_of_range&	)
	{
		flag	=	(THREAD_RETURN_TYPE)FALSE;
	}

	return	(THREAD_RETURN_TYPE)flag;
}

};
