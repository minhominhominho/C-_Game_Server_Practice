#pragma once

/*---------------------
	   RecvBuffer
----------------------*/

// read 커서와 write 커서가 같은 위치에서 시작,
// write 커서가 먼저 지나가고, read 커서가 따라가는 구조
// write 커서가 끝에 도달하면(공간이 차면), 여러 방법으로 해결해주는데
// 1) 순환 버퍼
// 2) read 커서와 write 커서가 일치할때, 두 커서를 모두 0으로 옮겨주는 방법 (복사 비용 없음)
	// write 커서가 끝에 도달했을 때 까지 일치하지 않으면, 
	// 두 커서의 상대적 위치를 유지한 채 read 커서를 0으로 옮겨줌 (이때는 약간의 복사 비용)
// 3) 기타 등등
// 우린 2번째 방법 활용

class RecvBuffer
{
	// 요청한 버퍼보다 10배 큰 버퍼사이즈를 만들어서,
	// 두 커서가 일치할 확률을 높이기 위함
	enum { BUFFER_COUNT = 10 };

public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void			Clean();
	bool			OnRead(int32 numOfBytes);
	bool			OnWrite(int32 numOfBytes);

	BYTE*			ReadPos() { return &_buffer[_readPos]; }
	BYTE*			WritePos() { return &_buffer[_writePos]; }
	int32			DataSize() { return _writePos - _readPos; }
	int32			FreeSize() { return _capacity - _writePos; }

private:
	int32			_capacity = 0;
	int32			_bufferSize = 0;
	int32			_readPos = 0;
	int32			_writePos = 0;
	Vector<BYTE>	_buffer;
};

