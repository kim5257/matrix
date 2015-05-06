/*
 * matrix_typedef.h
 *
 *  Created on: 2015. 1. 7.
 *      Author: asran
 */

#ifndef MATRIX_TYPEDEF_H_
#define MATRIX_TYPEDEF_H_

#include <matrix_settings.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <numeric>
#include <algorithm>

#if(PLATFORM == PLATFORM_WINDOWS)

#include <windows.h>
#include <process.h>
#define		MAX_SPIN_COUNT		(4000)

#elif(PLATFORM == PLATFORM_LINUX)

#include <pthread.h>

// TRUE 정의
enum	BOOLEAN
{
	FALSE	=	0,
	TRUE	=	1,
};

#else

#error	Wrong platform setting in setting.h file.

#endif

namespace	matrix
{

typedef	double	elem_t;		///< 요소 데이터 형식

class	node_t
{
public:
	size_t		mCol;
	elem_t		mElem;

	node_t		(	size_t		col,
					elem_t		data
				)
	{
		mCol	=	col;
		mElem	=	data;
	}
public:
	inline bool	operator==		(	size_t		col		) const
	{
		return (mCol == col);
	}
};

typedef	std::vector<node_t>::iterator		elem_vector_itor;	///< 한 개 행 데이터 참조자

struct	vector_node_t
{
	std::vector<node_t>		mVector;

#if(PLATFORM == PLATFORM_WINDOWS)
	CRITICAL_SECTION	mLock;
#elif(PLATFORM == PLATFORM_LINUX)
	pthread_mutex_t		mLock	=	PTHREAD_MUTEX_INITIALIZER;
#endif

	vector_node_t	()
	{
		#if(PLATFORM == PLATFORM_WINDOWS)

		::InitializeCriticalSectionEx	(	&mLock,
											MAX_SPIN_COUNT,
											NULL
										);

		#elif(PLATFORM == PLATFORM_LINUX)

		mLock	=	PTHREAD_MUTEX_INITIALIZER;

		#endif
	}
};

/////////////////////////////

class	node_t2
{
public:
	size_t		mRow;
	elem_t		mElem;

	node_t2	(	size_t		row,
					elem_t		data
				)
	{
		mRow	=	row;
		mElem	=	data;
	}
public:
	inline bool	operator==		(	size_t		row		) const
	{
		return (mRow == row);
	}
};

typedef	std::vector<node_t2>::iterator		elem_vector_itor2;	///< 한 개 행 데이터 참조자

struct	vector_node_t2
{
	std::vector<node_t2>		mVector;

#if(PLATFORM == PLATFORM_WINDOWS)
	CRITICAL_SECTION	mLock;
#elif(PLATFORM == PLATFORM_LINUX)
	pthread_mutex_t		mLock	=	PTHREAD_MUTEX_INITIALIZER;
#endif

	vector_node_t2()
	{
		#if(PLATFORM == PLATFORM_WINDOWS)

		::InitializeCriticalSectionEx	(	&mLock,
											MAX_SPIN_COUNT,
											NULL
										);

		#elif(PLATFORM == PLATFORM_LINUX)

		mLock	=	PTHREAD_MUTEX_INITIALIZER;

		#endif
	}
};

/////////////////////////////


typedef	std::map<size_t, elem_t>::const_iterator	elem_map_itor;		///< 한 개 행 데이터 참조자

struct	map_node_t
{
	std::map<size_t, elem_t>	mMap;

#if(PLATFORM == PLATFORM_WINDOWS)
	CRITICAL_SECTION	mLock;
#elif(PLATFORM == PLATFORM_LINUX)
	pthread_mutex_t		mLock	=	PTHREAD_MUTEX_INITIALIZER;
#endif

	map_node_t	()
	{
		#if(PLATFORM == PLATFORM_WINDOWS)

		::InitializeCriticalSectionEx	(	&mLock,
											MAX_SPIN_COUNT,
											NULL
										);

		#elif(PLATFORM == PLATFORM_LINUX)

		mLock	=	PTHREAD_MUTEX_INITIALIZER;

		#endif
	}
};

#if(PLATFORM == PLATFORM_WINDOWS)

#define		LOCK(lock)		EnterCriticalSection(lock)
#define		UNLOCK(lock)	LeaveCriticalSection(lock)

#define		THREAD_FUNC_TYPE	WINAPI
typedef		unsigned int		THREAD_RETURN_TYPE;

#elif(PLATFORM == PLATFORM_LINUX)

#define		LOCK(lock)		pthread_mutex_lock(lock)
#define		UNLOCK(lock)	pthread_mutex_unlock(lock)

#define		THREAD_FUNC_TYPE
typedef		void*	THREAD_RETURN_TYPE; 

#endif

typedef	std::vector<size_t>			csr_t;			///< 행렬 위치 데이터
typedef	csr_t::const_iterator		csr_itor;		///< 행렬 위치 데이터 참조자

typedef	std::vector<elem_t>			elem_csr_t;		///< 한 개 행 데이터 형식
typedef	elem_csr_t::const_iterator	elem_csr_itor;	///< 한 개 행 데이터 참조자


};



#endif /* MATRIX_TYPEDEF_H_ */
