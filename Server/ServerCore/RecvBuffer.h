#pragma once

/*---------------------
	   RecvBuffer
----------------------*/

// read Ŀ���� write Ŀ���� ���� ��ġ���� ����,
// write Ŀ���� ���� ��������, read Ŀ���� ���󰡴� ����
// write Ŀ���� ���� �����ϸ�(������ ����), ���� ������� �ذ����ִµ�
// 1) ��ȯ ����
// 2) read Ŀ���� write Ŀ���� ��ġ�Ҷ�, �� Ŀ���� ��� 0���� �Ű��ִ� ��� (���� ��� ����)
	// write Ŀ���� ���� �������� �� ���� ��ġ���� ������, 
	// �� Ŀ���� ����� ��ġ�� ������ ä read Ŀ���� 0���� �Ű��� (�̶��� �ణ�� ���� ���)
// 3) ��Ÿ ���
// �츰 2��° ��� Ȱ��

class RecvBuffer
{
	// ��û�� ���ۺ��� 10�� ū ���ۻ���� ����,
	// �� Ŀ���� ��ġ�� Ȯ���� ���̱� ����
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

