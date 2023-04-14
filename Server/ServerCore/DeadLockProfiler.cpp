#include "pch.h"
#include "DeadLockProfiler.h"

/*---------------------
	DeadLockProfiler
----------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock); // ��Ƽ������ ȯ�濡�� �����ؾ� �ϱ� ������

	// ���̵� ã�ų� �߱��Ѵ�.
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

	// �̹� ��� �ִ� ���� �־��ٸ�
	if (LLockStack.empty() == false)
	{
		// ������ �߰ߵ��� ���� ���̽��� ����� ���θ� Ȯ��

		const int32 prevId = LLockStack.top();
		if (lockId != prevId)
		{
			// �ٷ� ������ stack�� ���� lock�� ���� ���� lock�� ������,
			// (��������� lock�� ���� ��Ȳ)
			// DeadLock�� �̹� Ȯ���߱� ������ �ٽ� Ȯ���� �ʿ� ����
			// �׷��� ���� ���(lock�� ���� ä�� �ٸ� lock�� ���� ���)���� ����� ���� Ȯ��

			set<int32>& history = _lockHistory[prevId];
			// map ���� : map[idx] �ϸ�, idx�� key�� �ϴ� element�� map�� �߰���!
			// ���� �� ��, _lockHistory[prevId]�� �����ǰų� �̹� �ִ� ���� �ҷ����ų� �ϰ� ��

			// prevId�� �ش��ϴ� lock�� ���, lockId�� �ش��ϴ� lock�� ��� ����
			// �� �̰��� directed edge�� history�� ����ؾ���
			
			// �׷��� ���ο� ������ �߰��� ���, �� lockId�� ó�� �߰ߵ� ��쿡�� ����� ���θ� Ȯ���ϸ� ��
			// (prevId�� ���, lockId�� ó������ ��ƺ� ��쿡�� ����� ���� Ȯ��)
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

	// ���� ���� ����
	if (LLockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	// ���� ���� ����
	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	// cycle�� Ž���� �� �ʿ��� �ӽ� ������ ��� �ʱ�ȭ
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0;lockId < lockCount;lockId++)
		Dfs(lockId);

	// ������ �������� �����Ѵ�.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

// ��� �ߺ��Ǵ� lock�鿡 ���ؼ�  cycle ����� ��Ȳ�� ���ϱ� ����!
// �� ��� �߻� ������ edge(lock�� �ߺ�)�� ���ؼ� �˻���
// ��� lock�� Ǯ�ȴٰ� history���� ����ų� ���� ����
void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;
	
	// ��� ������ ������ ��ȸ�Ѵ�.
	// _lockHistory[here]�� ���� ���� -> �������� map element �߰��� ���� �����ϱ�
	auto findIt = _lockHistory.find(here); 
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;	// �̸� size��ŭ false ä���� �־���
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// ���� �湮�� ���� ���ٸ� �湮�Ѵ�.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here�� there���� ���� �߰ߵǾ��ٸ�, there�� here�� �ļ��̴�. (������ ����)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// �������� �ƴϰ�, 
		// Dfs(there)�� ���� �������� �ʾҴٸ�(=���� Dfs ���),
		// there�� here�� �����̴�. (������ ����)
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
