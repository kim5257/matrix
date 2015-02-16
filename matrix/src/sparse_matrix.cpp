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
#define	THREAD_NUM				(8)

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
 * ����
 */
SparseMatrix::SparseMatrix			(	void	)
:mCol(0),
mRow(0),
mData(NULL)
{
}

/**
 * ����
 */
SparseMatrix::SparseMatrix			(	col_t		col,	///< �� ũ��
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
SparseMatrix::SparseMatrix			(	const SparseMatrix&		matrix		///< ���� �� ��ü
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
SparseMatrix::~SparseMatrix			(	void	)
{
	freeElems();
}

/**
 * ��� ��� �� ����
 * @return		������ ��� ��� ��
 */
elem_t		SparseMatrix::getElem		(	col_t		col,	///< ���� �� �� ��ġ
											row_t		row		///< ���� �� �� ��ġ
										) const
{
	chkBound(col, row);

	return	SparseMatrix::getElem_(mData, col, row);
}

/**
 * ��� ��� �� ����
 */
void		SparseMatrix::setElem		(	col_t		col,	///< ���� �� �� ��ġ
												row_t		row,	///< ���� �� �� ��ġ
												elem_t		elem	///< ���� �� ��� ��
											)
{
	chkBound(col, row);

	SparseMatrix::setElem_(mData, col, row, elem);
}

/**
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ������ ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::padd	(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::sub		(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ������ ��� ���� ���
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::psub	(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::multiply	(	const SparseMatrix&	operand	///< �ǿ�����
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
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
 * ������ ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::pmultiply	(	const SparseMatrix&	operand
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
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
 * ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::multiply	(	elem_t		operand	///< �ǿ�����
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
 * ������ ��� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::pmultiply	(	elem_t		operand	///< �ǿ�����
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
 * ��ġ ��� ��ȯ �� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::tmultiply	(	const SparseMatrix&	operand	///< �ǿ�����
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
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
 * ������ ��ġ ��� ��ȯ �� ����
 * @return		��� ���� ���
 */
SparseMatrix	SparseMatrix::ptmultiply	(	const SparseMatrix&	operand	///< �ǿ�����
												) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("��� ũ�Ⱑ �ùٸ��� �ʽ��ϴ�.");
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
 * ��� ����
 * @return		���� �� ���
 */
const SparseMatrix&		SparseMatrix::equal			(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ������ ��� ����
 * @return		���� �� ���
 */
const SparseMatrix&		SparseMatrix::pequal			(	const SparseMatrix&	operand	///< �ǿ�����
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
 * ��� �� ����
 * @return		�� ����� ��ġ�ϸ� true, �� ��ġ�ϸ� false
 */
bool			SparseMatrix::compare		(	const SparseMatrix&	operand
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
 * ������ ��� �� ����
 * @return		�� ����� ��ġ�ϸ� true, �� ��ġ�ϸ� false
 */
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
SparseMatrix		SparseMatrix::solution		(	const SparseMatrix&	operand	///< �ǿ�����
													)
{
	SparseMatrix		x			=	SparseMatrix(this->getRow(), operand.getRow());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.ptmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<3;cnt++)
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
void		SparseMatrix::allocElems		(	col_t		col,	///< �� ũ��
												row_t		row		///< �� ũ��
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
 * ��� ������ �� �Ҵ� ����
 */
void		SparseMatrix::freeElems		(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

/**
 * ��� ������ ����
 */
void		SparseMatrix::copyElems		(	const SparseMatrix&		matrix		///< ���� �� ���
											)
{
	for(size_t col=0;col<getCol();++col)
	{
		mData[col].mVector	=	matrix.mData[col].mVector;
	}
}

/**
 * ������ ��� ������ ����
 */
void		SparseMatrix::pcopyElems		(	const SparseMatrix&		matrix		///< ���� �� ���
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
 * ���� ũ���� ������� �˻�
 * @exception		����� ���� ũ�Ⱑ �ƴ� ��� ���� �߻�
 */
void		SparseMatrix::chkSameSize	(	const SparseMatrix&		matrix		///< �� �� ���
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
void		SparseMatrix::chkBound		(	col_t		col,	///< ���� �� �� ��ġ
											row_t		row		///< ���� �� �� ��ġ
										) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("������ �Ѿ�� �����Դϴ�.");
	}
}

/**
 * ������ ���� ����
 */
void		SparseMatrix::doThreadFunc	(	FuncKind	kind,	///< ���� ����
											OpInfo&		info	///< ���� ���� ������
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
 * ������ ���� ����
 */
void		SparseMatrix::doThreadFunc	(	FuncKind	kind,	///< ���� ����
											OpInfo&		info	///< ���� ���� ������
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
 * ������ ������ �࿡ ���� ��� ����
 * return		�׻� NULL�� ����
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
						// ���� ���� 0�̶�� vector���� ����
						// ���� �Ŀ� itor ���� vector�� ���� ������ �������� �ʴ� ����
						// ���� break�� �ݺ������� ���������Ƿ� ���ʿ��� �����̴�.
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
 * ������ ������ �࿡ ���� ��� ����
 * return		�׻� NULL�� ����
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
						// ���� ���� 0�̶�� vector���� ����
						// ���� �Ŀ� itor ���� vector�� ���� ������ �������� �ʴ� ����
						// ���� break�� �ݺ������� ���������Ƿ� ���ʿ��� �����̴�.
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
 * ������ ������ �࿡ ���� ��� ����
 * return		�׻� NULL�� ����
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
					printf("�����Ͱ� 0\n");
					SparseMatrix::delElem_(nodeRet, col, itor2->mRow);
				}
			}
		}
	}

	return	NULL;
}

/**
 * ������ ������ �࿡ ���� ��� ����
 * return		�׻� NULL�� ����
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
				printf("�����Ͱ� 0\n");
				SparseMatrix::delElem_(nodeRet, col, itor->mRow);
			}
		}
	}

	return	NULL;
}

/**
 * ������ ������ �࿡ ���� ��ġ��� ��ȯ �� ����
 * return		�׻� NULL�� ����
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
				// ��ġ ��� ������ �и� �� �� �����帶��
				// nodeRet�� ���� ���� ���� �� �� �־� lock�� ����Ͽ� ���� ����
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
 * ������ ������ ���� ����
 * return		�׻� NULL�� ����
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
 * ������ ���� �� �࿡ ���ؼ� �� ����
 * return		�� ��� void* ������ ��ȯ�Ͽ� ����
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
 * ��� ������ ����
 */
void		SparseMatrix::delElem_		(	vector_data_t*	data,	///< vector ��ü �迭
											col_t			col,	///< ���� �� ������ ��
											row_t			row		///< ���� �� ������ ��
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
 * ��� ������ ����
 */
elem_t		SparseMatrix::getElem_		(	vector_data_t*	data,	///< vector ��ü �迭
											col_t			col,	///< ���� �� ������ ��
											row_t			row		///< ���� �� ������ ��
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
 * ��� ������ ����
 */
void		SparseMatrix::setElem_		(	vector_data_t*	data,	///< vector ��ü �迭
											col_t			col,	///< �߰� �� ������ ��
											row_t			row,	///< �߰� �� ������ ��
											elem_t			elem
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
				// ���� ���� 0�̶�� vector���� ����
				// ���� �Ŀ� itor ���� vector�� ���� ������ �������� �ʴ� ����
				// ���� break�� �ݺ������� ���������Ƿ� ���ʿ��� �����̴�.
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
