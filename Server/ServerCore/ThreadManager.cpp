#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"

/*-------------------
	ThreadManager
--------------------*/

ThreadManager::ThreadManager()
{
	// Main Thread
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

// function<void(void)> callback <- ����, �Լ� ������ �� �°� �Լ� ���¸� �� �޾��� �� ����
// input�� void, output�� void �� ������ callback �Լ�
void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
	// ���ٽ����� ������ ���� -> TLS �ʱ�ȭ -> �ݹ����� �Ѱ��� �Լ� ���� -> TLS ����
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
	// LThreadId ��ȣ�� ���������� �߱�
}

void ThreadManager::DestroyTLS()
{
	// ���߿� InitTLS���� �������� ���Ǵ� �����Ͱ� ������, ���⼭ ����
}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;

		JobQueueRef jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;

		jobQueue->Execute();
	}
}

void ThreadManager::DistributeReservedJobs()
{
	const uint64 now = ::GetTickCount64();

	GJobTimer->Distribute(now);
}
