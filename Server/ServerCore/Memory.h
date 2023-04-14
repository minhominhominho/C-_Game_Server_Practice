#pragma once
#include "Allocator.h"

class MemoryPool; // 전방선언

/*---------------------
		Memory
----------------------*/

// 일종의 메모리 매니저
class Memory
{
	// 큰 데이터는 굳이 풀링을 할 필요 없음
	enum
	{
		// 일단 풀의 총 개수는 48개
		// 32, 64, 96, 128, ... 1024 크기의 풀이 각 1개씩 -> 총 32개
		// 1024+128, 1024+128*2, 1024+128*3, ... 2048 크기의 풀이 각 1개씩 -> 총 8개
		// 2048+128, 2048+128*2, 2048+128*3, ... 4096 크기의 풀이 각 1개씩 -> 총 8개
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위가 이런 뜻
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		// 최대 크기의 풀이 4096 크기의 풀이라는 뜻!
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void	Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	// 메모리 크기 <-> 메모리 풀
	// O(1) 시간 안에 빠르게 찾기 위한 헬퍼 테이블
	// 0~32까지는 32짜리 pool테이블을 가르키고,
	// 33~64까지는 64짜리 pool테이블을 가르키는 방식으로 사용될 예정
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// placement new : 이미 할당된 메모리에 생성자를 호출하는 문법
	// new(memory)Type();		// 이렇게만 하면 인자가 있는 상황에 대한 대처가 안됨
	// new(memory)Type(100);	// template으로 동작하기 때문에 이렇게 하면 안됨

	// 과거에는 생성자의 개수대로 xnew를 오버로딩 했음
	// 그러나 Modern C++에서는 typename... Args를 사용하면 됨
	// typename... Args : 가변 길이 템플릿
	new(memory)Type(forward<Args>(args)...);
	// 이렇게 하면, 인자에 맞게 생성자 호출
	// std::forward는 전달 참조


	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();	// 포인터로 생성자를 호출하는 문법
	PoolAllocator::Release(obj);
}

template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ xnew<Type>(forward<Args>(args)...), xdelete<Type> };
}