#pragma once
#include <stack>
#include <map>
#include <vector>

/*---------------------
	DeadLockProfiler
----------------------*/

class DeadLockProfiler
{
public:
	// Lock을 받아옴
	// name은 Lock의 종류
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	// 받아올 때는 편의성을 위해 이름으로 받아오지만, 연산할 때는 정수로 연산
	unordered_map<const char*, int32>	_nameToId;
	unordered_map<int32, const char*>	_idToName;
	// 어떤 Lock이 다른 어떤 Lock을 잡았는지를 기록, 간선에 해당되는 정보 
	map<int32, set<int32>>				_lockHistory;
	

	Mutex _lock;

private:
	vector<int32>	_discoveredOrder;		// 정점이 발견된 순서를 기록하는 배열
	int32			_discoveredCount = 0;	// 정점이 발견된 순서
	vector<bool>	_finished;				// Dfs(i)가 종료되었는지에 대한 여부
	vector<int32>	_parent;				// 발견된 부모가 누구인지에 대한 정보
};

