/*
 * matrix_elem.h
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#ifndef MATRIX_ELEM_H_
#define MATRIX_ELEM_H_

#include <stdio.h>

typedef	double		elem_t;

namespace	matrix
{

class	MatrixElem
{
private:
	MatrixElem*	mColPrev;		///< 세로열 이전 값
	MatrixElem*	mColNext;		///< 세로열 다음 값
	MatrixElem*	mRowPrev;		///< 가로열 이전 값
	MatrixElem*	mRowNext;		///< 가로열 다음 값
private:
	size_t			mColPos;		///< 세로열 위치
	size_t			mRowPos;		///< 가로열	위치
	elem_t			mValue;		///< 데이터
private:
				MatrixElem		(	void	);
	virtual	~MatrixElem	(	void	);
public:
	inline size_t		getColPos		(	void	);
	inline size_t		getRowPos		(	void	);
	inline elem_t		getElem		(	void	);
public:
	static MatrixElem*	insertElem		(	MatrixElem*	colPrev,
												MatrixElem*	rowPrev,
												elem_t			mValue
											);
};

size_t		MatrixElem::getColPos		(	void	)
{
	return	mColPos;
}

size_t		MatrixElem::getRowPos		(	void	)
{
	return	mRowPos;
}

elem_t		MatrixElem::getElem			(	void	)
{
	return	mValue;
}


};

#endif /* MATRIX_ELEM_H_ */
