#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

/*---------------------
	MemoryHeader
----------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY // allocSize 이전에 SLIST_ENTRY 정보가 저장됨
{
	// [MemoryHeader][Data]
	// 일반적인 new/delete에서도 객체 앞에 있는 메모리 데이터를 이용함
	// 디버깅을 돕기 위한 용도도 있음

	MemoryHeader(int32 size) : allocSize(size) {}

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); // placement new
		return reinterpret_cast<void*>(++header);	// 실질적인 데이터의 주소 반환
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
	// TODO : 필요한 추가 정보
};

/*---------------------
	MemoryPool
----------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void			Push(MemoryHeader* ptr);
	MemoryHeader*	Pop();

private:
	SLIST_HEADER	_header;
	int32			_allocSize = 0;
	atomic<int32>	_useCount = 0; // 디버그 용도
	atomic<int32>	_reserveCount = 0;
};

