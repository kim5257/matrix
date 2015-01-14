/*
 * matrix_typedef.h
 *
 *  Created on: 2015. 1. 7.
 *      Author: asran
 */

#ifndef MATRIX_TYPEDEF_H_
#define MATRIX_TYPEDEF_H_

#include <stdint.h>

namespace	matrix
{

typedef	double			elem_t;		///< 요소 데이터 형식
typedef	uint32_t		col_t;			///< 행 위치 데이터 형식
typedef	uint32_t		row_t;			///< 열 위치 데이터 형식

struct		node_t
{
	row_t		mRow;
	elem_t		mData;

	node_t		(	row_t		row,
					elem_t		data
				)
	{
		mRow	=	row;
		mData	=	data;
	}
};

};



#endif /* MATRIX_TYPEDEF_H_ */
