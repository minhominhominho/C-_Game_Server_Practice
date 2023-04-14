#include "pch.h"
#include "MemoryPool.h"

/*---------------------
	MemoryPool
----------------------*/

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	// '=' 이 하나만 있음 -> 우항을 실행해서 좌항에 넣고 좌항을 체크하는 거임
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
		::_aligned_free(memory);
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	ptr->allocSize = 0;

	// Pool에 메모리 반납
	::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(ptr));

	_useCount.fetch_sub(1);
	_reserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));

	// 없으면 새로 만든다 -> 동적으로 할당
	// 아예 시작 단계에서 할당하는 방법도 있음
	if (memory == nullptr)
	{
		// 16byte로 맞추기 위해 _aligned_malloc 서용
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
		// Stomp Allocator를 사용하지 않는 이유:
		// Stomp Allocator는 UAF를 방지하기 위함이었는데,
		// 메모리 풀링은 재사용을 위함이기 때문에
		// 같이 사용할 이유가 없음
	}
	else // 혹시나 하는 버그 체크
	{
		// Push에서 queue에 반납할때는 allocSize 0으로 만들어서 반납
		ASSERT_CRASH(memory->allocSize == 0);
		_reserveCount.fetch_sub(1);
	}

	_useCount.fetch_add(1);

	return memory;
}