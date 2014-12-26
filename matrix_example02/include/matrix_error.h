/*
 * matrix_error.h
 *
 *  Created on: 2014. 12. 23.
 *      Author: asran
 */

#ifndef MATRIX_ERROR_H_
#define MATRIX_ERROR_H_

#include <stdio.h>
#include <string.h>

namespace	matrix
{

#define	MAX_ERR_STRING_LEN	(255)

class	ErrMsg
{
private:
	char*		errString;
private:
	inline				ErrMsg		(	void	);
	inline virtual	~ErrMsg	(	void	);
private:
	inline bool	setErrString		(	const char		string[]	);
	inline void	delErrString		(	void	);
public:
	inline const char*	getErrString		(	void	);
public:
	inline static ErrMsg*	createErrMsg	(	const char		string[]	);
	inline static void		destroyErrMsg	(	ErrMsg*	errMsg			);
};

ErrMsg::ErrMsg	(	void	)
:errString(NULL)
{

}

ErrMsg::~ErrMsg	(	void	)
{
	delErrString();
}

bool		ErrMsg::setErrString		(	const char		string[]	)
{
	bool	ret	=	false;

	do
	{
		int		length		=	strnlen(string, MAX_ERR_STRING_LEN);
		if( length == 0 )
		{
			break;
		}

		errString	=	new char[length+1];
		if( errString == NULL )
		{
			break;
		}

		memcpy(errString, string, length+1);

		ret		=	true;
	}while(0);

	return	ret;
}

void		ErrMsg::delErrString		(	void	)
{
	delete[]	errString;
}

const char*	ErrMsg::getErrString		(	void	)
{
	return	errString;
}

ErrMsg*	ErrMsg::createErrMsg		(	const char		string[]	)
{
	ErrMsg*	errMsg	=	NULL;

	errMsg	=	new ErrMsg();
	errMsg->setErrString(string);

	return	errMsg;
}

void		ErrMsg::destroyErrMsg	(	ErrMsg*	errMsg			)
{
	delete	errMsg;
}

};

#endif /* MATRIX_ERROR_H_ */
