#include "pch.h"
#include "SendBuffer.h"

/*---------------------
	   SendBuffer
----------------------*/

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

/*---------------------
	SendBufferChunk
----------------------*/

// SendBufferChunk의 함수들은 모두 TLS 영역 내부에서 실행될 것임
SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true);
	_open = false;
	_usedSize += writeSize;
}

/*---------------------
	SendBufferManager
----------------------*/

SendBufferRef SendBufferManager::Open(uint32 size)
{
	// 실질적으로 외부에서 사용됨, 할당을 요청하는 함수

	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();	// WRITE_LOCK
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	// 다 썼으면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop();	// WRITE_LOCK
		LSendBufferChunk->Reset();
	}

	// cout << "Free : " << LSendBufferChunk->FreeSize() << endl;

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	//cout << "Pop SENDBUFFERCHUNK" << endl;

	{
		// 여유분이 있으면, 가져다 씀
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	// 여유분이 없다면, 새로 만들어 줌
		// MakeShared는 소멸하면 해당 소멸자를 호출해주는 방식으로 구현했음
		// shared_ptr을 이용해 deleter를 지정해서 소멸할 때의 동작을 PushGlobal로 넘겨줌
	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	cout << "PushGlobal SENDBUFFERCHUNK" << endl;

	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}