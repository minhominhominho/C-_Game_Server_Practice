#pragma once
#include "Allocator.h"

class MemoryPool; // ���漱��

/*---------------------
		Memory
----------------------*/

// ������ �޸� �Ŵ���
class Memory
{
	// ū �����ʹ� ���� Ǯ���� �� �ʿ� ����
	enum
	{
		// �ϴ� Ǯ�� �� ������ 48��
		// 32, 64, 96, 128, ... 1024 ũ���� Ǯ�� �� 1���� -> �� 32��
		// 1024+128, 1024+128*2, 1024+128*3, ... 2048 ũ���� Ǯ�� �� 1���� -> �� 8��
		// 2048+128, 2048+128*2, 2048+128*3, ... 4096 ũ���� Ǯ�� �� 1���� -> �� 8��
		// ~1024���� 32����, ~2048���� 128����, ~4096���� 256������ �̷� ��
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		// �ִ� ũ���� Ǯ�� 4096 ũ���� Ǯ�̶�� ��!
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void	Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	// �޸� ũ�� <-> �޸� Ǯ
	// O(1) �ð� �ȿ� ������ ã�� ���� ���� ���̺�
	// 0~32������ 32¥�� pool���̺��� ����Ű��,
	// 33~64������ 64¥�� pool���̺��� ����Ű�� ������� ���� ����
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// placement new : �̹� �Ҵ�� �޸𸮿� �����ڸ� ȣ���ϴ� ����
	// new(memory)Type();		// �̷��Ը� �ϸ� ���ڰ� �ִ� ��Ȳ�� ���� ��ó�� �ȵ�
	// new(memory)Type(100);	// template���� �����ϱ� ������ �̷��� �ϸ� �ȵ�

	// ���ſ��� �������� ������� xnew�� �����ε� ����
	// �׷��� Modern C++������ typename... Args�� ����ϸ� ��
	// typename... Args : ���� ���� ���ø�
	new(memory)Type(forward<Args>(args)...);
	// �̷��� �ϸ�, ���ڿ� �°� ������ ȣ��
	// std::forward�� ���� ����


	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();	// �����ͷ� �����ڸ� ȣ���ϴ� ����
	PoolAllocator::Release(obj);
}

template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ xnew<Type>(forward<Args>(args)...), xdelete<Type> };
}