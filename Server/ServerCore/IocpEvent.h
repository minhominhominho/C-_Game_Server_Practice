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

// Virtual �Լ��� ����ϸ�, 
// Virtual Table ������ ù �ּҰ� Overlapped�� �ƴϰ� �� ���� ����
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
	// �߰����� ������ �ʿ��ϴ� <- AcceptEx() �Լ� Ư����
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