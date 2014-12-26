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
				ErrMsg		(	void	);
	virtual	~ErrMsg	(	void	);
private:
	inline bool	SetErrString		(	const char		string[]	);
	inline void	DelErrString		(	void	);
public:
	inline static ErrMsg*	CreateErrMsg	(	const char		string[]	);
	inline static void		DestroyErrMsg	(	ErrMsg*	errMsg			);
};

ErrMsg::ErrMsg	(	void	)
:errString(NULL)
{

}

ErrMsg::~ErrMsg	(	void	)
{
	DelErrString();
}

bool		ErrMsg::SetErrString		(	const char		string[]	)
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

void		ErrMsg::DelErrString		(	void	)
{
	delete[]	errString;
}

ErrMsg*	ErrMsg::CreateErrMsg		(	const char		string[]	)
{
	ErrMsg*	errMsg	=	NULL;

	return	errMsg;
}

void		ErrMsg::DestroyErrMsg	(	ErrMsg*	errMsg			)
{
	delete	errMsg;
}

};

#endif /* MATRIX_ERROR_H_ */
