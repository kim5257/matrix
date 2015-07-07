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

#define	MAX_ERR_STRING_LEN	(255)		///< 에러 메시지 최대 길이


/**
 * 에러 메시지 전달 클래스
 */
class	ErrMsg
{
private:
	char*		mErrString;		///< 에러 메시지
private:
	inline				ErrMsg		(	void	);
	inline virtual	~ErrMsg		(	void	);
private:
	inline bool		setErrString		(	const char		string[]	);
	inline void		delErrString		(	void	);
public:
	inline const char*		getErrString		(	void	);
public:
	inline static ErrMsg*	createErrMsg		(	const char	string[]	);
	inline static void		destroyErrMsg		(	ErrMsg*		errMsg		);
};

/**
 * 생성자
 */
ErrMsg::ErrMsg	(	void	)
:mErrString(NULL)
{

}

/**
 * 소멸자
 */
ErrMsg::~ErrMsg	(	void	)
{
	delErrString();
}

/**
 * 에러 메시지 문자열 할당 및 메시지 설정
 * @return 에러 메시지 문자열 할당 및 설정에 성공하면 true, 할당에 실패하면 false
 */
bool		ErrMsg::setErrString		(	const char		string[]	///< 에러 메시지
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
 * 에러 메시지 문자열 할당 해제
 */
void		ErrMsg::delErrString		(	void	)
{
	delete[]	mErrString;
}

/**
 * 에러 메시지 가져오기
 * @return 저장 된 에러메시지
 */
const char*	ErrMsg::getErrString		(	void	)
{
	return	mErrString;
}

/**
 * 에러 메시지 전달 객체 생성
 * @return 생성한 에러 메시지 전달 객체
 */
ErrMsg*	ErrMsg::createErrMsg		(	const char		string[]	///< 에러 메시지
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
 * 에러 메시지 전달 객체 제거
 */
void		ErrMsg::destroyErrMsg	(	ErrMsg*	errMsg			///< 에러 메시지 전달 객체
										)
{
	delete	errMsg;
}

};

#endif /* MATRIX_ERROR_H_ */
