/*
 * setting.h
 *
 *  Created on: 2015. 2. 3.
 *      Author: asran
 */

#ifndef INCLUDE_SETTING_H_
#define INCLUDE_SETTING_H_

//#define	MATRIX
//#define	MATRIX_CSR
//#define	MATRIX_VECTOR
#define	MATRIX_MAP

#define	COL_SIZE				(1000)
#define	ROW_SIZE				(1000)

#define	VAL_RANGE_START		(1)
#define	VAL_RANGE_END			(10)

#define	VAL_PER_COL			(6)

#ifdef	MATRIX

#include <matrix.h>
typedef	matrix::Matrix			matrix_t;

#define	TEST_MULTI_THREAD			(0)

#endif

#ifdef	MATRIX_CSR

#include <matrix_csr.h>
typedef	matrix::MatrixCSR			matrix_t;

#define	TEST_MULTI_THREAD			(0)

#endif

#ifdef	MATRIX_MAP

#include <sparse_matrix2.h>
typedef	matrix::SparseMatrix2	matrix_t;

#define	TEST_MULTI_THREAD			(1)

#endif

#ifdef	MATRIX_VECTOR

#include <sparse_matrix.h>
typedef	matrix::SparseMatrix		matrix_t;

#define	TEST_MULTI_THREAD			(1)

#endif

#include "matrix_error.h"

#endif /* INCLUDE_SETTING_H_ */
