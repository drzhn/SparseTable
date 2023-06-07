#ifndef SPARSE_TABLE_CHUNK_H
#define SPARSE_TABLE_CHUNK_H

#include "ScalarArray.h"
#include "SparseSet.h"
#include "Assert.h"

template <typename T, size_t Size>
class SparseTableChunk
{
public:
	SparseTableChunk()
	{
		m_setArray = std::make_unique<ScalarArray<SetItem, Size>>();

		m_data = static_cast<T*>(malloc(Size * sizeof(T))); // TODO aligned malloc

		for (int32_t i = 0; i < Size; i++)
		{
			m_setArray->operator[](i).sparse = 0;
			m_setArray->operator[](i).dense = i;
		}
		m_count = 0;
	}

	SparseTableChunk& operator=(SparseTableChunk&& other) noexcept
	{
		Clear();
		free(m_data);

		m_data = other.m_data;
		other.m_data = nullptr;

		m_setArray= std::move(other.m_setArray);

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
			m_setArray->operator[](i).sparse = 0;
			m_setArray->operator[](i).dense = i;
		}
		m_count = 0;
	}

	bool ContainsKey(int32_t key)
	{
		int32_t a = m_setArray->operator[](key).sparse;
		return a >= 0 && a < m_count && m_setArray->operator[](a).dense == key;
	}

	template <typename... Args>
	int32_t Emplace(Args&&... args)
	{
		int32_t key = m_setArray->operator[](m_count).dense;

		ASSERT(m_count < Size);

		int32_t a = m_setArray->operator[](key).sparse;
		int32_t n = m_count;
		ASSERT(a >= n || m_setArray->operator[](a).dense != key);

		m_setArray->operator[](key).sparse = n;
		m_setArray->operator[](n).dense = key;
		m_count = n + 1;

		T* objPtr = new(m_data + m_setArray->operator[](key).sparse) T(std::forward<Args>(args)...);

		return key;
	}

	void Remove(int32_t key)
	{
		ASSERT(ContainsKey(key));

		const int32_t keyIndex = m_setArray->operator[](key).sparse;
		const int32_t lastElemIndex = m_count - 1;

		ASSERT(keyIndex <= lastElemIndex && m_setArray->operator[](keyIndex).dense == key);

		int32_t lastElement = m_setArray->operator[](lastElemIndex).dense;

		m_count = lastElemIndex;
		m_setArray->operator[](keyIndex).dense = lastElement;
		m_setArray->operator[](m_count).dense = key;

		m_setArray->operator[](lastElement).sparse = keyIndex;
		m_setArray->operator[](keyIndex).sparse = m_count;

		m_data[keyIndex] = std::move(m_data[lastElemIndex]);
		m_data[lastElemIndex].~T();
	}

	T& At(int32_t key)
	{
		ASSERT(ContainsKey(key));
		int32_t elemIndex = m_setArray->operator[](key).sparse;
		return m_data[elemIndex];
	}

	T& operator[](int32_t index)
	{
		ASSERT(index >= 0 && index < m_count);
		return m_data[index];
	}

	int32_t GetSize() const
	{
		return m_count;
	}

private:
	T* m_data = nullptr;

	std::unique_ptr<ScalarArray<SetItem, Size>> m_setArray;

	int32_t m_count = 0;

	// Iterators

private:
	class SparseTableChunkIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;


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
