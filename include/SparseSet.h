#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include "ScalarArray.h"

#include "SptableAssert.h"

struct SetItem
{
	int32_t sparse;
	int32_t dense;
};


template <size_t Size>
class SparseSet
{
public:
	SparseSet()
	{
		m_setArray = std::make_unique<ScalarArray<SetItem, Size>>();

		for (int32_t i = 0; i < Size; i++)
		{
			m_setArray->operator[](i).sparse = 0;
			m_setArray->operator[](i).dense = 0;
		}
	}

	void Insert(int32_t key)
	{
		SPTABLE_ASSERT(!ContainsKey(key));

		int32_t a = m_setArray->operator[](key).sparse;
		int32_t n = m_count;
		SPTABLE_ASSERT(a >= n || m_setArray->operator[](a).dense != key);

		m_setArray->operator[](key).sparse = n;
		m_setArray->operator[](n).dense = key;
		m_count++;
	}

	void Remove(int32_t key)
	{
		SPTABLE_ASSERT(ContainsKey(key));
		int32_t a = m_setArray->operator[](key).sparse;
		int32_t n = m_count - 1;

		SPTABLE_ASSERT(a <= n && m_setArray->operator[](a).dense == key)
		int32_t e = m_setArray->operator[](n).dense;

		m_count = n;
		m_setArray->operator[](a).dense = e;
		m_setArray->operator[](e).sparse = a;
	}

	void Clear()
	{
		for (int32_t i = 0; i < Size; i++)
		{
			m_setArray->operator[](i).sparse = 0;
			m_setArray->operator[](i).dense = 0;
		}
		m_count = 0;
	}

	bool ContainsKey(int32_t k)
	{
		int32_t a = m_setArray->operator[](k).sparse;
		return a < m_count && m_setArray->operator[](a).dense == k;
	}

	int32_t operator[](int32_t index)
	{
		SPTABLE_ASSERT(index < m_count);
		return m_setArray->operator[](index).dense;
	}

	int32_t GetSize() const
	{
		return m_count;
	}

private:
	std::unique_ptr<ScalarArray<SetItem, Size>> m_setArray;
	int32_t m_count = 0;
};
#endif // SPARSE_SET_H
