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
#include <new>

namespace	matrix
{

#define	MAX_ERR_STRING_LEN	(255)		///< �ִ� ���ڿ� ����


/**
 * �����޽��� ��� Ŭ����
 */
class	ErrMsg
{
private:
	char*		mErrString;		///< ���� ���ڿ�
private:
	inline			ErrMsg		(	void	);
	inline virtual	~ErrMsg		(	void	);
private:
	inline bool		setErrString		(	const char		string[]	);
	inline void		delErrString		(	void	);
public:
	inline const char*		getErrString		(	void	);
public:
	inline static ErrMsg*	createErrMsg		(	const char		string[]	);
	inline static void		destroyErrMsg		(	ErrMsg*	errMsg			);
};

/**
 * ����
 */
ErrMsg::ErrMsg	(	void	)
:mErrString(NULL)
{

}

/**
 * �Ҹ���
 */
ErrMsg::~ErrMsg	(	void	)
{
	delErrString();
}

/**
 * ���� �޽��� ���ڿ� �Ҵ� �� �޽��� ����
 * @return		���� �޽��� ���ڿ� �Ҵ� �� ������ �����ϸ� true, �Ҵ翡 �����ϸ� false
 */
bool		ErrMsg::setErrString		(	const char		string[]	///< ���� �޽���
										)
{
	bool	ret	=	false;

	do
	{
		size_t	length		=	strnlen(string, MAX_ERR_STRING_LEN);
		if( length == 0 )
		{
			break;
		}

		try
		{
			mErrString		=	new char[length+1];
		}
		catch( std::bad_alloc&	exception	)
		{
			exception.what();
			break;
		}

		memcpy(mErrString, string, length+1);

		ret		=	true;
	}while(0);

	return	ret;
}

/**
 * ���� �޽��� ���ڿ� �Ҵ� ����
 */
void		ErrMsg::delErrString		(	void	)
{
	delete[]	mErrString;
}

/**
 * ���� �޽��� ��������
 * @return		���� �� �����޽���
 */
const char*	ErrMsg::getErrString		(	void	)
{
	return	mErrString;
}

/**
 * ���� �޽��� ��� ��ü ��
 * @return		���� ���� �޽��� ��� ��ü
 */
ErrMsg*	ErrMsg::createErrMsg		(	const char		string[]	///< ���� �޽���
										)
{
	ErrMsg*	errMsg	=	NULL;

	errMsg	=	new ErrMsg();
	if( errMsg->setErrString(string) == false )
	{
		delete	errMsg;
		errMsg	=	NULL;
	}

	return	errMsg;
}

/**
 * ���� �޽��� ��� ��ü ����
 */
void		ErrMsg::destroyErrMsg	(	ErrMsg*	errMsg			///< ���� �޽��� ��� ��ü
										)
{
	delete	errMsg;
}

};

#endif /* MATRIX_ERROR_H_ */
