/*
 * matrix.h
 *
 *  Created on: 2014. 12. 23.
 *      Author: asran
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stddef.h>

typedef	int		elem_t;

class	Matrix
{
private:
	elem_t*	mMatrix;
	size_t		mCol;
	size_t		mRow;
private:
				Matrix		(	void	);
	virtual	~Matrix	(	void	);
private:
	bool		allocElems			(	size_t		col,
										size_t		row
									);
	void		deallocMatrix		(	void	);
public:
	void		setValue			(	size_t		col,
										size_t		row,
										elem_t		value
									);
	elem_t		getValue			(	size_t		col,
										size_t		row
									);
	void		equal				(	Matrix*	matrix		);
public:
	void		add					(	Matrix*	operand,
										Matrix*	result
									);
	void		sub					(	Matrix*	operand,
										Matrix*	result
									);
	void		multiply			(	Matrix*	operand,
										Matrix*	result
									);
	void		multiply			(	elem_t		operand,
										Matrix*	result
									);
	void		transpose			(	Matrix*	result		);
	void		solution			(	Matrix*	operand,
										Matrix*	result
									);
public:
	void		chkBound			(	size_t		col,
										size_t		row
									);
	void		chkSameSize		(	Matrix*	matrix		);
public:
	inline size_t		getRow		(	void	);
	inline size_t		getCol		(	void	);
private:
	static Matrix*	allocMatrix		(	void	);
public:
	static Matrix*	createMatrix		(	size_t		col,
												size_t		row
											);
	static Matrix*	createMatrix		(	Matrix*	matrix		);
	static void		destroyMatrix		(	Matrix*	matrix		);
};

size_t		Matrix::getRow	(	void	)
{
	return	mRow;
}

size_t		Matrix::getCol	(	void	)
{
	return	mCol;
}

#endif /* MATRIX_H_ */