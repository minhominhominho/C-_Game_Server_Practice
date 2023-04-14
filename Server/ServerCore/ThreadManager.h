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
	// callback �Լ��� �޴� ���ÿ� �����Ű�� �Լ�
	void Join();
	// std::join�� ���� �����尡 ���������� ��ٷ��ִ� �Լ�

	static void InitTLS();
	// TLS ���� �ʱ�ȭ
	static void DestroyTLS();

	static void DoGlobalQueueWork();
	static void DistributeReservedJobs();

private:
	Mutex _lock;
	vector<thread> _threads;
	// ������ ���
};

