#pragma once
#include "Types.h"

/*-------------------
	RW SpinLock
--------------------*/

// MMORPG에서는 경합 상태가 일시적인 경우가 많기 때문에,
// Event 방식이 아닌 SpinLock 방식으로 구현하는 것이 더 효율적

/*-------------------------------------------
32bit 변수 활용
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
상위 16비트와 하위 16비트의 의미가 각각 다름
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
--------------------------------------------*/

// 동일한 쓰레드 내부에서
// W -> W (O)
// W -> R (O) (W를 잡은 상태에서 R 잡을 수 있음)
// R -> W (X) (R만 잡은 상테에선 W 잡을 수 없음)
// R -> R (O)

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000, // '는 아무 의미 없고, 코드 분리 표시
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

// RAII 패턴
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