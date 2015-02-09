/*
 * matrix_typedef.h
 *
 *  Created on: 2015. 1. 7.
 *      Author: asran
 */

#ifndef MATRIX_TYPEDEF_H_
#define MATRIX_TYPEDEF_H_

#include <stdint.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <numeric>
#include <pthread.h>

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

typedef	std::vector<node_t>			elem_vector_t;		///< 한 개 행 데이터 형식
typedef	elem_vector_t::iterator		elem_vector_itor;		///< 한 개 행 데이터 참조자

struct		vector_data_t
{
	elem_vector_t		mVector;
	pthread_mutex_t	mLock		=	PTHREAD_MUTEX_INITIALIZER;

	vector_data_t	()
	{
	}
};

typedef	std::map<row_t, elem_t>		elem_map_t;			///< 한 개 행 데이터 형식
typedef	elem_map_t::const_iterator	elem_map_itor;		///< 한 개 행 데이터 참조자

struct		map_data_t
{
	elem_map_t			mMap;
	pthread_mutex_t	mLock		=	PTHREAD_MUTEX_INITIALIZER;

	map_data_t	()
	{
	}
};

typedef	std::vector<uint32_t>		csr_t;				///< 행렬 위치 데이터
typedef	csr_t::const_iterator		csr_itor;			///< 행렬 위치 데이터 참조자

typedef	std::vector<elem_t>			elem_csr_t;		///< 한 개 행 데이터 형식
typedef	elem_csr_t::const_iterator	elem_csr_itor;	///< 한 개 행 데이터 참조자


};



#endif /* MATRIX_TYPEDEF_H_ */
