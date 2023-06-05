#ifndef SPARSE_SET_H
#define SPARSE_SET_H


#include "Assert.h"

template <size_t Size>
class SparseSet
{
public:
	SparseSet()
	{
		for (int32_t i = 0; i < Size; i++)
		{
			m_sparse[i] = 0;
			m_dense[i] = 0;
		}
	}

	void Insert(int32_t key)
	{
		ASSERT(!ContainsKey(key));

		int32_t a = m_sparse[key];
		int32_t n = m_count;
		ASSERT(a >= n || m_dense[a] != key);

		m_sparse[key] = n;
		m_dense[n] = key;
		m_count++;
	}

	void Remove(int32_t key)
	{
		ASSERT(ContainsKey(key));
		int32_t a = m_sparse[key];
		int32_t n = m_count - 1;

		ASSERT(a <= n && m_dense[a] == key)
			int32_t e = m_dense[n];

		m_count = n;
		m_dense[a] = e;
		m_sparse[e] = a;
	}

	bool ContainsKey(int32_t k)
	{
		int32_t a = m_sparse[k];
		return a < m_count && m_dense[a] == k;
	}

	int32_t operator[](int32_t index)
	{
		ASSERT(index < m_count);
		return m_dense[index];
	}

	int32_t GetSize() const
	{
		return m_count;
	}

private:
	int32_t m_sparse[Size]{};
	int32_t m_dense[Size]{};
	int32_t m_count = 0;
};
#endif // SPARSE_SET_H
