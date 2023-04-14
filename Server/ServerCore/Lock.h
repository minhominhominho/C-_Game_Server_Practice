#pragma once
#include "Types.h"

/*-------------------
	RW SpinLock
--------------------*/

// MMORPG������ ���� ���°� �Ͻ����� ��찡 ���� ������,
// Event ����� �ƴ� SpinLock ������� �����ϴ� ���� �� ȿ����

/*-------------------------------------------
32bit ���� Ȱ��
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
���� 16��Ʈ�� ���� 16��Ʈ�� �ǹ̰� ���� �ٸ�
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
--------------------------------------------*/

// ������ ������ ���ο���
// W -> W (O)
// W -> R (O) (W�� ���� ���¿��� R ���� �� ����)
// R -> W (X) (R�� ���� ���׿��� W ���� �� ����)
// R -> R (O)

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000, // '�� �ƹ� �ǹ� ����, �ڵ� �и� ǥ��
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0X0000'0000
	};

public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
};

/*-------------------
	LockGuard
--------------------*/

// RAII ����
class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
	~ReadLockGuard() { _lock.ReadUnlock(_name); }

private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
	Lock& _lock;
	const char* _name;
};