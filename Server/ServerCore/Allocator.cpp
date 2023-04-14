#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

/*---------------------
	BaseAllocator
----------------------*/

// ���⿡�� �α׸� ��ų� ��ü ���� �׽�Ʈ �� �� ����
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
	// VirtualAlloc�� 2��° ���ڴ� ����Ʈ ������ ������,
	// ���������δ� ������ ����(4KB)�� ó���ϱ� ������
	// ���������� ����� �޸� ���� �� ��������� �����ֱ� ���ؼ�
	// �Ʒ��� ���� ���
	// 4096 byte = 4KB
	// 4097 byte���� pageCount�� 2�� ��
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	// ���������� �������� ���� �޸𸮸� �Ҵ��ؼ� �����÷ο츦 ����
	// ����÷ο� ������ ���ɼ��� ������, ����÷ο�� ���� �Ͼ�� �ʱ� ������ ����
	// ex1) size�� 1000�̸�, �ּҰ��� 3096���� ��ȯ
	// ex2) size�� 8000�̸�, �ּҰ��� 8192-8000=192�� ��ȯ
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// baseAddress�� 1byte ������ �ٲ㼭 dataOffset�� ���
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
