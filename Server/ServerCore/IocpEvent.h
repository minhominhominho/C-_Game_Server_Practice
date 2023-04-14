#pragma once
class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	// PreRecv, // 0 byte recv
	Recv,
	Send
};

/*---------------------
		IocpEvent
----------------------*/

// Virtual 함수를 사용하면, 
// Virtual Table 때문에 첫 주소가 Overlapped가 아니게 될 수도 있음
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void			Init();

public:
	EventType		eventType;
	IocpObjectRef	owner;
};

/*---------------------
	   ConnectEvent
----------------------*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() :IocpEvent(EventType::Connect) { }
};

/*---------------------
	   DisconnectEvent
----------------------*/

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() :IocpEvent(EventType::Disconnect) { }
};

/*---------------------
	   AcceptEvent
----------------------*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() :IocpEvent(EventType::Accept) { }

public:
	// 추가적인 정보가 필요하다 <- AcceptEx() 함수 특성상
	SessionRef session = nullptr;
};

/*---------------------
	   RecvEvent
----------------------*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() :IocpEvent(EventType::Recv) { }
};

/*---------------------
	   SendEvent
----------------------*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() :IocpEvent(EventType::Send) { }

	Vector<SendBufferRef> sendBuffers;
};