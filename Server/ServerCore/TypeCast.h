#pragma once
#include "Types.h"

#pragma region TypeList
// ���ø� ���� �� �Ʒ����� ���ø� Ư��ȭ
template<typename... T>
struct TypeList;

// �����Ͱ� 2���ۿ� ���� ���
template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

// �����Ͱ� 2�� �ʰ��� ���
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

// ����ִ� ���
template<>
struct Length<TypeList<>>
{
	enum { value = 0 };
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	// ��������� ȣ�� -> 1 + (1 + (1 + ... + 0))
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
	// ��������� ȣ�� -> ���� 0�� �ְ� ���� �� ���� ã��
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

// ����ִ� ���
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

	// ������ ��� �� -> �Ǻ� ��ü�� ���� ��!
	static Small Test(const To&) { return 0; } // �켱������ �Ʒ����� ����
	static Big Test(...) { return 0; }
	static From MakeFrom() { return 0; }

public:
	enum
	{
		// From�� To�� ��ȯ �����ϸ� exists -> true
		//					�������� �ʴٸ� Big Test(...) ȣ��, exists -> false
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
	};
};

#pragma endregion

#pragma region TypeCast

// ���ø� �μ��� ������ Ÿ�ӿ� �����Ǿ� �־�� ��!
// -> i, j�� �״�� ������� �ʰ� Int2Type �̿�!
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
		// �ڵ����� ��ü�� �ѹ��� ����� ����.
		// ��������� ���� table�� �˾Ƽ� ä����
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[length][length];
};

// static ��� ���� �ʱ�ȭ
template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

// ��������� ����ϱ� ���� �Լ�
// ������ dynamic_cast
template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	// remove_pointer_t�� �����͸� ������ ������ ��ȯ�Ѵٰ� ����
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

// shared_ptr ����
template<typename To, typename From>
shared_ptr<To> TypeCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	// remove_pointer_t�� �����͸� ������ ������ ��ȯ�Ѵٰ� ����
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

// shared_ptr ����
template<typename To, typename From>
bool CanCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}

#pragma endregion

// ������ ���Ǽ� ��ũ��
#define DECLARE_TL		using TL = TL; int32 _typeId;
#define INIT_TL(Type)	_typeId = IndexOf<TL,Type>::value;