/*
 * sparse_matrix2.h
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace	matrix
{

/**
 * 占쏙옙占�占쏙옙占�표占쏙옙 클占쏙옙占쏙옙
 */
class	SparseMatrix2
{
public:
	enum	FuncKind
	{
		FUNC_ADD,
		FUNC_SUB,
		FUNC_MULTIPLY,
		FUNC_ELEM_MUL,
		FUNC_PMULTIPLY,
		FUNC_COPY,
		FUNC_COMPARE,
	};
	struct		OpInfo
	{
		const SparseMatrix2*	operandA;
		const SparseMatrix2*	operandB;
		elem_t					elemOperandB;
		SparseMatrix2*			result;
		THREAD_RETURN_TYPE		retVal;
	};
private:
	col_t			mCol;		///< 占쏙옙 크占쏙옙
	row_t			mRow;		///< 占쏙옙 크占쏙옙
	map_data_t*		mData;		///< 占쏙옙占�占쏙옙占쏙옙占쏙옙
public:
				SparseMatrix2			(	void	);
				SparseMatrix2			(	col_t		col,
											row_t		row
										);
				SparseMatrix2			(	const SparseMatrix2&		matrix		);
	virtual	~SparseMatrix2			(	void	);
public:
	elem_t		getElem		(	col_t		col,
									row_t		row
								) const;
	void		setElem		(	col_t		col,
									row_t		row,
									elem_t		elem
								);
	SparseMatrix2	add			(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	padd		(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	sub			(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	psub		(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	multiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	pmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	multiply	(	elem_t		operand	) const;
	SparseMatrix2	pmultiply	(	elem_t		operand	) const;
	SparseMatrix2	tmultiply	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	ptmultiply	(	const SparseMatrix2&	operand	) const;
	const SparseMatrix2&		equal		(	const SparseMatrix2&	operand	);
	const SparseMatrix2&		pequal		(	const SparseMatrix2&	operand	);
	bool			compare	(	const SparseMatrix2&	operand	) const;
	bool			pcompare	(	const SparseMatrix2&	operand	) const;
	SparseMatrix2	solution	(	const SparseMatrix2&	operand	);
public:
	inline SparseMatrix2		operator+		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator-		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	const SparseMatrix2&	operand	) const;
	inline SparseMatrix2		operator*		(	elem_t		operand		) const;
	inline const SparseMatrix2&		operator=		(	const SparseMatrix2&	operand	);
	inline bool	operator==		(	const SparseMatrix2&	operand	) const;
public:
	inline bool	isValid		(	void	);
	inline col_t	getCol			(	void	) const;
	inline row_t	getRow			(	void	) const;
	inline size_t	getSize		(	void	) const;
private:
	void		allocElems		(	col_t		col,
									row_t		row
								);
	void		freeElems		(	void	);
	void		copyElems		(	const SparseMatrix2&		matrix		);
	void		pcopyElems		(	const SparseMatrix2&		matrix		);
	void		chkSameSize	(	const SparseMatrix2&		matrix		) const;
	void		chkBound		(	size_t		col,
									size_t		row
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								);
private:
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadFunc			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadAdd			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadSub			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadMultiply	(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadElemMul		(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadTmultiply	(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCopy			(	void*	pData	);
	static THREAD_RETURN_TYPE THREAD_FUNC_TYPE	threadCompare		(	void*	pData	);
};

/**
 * 占쏙옙占�占쏙옙占쏙옙
 * @return		占쏙옙占�占쏙옙占쏙옙 占쏙옙占� */
SparseMatrix2		SparseMatrix2::operator+		(	const SparseMatrix2&	operand	///< 占실울옙占쏙옙占쏙옙
													) const
{
	return	add(operand);
}

/**
 * 占쏙옙占�占쏙옙占쏙옙
 * @return		占쏙옙占�占쏙옙占쏙옙 占쏙옙占� */
SparseMatrix2		SparseMatrix2::operator-		(	const SparseMatrix2&	operand	///< 占실울옙占쏙옙占쏙옙
													) const
{
	return	sub(operand);
}

/**
 * 占쏙옙占�占쏙옙占쏙옙
 * @return		占쏙옙占�占쏙옙占쏙옙 占쏙옙占� */
SparseMatrix2		SparseMatrix2::operator*		(	const SparseMatrix2&	operand	///< 占실울옙占쏙옙占쏙옙
													) const
{
	return	pmultiply(operand);
}

/**
 * 占쏙옙占�占쏙옙占쏙옙
 * @return		占쏙옙占�占쏙옙占쏙옙 占쏙옙占� */
SparseMatrix2		SparseMatrix2::operator*		(	elem_t		operand	///< 占실울옙占쏙옙占쏙옙
													) const
{
	return	multiply(operand);
}

/**
 * 占쏙옙占�占쏙옙占쏙옙
 * @return		占쏙옙占쏙옙 占쏙옙 占쏙옙占� */
const SparseMatrix2&		SparseMatrix2::operator=		(	const SparseMatrix2&	operand	///< 占실울옙占쏙옙占쏙옙
															)
{
	return	equal(operand);
}

/**
 * 占쏙옙占�占쏙옙
 * @return		占쏙옙 占쏙옙占� */
bool	SparseMatrix2::operator==		(	const SparseMatrix2&	operand	///< 占실울옙占쏙옙占쏙옙
										) const
{
	return	compare(operand);
}

/**
 * 占쏙옙占�占쏙옙체占쏙옙 占쏙옙효占쏙옙占쏙옙 占싯삼옙
 * @return		占쏙옙占�占쏙옙체占쏙옙 占쏙옙효占싹몌옙 true, 占쏙옙효占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 false
 */
bool	SparseMatrix2::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mCol != 0) &&
		(mRow != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

/**
 * 占쏙옙 크占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙
 * @return		占쏙옙 크占쏙옙
 */
col_t	SparseMatrix2::getCol			(	void	) const
{
	return	mCol;
}

/**
 * 占쏙옙 크占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙
 * @return		占쏙옙 크占쏙옙
 */
row_t	SparseMatrix2::getRow			(	void	) const
{
	return	mRow;
}

/**
 * 占쏙옙占�占쏙옙占�占쏙옙占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙
 * @return		占쏙옙占�占쏙옙占쏙옙占쏙옙 크占쏙옙
 */
size_t	SparseMatrix2::getSize		(	void	) const
{
	size_t		sum		=	0;

	for(size_t cnt=0;cnt<getCol();cnt++)
	{
		sum		+=	mData[cnt].mMap.size();
	}

	return	sum;
}

}

#endif /* SPARSE_MATRIX_H_ */
