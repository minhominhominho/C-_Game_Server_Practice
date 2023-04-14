#pragma once
#include "Types.h"

#pragma region TypeList
// 템플릿 선언 후 아래에서 템플릿 특수화
template<typename... T>
struct TypeList;

// 데이터가 2개밖에 없는 경우
template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

// 데이터가 2개 초과인 경우
template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};
#pragma endregion

#pragma region Length
template<typename T>
struct Length;

// 비어있는 경우
template<>
struct Length<TypeList<>>
{
	enum { value = 0 };
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	// 재귀적으로 호출 -> 1 + (1 + (1 + ... + 0))
	enum { value = 1 + Length<TypeList<U...>>::value };
};
#pragma endregion

#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;

template<typename Head, typename... Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	// 재귀적으로 호출 -> 위의 0인 애가 나올 때 까지 찾음
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};
#pragma endregion

#pragma region IndexOf
template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum { value = 0 };
};

// 비어있는 경우
template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value };
public:
	enum { value = (temp == -1 ? -1 : temp + 1) };
};
#pragma endregion

#pragma region Conversion
template<typename From, typename To>
struct Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	// 구현부 없어도 됨 -> 판별 자체를 위한 것!
	static Small Test(const To&) { return 0; } // 우선순위가 아래보다 높음
	static Big Test(...) { return 0; }
	static From MakeFrom() { return 0; }

public:
	enum
	{
		// From이 To로 변환 가능하면 exists -> true
		//					가능하지 않다면 Big Test(...) 호출, exists -> false
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
	};
};

#pragma endregion

#pragma region TypeCast

// 템플릿 인수는 컴파일 타임에 결정되어 있어야 함!
// -> i, j를 그대로 사용하지 않고 Int2Type 이용!
template<int32 v>
struct Int2Type
{
	enum { value = v };
};

template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeTable(Int2Type<0>(), Int2Type<0>());

		/*for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < length; j++)
			{
				using FromType = typename TypeAt<TL, i>::Result;
				using ToType = typename TypeAt<TL, j>::Result;

				if (Conversion<const FromType*, const ToType*>::exists)
				{
					s_convert[i][j] = true;
				}
				else
				{
					s_convert[i][j] = false;
				}
			}
		}*/
	}

	template<int32 i, int32 j>
	static void MakeTable(Int2Type<i>, Int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		if (Conversion<const FromType*, const ToType*>::exists)
		{
			s_convert[i][j] = true;
		}
		else
		{
			s_convert[i][j] = false;
		}

		MakeTable(Int2Type<i>(), Int2Type<j + 1>());
	}

	template<int32 i>
	static void MakeTable(Int2Type<i>, Int2Type<length>)
	{
		MakeTable(Int2Type<i + 1>(), Int2Type<0>());
	}	

	template<int32 j>
	static void MakeTable(Int2Type<length>, Int2Type<j>)
	{
	}

	static inline bool CanConvert(int32 from, int32 to)
	{
		// 자동으로 객체를 한번만 만들기 위함.
		// 만들어지는 순간 table이 알아서 채워짐
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[length][length];
};

// static 멤버 변수 초기화
template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

// 결과적으로 사용하기 위한 함수
// 일종의 dynamic_cast
template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	// remove_pointer_t는 포인터를 제거한 버전을 반환한다고 생각
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

// shared_ptr 버전
template<typename To, typename From>
shared_ptr<To> TypeCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	// remove_pointer_t는 포인터를 제거한 버전을 반환한다고 생각
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}

// shared_ptr 버전
template<typename To, typename From>
bool CanCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}

#pragma endregion

// 일종의 편의성 매크로
#define DECLARE_TL		using TL = TL; int32 _typeId;
#define INIT_TL(Type)	_typeId = IndexOf<TL,Type>::value;