/*
 * sparse_matrix3.cpp
 *
 *  Created on: 2015. 1. 12.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_NUM					(8)
#define	THREAD_FUNC_THRESHOLD	(THREAD_NUM)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix::OpInfo	opInfo;
	Operation					func;
	size_t						startRow;
	size_t						endRow;
};

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

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	row,
									itor->mCol,
									result.getElem(row, itor->mCol) + itor->mElem
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

	SparseMatrix	result		=	SparseMatrix(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t row=0;row<operand.getRow();++row)
		{
			std::vector<node_t>&	vec		=	operand.mData[row].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	row,
										itor->mCol,
										result.getElem(row, itor->mCol) + itor->mElem
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
SparseMatrix	SparseMatrix::sub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix		result		=	SparseMatrix(getRow(), getCol());

	result.equal(*this);

	for(size_t row=0;row<operand.getRow();++row)
	{
		std::vector<node_t>&	vec		=	operand.mData[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			result.setElem	(	row,
									itor->mCol,
									result.getElem(row, itor->mCol) - itor->mElem
								);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix	SparseMatrix::psub	(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		result.equal(*this);

		for(size_t row=0;row<operand.getRow();++row)
		{
			std::vector<node_t>&	vec		=	operand.mData[row].mVector;

			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				result.setElem	(	row,
										itor->mCol,
										result.getElem(row, itor->mCol) - itor->mElem
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
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::pmultiply		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	if( ( getRow() != operand.getCol() ) &&
		( getCol() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getRow(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
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
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
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
 * 뒤 전치행렬 변환 후 앞 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::tmultiply		(	const SparseMatrix&	operand	///< 피연산자
												) const
{
	if( ( getRow() != operand.getRow() ) &&
		( getCol() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getRow(), operand.getCol());


	for(size_t row=0;row<getRow();++row)
	{
		std::vector<node_t>&	vec		=	mData[row].mVector;

		if( (row % 100) == 0 )
		{
			printf("row = %lu\n", row);
		}

		for(size_t row2=0;row2<operand.getRow();++row2)
		{
			elem_t		sum		=	0;

			std::vector<node_t>&	vec2	=	operand.mData[row2].mVector;

			size_t itor	=	0;
			size_t itor2	=	0;

			while( (itor<vec.size()) && (itor2<vec2.size()) )
			{
				if( vec[itor].mCol > vec2[itor2].mCol )
				{
					++itor2;
				}
				else if( vec[itor].mCol < vec2[itor2].mCol )
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

			if( sum != 0 )
			{
				result.setElem(row, row2, sum);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 뒤 전치행렬 변환 후 앞 행렬 곱셈
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

	SparseMatrix	result	=	SparseMatrix(getRow(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
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
 * 쓰레드 앞 전치행렬 변환 후 뒤 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::pstmultiply	(	const SparseMatrix&	operand	) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
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
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_SPMULTIPLY, info);
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
 * 쓰레드 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::pequal	(	const SparseMatrix&	operand	///< 피연산자
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
 * 쓰레드 행렬 비교 연산
 * @return 두 행렬이 일치하면 true, 비 일치하면 false
 */
