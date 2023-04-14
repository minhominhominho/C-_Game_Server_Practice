#pragma once

/*---------------------
		IocpObject
----------------------*/

class IocpObject : public enable_shared_from_this<IocpObject>
{
	// enable_shared_from_this<IocpObject> 상속받으면
	// weak_ptr<IocpObject> _Wptr; <- 사실상 이런 코드가 들어있는 셈(weak ptr)
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytews = 0) abstract;
};


/*---------------------
	    IocpCore
----------------------*/


class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE	GetHandle() { return _iocpHandle; };

	bool	Register(IocpObjectRef iocpObject);
	bool	Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};