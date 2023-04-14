#pragma once

#include <thread>
#include <functional>

/*-------------------
	ThreadManager
--------------------*/

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void(void)> callback);
	// callback 함수를 받는 동시에 실행시키는 함수
	void Join();
	// std::join과 같이 쓰레드가 끝날때까지 기다려주는 함수

	static void InitTLS();
	// TLS 영역 초기화
	static void DestroyTLS();

	static void DoGlobalQueueWork();
	static void DistributeReservedJobs();

private:
	Mutex _lock;
	vector<thread> _threads;
	// 쓰레드 목록
};

