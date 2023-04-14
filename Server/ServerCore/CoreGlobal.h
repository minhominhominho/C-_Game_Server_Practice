#pragma once
// Main���� ThreadManager�� new, delete �ϴ� ���� ���ŷӰ� ������
// �̱������� �ϴ� ����� ������, ���⼱ CoreGlobal���� �����ǵ��� ����

// ���� ������ �԰� ���ÿ� ThreadManager�� �ϳ� ���� ����
// extern Ű����� GThreadManager�� ��������ó�� ���
extern class ThreadManager*		GThreadManager;
extern class Memory*			GMemory;
extern class SendBufferManager*	GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class JobTimer*			GJobTimer;

extern class DeadLockProfiler*	GDeadLockProfiler;
extern class DBConnectionPool*	GDBConnectionPool;
extern class ConsoleLog*		GConsoleLogger;