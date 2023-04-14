#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

/*---------------------
	BaseAllocator
----------------------*/

// 여기에서 로그를 찍거나 객체 수를 테스트 할 수 있음
void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}

/*---------------------
	StompAllocator
----------------------*/

void* StompAllocator::Alloc(int32 size)
{
	// ex) 4 -> 4096
	// VirtualAlloc의 2번째 인자는 바이트 단위를 넣지만,
	// 내부적으로는 페이지 단위(4KB)로 처리하기 때문에
	// 실질적으로 사용할 메모리 양을 더 사실적으로 보여주기 위해서
	// 아래와 같이 계산
	// 4096 byte = 4KB
	// 4097 byte부터 pageCount가 2가 됨
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	// 인위적으로 페이지의 끝에 메모리를 할당해서 오버플로우를 방지
	// 언더플로우 문제의 가능성은 있지만, 언더플로우는 거의 일어나지 않기 때문에 괜찮
	// ex1) size가 1000이면, 주소값을 3096으로 반환
	// ex2) size가 8000이면, 주소값을 8192-8000=192로 반환
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// baseAddress를 1byte 단위로 바꿔서 dataOffset와 계산
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

/*---------------------
	PoolAllocator
----------------------*/

void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
