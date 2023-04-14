#pragma once
// Main에서 ThreadManager를 new, delete 하는 것은 번거롭고 불편함
// 싱글톤으로 하는 방법도 있지만, 여기선 CoreGlobal에서 관리되도록 구성

// 전방 선언을 함과 동시에 ThreadManager를 하나 만들어서 관리
// extern 키워드로 GThreadManager를 전역변수처럼 사용
extern class ThreadManager*		GThreadManager;
extern class Memory*			GMemory;
extern class SendBufferManager*	GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class JobTimer*			GJobTimer;

extern class DeadLockProfiler*	GDeadLockProfiler;
extern class DBConnectionPool*	GDBConnectionPool;
extern class ConsoleLog*		GConsoleLogger;