#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

/*-------------------
	RW SpinLock
--------------------*/

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 1. 동일한 쓰레드가 lock을 소유(write를 하는 상태)하고 있다면, 무조건 성공
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	// 현재 Lock을 잡고 있는 쓰레드의 id
	if (LThreadId == lockThreadID)
	{
		_writeCount++;
		return;
	}

	// 2. 아무도 소유(write를 하는 상태) 및 공유하고 있지 않을 때 경합해서 소유권을 얻는다.
	//if (_lockFlag == EMPTY_FLAG)
	//{
	//	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	//	_lockFlag = desired;
	//}
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	//  & WRITE_THREAD_MASK는 그냥 가독성을 위함
	// ((LThreadId << 16)로 이미 하위 16비트는 0
	while (true)
	{
		for (uint32 spinCount = 0;spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			// expected 값이 바뀔 수도 있으니까 OUT으로 표시
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// 원하는 대로 _lockFlag를 바꿨을 경우

				_writeCount++;
				// 재귀적으로 lock 호출을 허용하기 위해서, 
				// lock이 한번 더 호출 됐을 때, 크래시 대신 _writeCount를 늘려줌

				return;
			}
		}

		// 너무 시간이 오래 걸리면 문제가 있는 것으로 판단, 의도적으로 크래시
		// 데드락 또는 컨텐츠적인 문제일 가능성이 높음
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
		// 소유권 내려놓음
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// ReadLock 다 풀기 전에는 WriteUnlock은 불가능!
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 1. 동일한 쓰레드가 lock을 소유(write를 하는 상태)하고 있다면, 무조건 성공
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadID)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 2. 아무도 소유(write를 하는 상태)하고 있지 않을 때 경합해서 공유 카운트롤 올린다.
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT;spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			// 여기서 if문에 들어가지 못하는 경우는
				// 1. _lockFlag의 W 부분에 값이 있는 경우
				//	// 즉, 이미 다른 write lock이 잡혀있는 경우
				// 2. _lockFlag의 R 부분이 이 사이에 바껴서 다시 expected를 설정해야 하는 경우
					// 즉, 이 사이에 다른 ReadLock()이 실행된 경우
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		// 너무 시간이 오래 걸리면 문제가 있는 것으로 판단, 의도적으로 크래시
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
		// 소유권 내려놓음
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// 이미 _lockFlag의 R 부분이 0인 경우 확인 -> 이 경우 문제가 있는 상황
	// fetch_sub()는 값을 빼는데, 반환은 그 이전 값을 반환
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE_UNLOCK");
	// 거의 일어나지 않을 CRASH임
}
