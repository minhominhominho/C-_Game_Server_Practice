#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "Memory.h"
#include "DeadLockProfiler.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"
#include "ConsoleLog.h"

ThreadManager*		GThreadManager = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
JobTimer*			GJobTimer = nullptr;

DeadLockProfiler*	GDeadLockProfiler = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;
ConsoleLog*			GConsoleLogger = nullptr;

// 나중에 매니저가 여러 개 등장할 수 있는데, 
// 그 순서를 관리하기 위해서 CoreGlobal class를 만듦
// CoreGlobal.h에 있던 CoreGlobal class를 여기에 넣어도 됨
// -> Main에서 CoreGlobal core; 선언조차 안해도 되게끔 하기 위해
// 즉 컨텐츠에서 CoreGlobal core를 꺼내서 쓰는 것이 아니라 엔진 내부에서 처리하도록 하기 위함
class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
		GDeadLockProfiler = new DeadLockProfiler();
		GDBConnectionPool = new DBConnectionPool();
		GConsoleLogger = new ConsoleLog();
		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GSendBufferManager;
		delete GGlobalQueue;
		delete GJobTimer;
		delete GDeadLockProfiler;
		delete GDBConnectionPool;
		delete GConsoleLogger;
		SocketUtils::Clear();
	}
} GCoreGlobal; // class 선언과 동시에 전역 객체 생성