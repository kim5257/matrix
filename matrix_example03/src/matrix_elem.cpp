/*
 * matrix_elem.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "matrix_elem.h"
#include "matrix_error.h"
#include <new>

namespace matrix
{

MatrixElem*	MatrixElem::insertElem		(	MatrixElem*	colPrev,
													MatrixElem*	rowPrev,
													elem_t			mValue
												)
{
	MatrixElem*	newElem	=	NULL;

	try
	{
		newElem	=	new MatrixElem();
	}
	catch(	std::bad_alloc&	exception	)
	{
		throw	ErrMsg::createErrMsg(exception.what());
	}

	return	newElem;
}

};
