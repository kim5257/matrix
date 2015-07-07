/*
 * test.h
 *
 *  Created on: 2015. 6. 16.
 *      Author: asran
 */

#ifndef INCLUDE_TEST_H_
#define INCLUDE_TEST_H_

#include <stddef.h>

namespace test
{

class	Test
{
public:
				Test		(	void	);
	virtual	~Test		(	void	);
public:
	static size_t		calcMemUsage	(	void	);
};

};

#endif /* INCLUDE_TEST_H_ */
