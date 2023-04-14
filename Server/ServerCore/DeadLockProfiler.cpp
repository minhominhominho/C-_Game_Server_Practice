#include "pch.h"
#include "DeadLockProfiler.h"

/*---------------------
	DeadLockProfiler
----------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock); // 멀티쓰레드 환경에서 동작해야 하기 때문에

	// 아이디를 찾거나 발급한다.
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}

	// 이미 잡고 있는 락이 있었다면
	if (LLockStack.empty() == false)
	{
		// 기존에 발견되지 않은 케이스만 데드락 여부를 확인

		const int32 prevId = LLockStack.top();
		if (lockId != prevId)
		{
			// 바로 이전에 stack에 들어온 lock과 지금 들어온 lock이 같으면,
			// (재귀적으로 lock을 잡은 상황)
			// DeadLock을 이미 확인했기 때문에 다시 확인할 필요 없음
			// 그렇지 않을 경우(lock을 잡은 채로 다른 lock을 잡은 경우)에만 데드락 여부 확인

			set<int32>& history = _lockHistory[prevId];
			// map 문법 : map[idx] 하면, idx를 key로 하는 element가 map에 추가됨!
			// 따라서 이 때, _lockHistory[prevId]가 생성되거나 이미 있는 값을 불러오거나 하게 됨

			// prevId에 해당하는 lock을 잡고, lockId에 해당하는 lock을 잡는 상태
			// → 이것을 directed edge로 history에 기록해야함
			
			// 그러나 새로운 간선을 발견한 경우, 즉 lockId가 처음 발견된 경우에만 데드락 여부를 확인하면 됨
			// (prevId를 잡고, lockId를 처음으로 잡아본 경우에만 데드락 여부 확인)
			if (history.find(lockId) == history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}

		}
	}

	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	// 버그 예방 차원
	if (LLockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	// 버그 예방 차원
	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	// cycle을 탐지할 때 필요한 임시 변수들 모두 초기화
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0;lockId < lockCount;lockId++)
		Dfs(lockId);

	// 연산이 끝났으면 정리한다.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

// 모든 중복되는 lock들에 대해서  cycle 만드는 상황을 피하기 위함!
// → 모든 발생 가능한 edge(lock의 중복)에 대해서 검사함
// 몇몇 lock이 풀렸다고 history에서 지우거나 하지 않음
void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;
	
	// 모든 인접한 정점을 순회한다.
	// _lockHistory[here]로 하지 않음 -> 쓸데없이 map element 추가될 수도 있으니까
	auto findIt = _lockHistory.find(here); 
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;	// 미리 size만큼 false 채워져 있었음
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// 아직 방문한 적이 없다면 방문한다.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here가 there보다 먼저 발견되었다면, there는 here의 후손이다. (순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// 순방향이 아니고, 
		// Dfs(there)가 아직 종료하지 않았다면(=같은 Dfs 라면),
		// there는 here의 선조이다. (역방향 간선)
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);
			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
