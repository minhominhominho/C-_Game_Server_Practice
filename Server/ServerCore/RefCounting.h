#pragma once

/*---------------------
	  RefCountable
----------------------*/

// RefCountable만 사용하면, _refCount를 수동으로 관리해 주어야함
class RefCountable
{
public:
	RefCountable() : _refCount(1) {}
	virtual ~RefCountable() {}
	// virtual로 선언해야 예상치 못한 메모리 누수가 없음!
	// 이래야 부모 소멸자와 자식 소멸자 모두 호출되니까

	int32 GetRefCount() { return _refCount; }

	int32 AddRef() { return ++_refCount; }
	int32 ReleaseRef()
	{
		int32 refCount = --_refCount;
		// RefCountable만 단독으로 사용하면, 여기에서 다른 쓰레드의 개입 가능 -> 위험함
		if (refCount == 0)
		{
			delete this;
		}
		return refCount;
	}

protected:
	atomic<int32> _refCount;
	
	// int32 _refCount;
	// 그냥 int32만 하면 원자적이지 않기 때문에 멀티쓰레드 환경에서는 위험함
};


/*---------------------
	   SharedPtr
----------------------*/

// RefCountable을 래핑하는 클래스
// AddRef()와 ReleaseRef()를 대신 해줌
template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() {}
	TSharedPtr(T* ptr) { Set(ptr); }

	// 복사
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
	// 이동
	TSharedPtr(TSharedPtr&& rhs) { _ptr = rhs._ptr; rhs._ptr = nullptr; }
	// 상속 관계 복사
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

	~TSharedPtr() { Release(); }

public:
	// 복사 연산자
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}
		return *this;
	}

	// 이동 연산자
	TSharedPtr& operator=(TSharedPtr&& rhs)
	{
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
		return *this;
	}

	bool		operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
	bool		operator==(T* ptr) const { return _ptr == ptr; }
	bool		operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
	bool		operator!=(T* ptr) const { return _ptr != ptr; }
	bool		operator<(const TSharedPtr& rhs) const { return _ptr < rhs._ptr; }
	T*			operator*() { return _ptr; }
	const T*	operator*() const { return _ptr; }
				operator T* () const { return _ptr; }
	T*			operator->() { return _ptr; }
	const T*	operator->() const { return _ptr; }

	bool IsNull() { return _ptr == nullptr; }

private:
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if (ptr)
			ptr->AddRef();
	}

	inline void Release()
	{
		if (_ptr != nullptr)
		{
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}

private:
	T* _ptr = nullptr;

};