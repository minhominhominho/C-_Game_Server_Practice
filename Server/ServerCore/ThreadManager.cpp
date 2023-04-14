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

// function<void(void)> callback <- 람다, 함수 포인터 등 온갖 함수 형태를 다 받아줄 수 있음
// input은 void, output도 void 인 형태의 callback 함수
void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
	// 람다식으로 쓰레드 실행 -> TLS 초기화 -> 콜백으로 넘겨준 함수 실행 -> TLS 정리
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
	// LThreadId 번호를 순차적으로 발급
}

void ThreadManager::DestroyTLS()
{
	// 나중에 InitTLS에서 동적으로 사용되는 데이터가 있으면, 여기서 정리
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
