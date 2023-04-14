#pragma once
#include "Types.h"
#include "MemoryPool.h"

template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static Type* Pop(Args&&... args)
	{
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else 
		// s_pool에서 Pop을 하고, 그 MemoryHeader에 s_allocSize의 MemoryHeader를 설정하고 실제 공간 받음
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif
		new(memory)Type(forward<Args>(args)...); // placement new, 생성자 호출
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type();
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.Push(MemoryHeader::DetachHeader(obj));
#endif
	}

	template<typename... Args>
	static shared_ptr<Type> MakeShared(Args&&... args)
	{
		shared_ptr<Type> ptr = {Pop(forward<Args>(args)...), Push};
		return ptr;
	}

private:
	// template과 static을 같이 사용하면, Type 별로 1개씩 존재
	static int32		s_allocSize;
	static MemoryPool	s_pool;
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{s_allocSize};