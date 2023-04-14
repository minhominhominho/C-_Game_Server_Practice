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

	// 1. ������ �����尡 lock�� ����(write�� �ϴ� ����)�ϰ� �ִٸ�, ������ ����
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	// ���� Lock�� ��� �ִ� �������� id
	if (LThreadId == lockThreadID)
	{
		_writeCount++;
		return;
	}

	// 2. �ƹ��� ����(write�� �ϴ� ����) �� �����ϰ� ���� ���� �� �����ؼ� �������� ��´�.
	//if (_lockFlag == EMPTY_FLAG)
	//{
	//	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	//	_lockFlag = desired;
	//}
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	//  & WRITE_THREAD_MASK�� �׳� �������� ����
	// ((LThreadId << 16)�� �̹� ���� 16��Ʈ�� 0
	while (true)
	{
		for (uint32 spinCount = 0;spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			// expected ���� �ٲ� ���� �����ϱ� OUT���� ǥ��
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// ���ϴ� ��� _lockFlag�� �ٲ��� ���

				_writeCount++;
				// ��������� lock ȣ���� ����ϱ� ���ؼ�, 
				// lock�� �ѹ� �� ȣ�� ���� ��, ũ���� ��� _writeCount�� �÷���

				return;
			}
		}

		// �ʹ� �ð��� ���� �ɸ��� ������ �ִ� ������ �Ǵ�, �ǵ������� ũ����
		// ����� �Ǵ� ���������� ������ ���ɼ��� ����
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
		// ������ ��������
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// ReadLock �� Ǯ�� ������ WriteUnlock�� �Ұ���!
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

	// 1. ������ �����尡 lock�� ����(write�� �ϴ� ����)�ϰ� �ִٸ�, ������ ����
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadID)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 2. �ƹ��� ����(write�� �ϴ� ����)�ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���.
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT;spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			// ���⼭ if���� ���� ���ϴ� ����
				// 1. _lockFlag�� W �κп� ���� �ִ� ���
				//	// ��, �̹� �ٸ� write lock�� �����ִ� ���
				// 2. _lockFlag�� R �κ��� �� ���̿� �ٲ��� �ٽ� expected�� �����ؾ� �ϴ� ���
					// ��, �� ���̿� �ٸ� ReadLock()�� ����� ���
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		// �ʹ� �ð��� ���� �ɸ��� ������ �ִ� ������ �Ǵ�, �ǵ������� ũ����
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
		// ������ ��������
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// �̹� _lockFlag�� R �κ��� 0�� ��� Ȯ�� -> �� ��� ������ �ִ� ��Ȳ
	// fetch_sub()�� ���� ���µ�, ��ȯ�� �� ���� ���� ��ȯ
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE_UNLOCK");
	// ���� �Ͼ�� ���� CRASH��
}
