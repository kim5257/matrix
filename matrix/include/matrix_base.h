/*
 * matrix_base.h
 *
 *  Created on: 2015. 2. 16.
 *      Author: asran
 */

#ifndef INCLUDE_MATRIX_BASE_H_
#define INCLUDE_MATRIX_BASE_H_

#include <stdio.h>
#include "matrix_typedef.h"

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

class	MatrixBase
{
protected:
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
		const MatrixBase*		mOperandA;
		const MatrixBase*		mOperandB1;
		elem_t					mOperandB2;
		MatrixBase*			mResult;
		THREAD_RETURN_TYPE	mRetVal;
	};

	struct		FuncInfo
	{
		MatrixBase::OpInfo		opInfo;
		Operation					func;
		col_t						startCol;
		col_t						endCol;
	};
private:
	col_t			mCol;		///< 행 값
	row_t			mRow;		///< 열 값
public:
	MatrixBase		(	void	);
	MatrixBase		(	col_t		col,
						row_t		row
					);
	MatrixBase		(	const MatrixBase&		matrix		);
	virtual	~MatrixBase		(	void	);
public:
	inline bool	isValid		(	void	);
	inline col_t	getCol			(	void	) const;
	inline row_t	getRow			(	void	) const;
protected:
	virtual void		allocElems		(	col_t		col,
											row_t		row
										)=0;
	virtual void		freeElems		(	void	)=0;
	virtual void		copyElems		(	const MatrixBase&		matrix		)=0;
private:
	void		pcopyElems		(	const MatrixBase&		matrix		);
private:
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								) const;
	void		doThreadFunc	(	FuncKind		kind,
									OpInfo&		info
								);
};

bool		MatrixBase::isValid		(	void	)
{
	bool	ret		=	false;

	if( (mCol != 0) &&
		(mRow != 0) )
	{
		ret		=	true;
	}

	return	ret;
}

col_t		MatrixBase::getCol		(	void	) const
{
	return	mCol;
}

row_t		MatrixBase::getRow		(	void	) const
{
	return	mRow;
}

}

#endif /* INCLUDE_MATRIX_BASE_H_ */
