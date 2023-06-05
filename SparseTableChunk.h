#ifndef SPARSE_TABLE_CHUNK_H
#define SPARSE_TABLE_CHUNK_H

#include "Assert.h"

#include <array>

template <typename T, size_t Size>
class SparseTableChunk
{
public:
	SparseTableChunk()
	{
		m_data = static_cast<T*>(malloc(Size * sizeof(T))); // TODO aligned malloc

		for (int32_t i = 0; i < Size; i++)
		{
			m_sparse[i] = 0;
			m_dense[i] = i;
		}
		m_count = 0;
	}

	SparseTableChunk& operator=(SparseTableChunk&& other) noexcept
	{
		Clear();
		free(m_data);

		m_data = other.m_data;
		other.m_data = nullptr;

		m_sparse = other.m_sparse; // TODO do smth with array copying 
		m_dense = other.m_dense;

		m_count = other.m_count;
		other.m_count = 0;

		return *this;
	}

	~SparseTableChunk()
	{
		ASSERT_DESC(m_count == 0, "Please, call Clear() before destructor"); // we need guarantees that we delete table after removing all elements from it
		if (m_data != nullptr)
		{
			free(m_data);
			m_data = nullptr;
		}
	}

	void Clear()
	{
		for (int32_t i = 0; i < Size; i++)
		{
			if (i < m_count)
			{
				m_data[i].~T();
			}
			m_sparse[i] = 0;
			m_dense[i] = i;
		}
		m_count = 0;
	}

	bool ContainsKey(int32_t key) const
	{
		int32_t a = m_sparse[key];
		return a >= 0 && a < m_count && m_dense[a] == key;
	}

	template <typename... Args>
	int32_t Emplace(Args&&... args)
	{
		int32_t key = m_dense[m_count];

		ASSERT(m_count < Size);

		int32_t a = m_sparse[key];
		int32_t n = m_count;
		ASSERT(a >= n || m_dense[a] != key);

		m_sparse[key] = n;
		m_dense[n] = key;
		m_count = n + 1;

		T* objPtr = new(m_data + m_sparse[key]) T(std::forward<Args>(args)...);

		return key;
	}

	void Remove(int32_t key)
	{
		ASSERT(ContainsKey(key));

		const int32_t keyIndex = m_sparse[key];
		const int32_t lastElemIndex = m_count - 1;

		ASSERT(keyIndex <= lastElemIndex && m_dense[keyIndex] == key);

		int32_t lastElement = m_dense[lastElemIndex];

		m_count = lastElemIndex;
		m_dense[keyIndex] = lastElement;
		m_dense[m_count] = key;

		m_sparse[lastElement] = keyIndex;
		m_sparse[keyIndex] = m_count;

		m_data[keyIndex] = std::move(m_data[lastElemIndex]);
		m_data[lastElemIndex].~T();
	}

	T& At(int32_t key)
	{
		ASSERT(ContainsKey(key));
		int32_t elemIndex = m_sparse[key];
		return m_data[elemIndex];
	}

	int32_t GetSize() const
	{
		return m_count;
	}

private:
	T* m_data = nullptr;

	std::array<int32_t, Size> m_sparse;
	std::array<int32_t, Size> m_dense;

	int32_t m_count = 0;

	// Iterators

private:
	class SparseTableChunkIterator : public std::iterator<std::bidirectional_iterator_tag, T>
	{
	public:
		SparseTableChunkIterator(T* iter) : current(iter)
		{
		}

		T& operator*() const
		{
			return *current;
		}

		SparseTableChunkIterator& operator++()
		{
			++current;
			return *this;
		}

		SparseTableChunkIterator operator++(int)
		{
			SparseTableChunkIterator tmp = *this;
			++(*this);
			return tmp;
		}

		SparseTableChunkIterator& operator--()
		{
			--current;
			return *this;
		}

		SparseTableChunkIterator operator--(int)
		{
			SparseTableChunkIterator tmp = *this;
			--(*this);
			return tmp;
		}

		bool operator==(const SparseTableChunkIterator& other) const
		{
			return current == other.current;
		}

		bool operator!=(const SparseTableChunkIterator& other) const
		{
			return !(*this == other);
		}

	private:
		T* current;
	};

public:
	SparseTableChunkIterator begin()
	{
		return SparseTableChunkIterator(m_data);
	}

	SparseTableChunkIterator end()
	{
		return SparseTableChunkIterator(m_data + m_count);
	}
};
#endif // SPARSE_TABLE_CHUNK_H