bool			SparseMatrix::pcompare		(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		if( getRow() < THREAD_FUNC_THRESHOLD )
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
	SparseMatrix		x			=	SparseMatrix(this->getCol(), operand.getCol());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.pstmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<32;cnt++)
	{
		SparseMatrix	ap		=	(*this) * p;

		elem_t			ptval	=	(p.pstmultiply(ap)).getElem(0,0);
		elem_t			alpha	=	rSold.getElem(0,0) / ptval;

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.pstmultiply(r);

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
 * 쓰레드 행렬 데이터 복사
 */
void		SparseMatrix::pcopyElems	(	const SparseMatrix&		matrix		///< 복사 할 행렬
										)
{
	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			mData[row].mVector.clear();
			mData[row]		=	matrix.mData[row];
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
	case FUNC_SPMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix::threadStmultiply;
		break;
	case FUNC_COMPARE:
		orgFuncInfo.func	=	SparseMatrix::threadCompare;
		break;
	default:
		break;
	}

	size_t	threadPerRow	=	getRow() / THREAD_NUM;
	size_t	rowMod			=	getRow() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startRow	=	(size_t)(num * threadPerRow);
		funcInfo[num].endRow		=	funcInfo[num].startRow + threadPerRow - 1;
	}

	funcInfo[THREAD_NUM-1].endRow	+=	rowMod;

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

	size_t	threadPerRow	=	getRow() / THREAD_NUM;
	size_t	rowMod			=	getRow() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startRow	=	(size_t)(num * threadPerRow);
		funcInfo[num].endRow	=	funcInfo[num].startRow + threadPerRow - 1;
	}

	funcInfo[THREAD_NUM-1].endRow	+=	rowMod;

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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet		=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		nodeRet[row]	=	nodeA[row];
	}

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeB[row].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t>&	vec2	=	nodeRet[row].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mCol == itor->mCol )
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
				vec2.push_back(node_t(itor->mCol, itor->mElem));
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		nodeRet[row]	=	nodeA[row];
	}

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeB[row].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			bool	found	=	false;

			std::vector<node_t>&	vec2	=	nodeRet[row].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				if( itor2->mCol == itor->mCol )
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
				vec2.push_back(node_t(itor->mCol, -itor->mElem));
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	operandB.mData;
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeA[row].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	nodeB[itor->mCol].mVector;
			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();++itor2)
			{
				elem_t		val		=	SparseMatrix::getElem_(nodeRet, row, itor2->mCol) + (itor->mElem * itor2->mElem);

				if( val != 0 )
				{
					SparseMatrix::setElem_(nodeRet, row, itor2->mCol, val);
				}
				else
				{
					printf("데이터가 0\n");
					SparseMatrix::delElem_(nodeRet, row, itor2->mCol);
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	elem_t				operandB	=	info->opInfo.elemOperandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeA[row].mVector;
		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			elem_t		val			=	itor->mElem * operandB;

			if( val != 0 )
			{
				SparseMatrix::setElem_(nodeRet, row, itor->mCol, val);
			}
			else
			{
				printf("데이터가 0\n");
				SparseMatrix::delElem_(nodeRet, row, itor->mCol);
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&		result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet		=	result.mData;

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeA[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	nodeB[row].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				// 전치 행렬 곱셈은 분리 된 각 스레드마다
				// nodeRet의 같은 행을 참조 할 수 있어 lock을 사용하여 접근 제어
				LOCK(&nodeRet[itor->mCol].mLock);
				elem_t		val		=	getElem_(nodeRet, itor->mCol, itor2->mCol);

				setElem_	(	nodeRet,
								itor->mCol,
								itor2->mCol,
								val + (itor->mElem * itor2->mElem)
							);
				UNLOCK(&nodeRet[itor->mCol].mLock);
			}
		}
	}

	return	NULL;
}

/**
 * 지정한 범위의 행에 대한 전치행렬 변환 후 곱셈
 * return 항상 NULL을 리턴
 */
THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix::threadStmultiply	(	void*	pData	)
{
	FuncInfo*		info		=	(FuncInfo*)pData;
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;
	SparseMatrix&			result		=	*info->opInfo.result;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];
	vector_node_t*		nodeRet	=	result.mData;

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeA[row].mVector;

		for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
		{
			std::vector<node_t>&	vec2	=	nodeB[row].mVector;

			for(elem_vector_itor itor2=vec2.begin();itor2!=vec2.end();itor2++)
			{
				// 전치 행렬 곱셈은 분리 된 각 스레드마다
				// nodeRet의 같은 행을 참조 할 수 있어 lock을 사용하여 접근 제어
				LOCK(&nodeRet[itor->mCol].mLock);
				elem_t		val		=	getElem_(nodeRet, itor->mCol, itor2->mCol);

				setElem_	(	nodeRet,
								itor->mCol,
								itor2->mCol,
								val + (itor->mElem * itor2->mElem)
							);
				UNLOCK(&nodeRet[itor->mCol].mLock);
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		nodeA[row].mVector	=	nodeB[row].mVector;
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
	size_t			start		=	info->startRow;
	size_t			end			=	info->endRow;
	size_t			range		=	end - start;

	const SparseMatrix&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix&	operandB	=	*info->opInfo.operandB;

	vector_node_t*		nodeA		=	&operandA.mData[start];
	vector_node_t*		nodeB		=	&operandB.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		std::vector<node_t>&	vec		=	nodeA[row].mVector;

		if( vec.size() != nodeB[row].mVector.size() )
		{
			flag	=	false;
		}
		else
		{
			for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
			{
				if( itor->mElem != SparseMatrix::getElem_(nodeB, row, itor->mCol) )
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

	//bool			found	=	false;
	//for(elem_vector_itor itor=vec.begin();itor!=vec.end();++itor)
	//{
	//	if( itor->mCol == row )
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
