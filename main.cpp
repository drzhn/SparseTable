#include "Assert.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>


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
		delete(m_data);
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
		ASSERT(m_count == 0); // we need guarantees that we delete table after removing all elements from it
		if (m_data != nullptr)
		{
			delete(m_data);
		}
	}

	bool ContainsKey(int32_t key)
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
		ASSERT(ContainsKey(key))

		const int32_t keyIndex = m_sparse[key];
		const int32_t lastElemIndex = m_count - 1;

		ASSERT(keyIndex <= lastElemIndex && m_dense[keyIndex] == key)

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

	T& operator[](int32_t index)
	{
		ASSERT(index < m_count);
		return m_data[index];
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
};

template <typename T, size_t MaxTablesCount, size_t TableSize>
class SparseTable
{
	typedef SparseTableChunk<T, TableSize> TableT;

public:
	template <typename... Args>
	int32_t Emplace(Args&&... args)
	{
		int32_t tableIndex = -1;

		if (m_nonFullTables.GetSize() > 0)
		{
			tableIndex = m_nonFullTables[0];
		}
		else if (m_tables.GetSize() < MaxTablesCount)
		{
			tableIndex = m_tables.Emplace();
		}

		ASSERT_DESC(tableIndex != -1, "SparseTable is full, increase MaxTablesCount")

		const int32_t itemIndex = m_tables.At(tableIndex).Emplace(std::forward<Args>(args)...);

		if (m_tables.At(tableIndex).GetSize() < TableSize && !m_nonFullTables.ContainsKey(tableIndex))
		{
			m_nonFullTables.Insert(tableIndex);
		}

		if (m_tables.At(tableIndex).GetSize() == TableSize)
		{
			ASSERT(m_nonFullTables.ContainsKey(tableIndex));
			m_nonFullTables.Remove(tableIndex);
		}


		return tableIndex * TableSize + itemIndex;
	}

	void Remove(int32_t index)
	{
		const int32_t tableIndex = index / TableSize;
		const int32_t itemIndex = index % TableSize;

		ASSERT(m_tables.ContainsKey(tableIndex));
		auto& table = m_tables.At(tableIndex);
		ASSERT(table.ContainsKey(itemIndex));

		table.Remove(itemIndex);

		if (table.GetSize() == 0)
		{
			m_tables.Remove(tableIndex);
			m_nonFullTables.Remove(tableIndex);
		}
		else if (table.GetSize() < TableSize && !m_nonFullTables.ContainsKey(tableIndex))
		{
			m_nonFullTables.Insert(tableIndex);
		}
	}

private:
	SparseTableChunk<TableT, MaxTablesCount> m_tables;
	SparseSet<MaxTablesCount> m_nonFullTables;

	int32_t m_chunksCount = 0;
};


class TestClass
{
public:
	TestClass() = delete;

	TestClass(int number) :
		m_number(number)
	{
	}

	TestClass(TestClass&& other) noexcept
	{
		m_number = other.m_number;
		other.m_number = -1;
	}

	TestClass& operator=(TestClass&& other) noexcept
	{
		m_number = other.m_number;
		other.m_number = -1;

		return *this;
	}

private:
	int m_number = -1;
};


int main()
{
	SparseTable<TestClass, 2, 4> set;
	auto a = set.Emplace(0);
	auto b = set.Emplace(1);
	auto c = set.Emplace(2);
	auto d = set.Emplace(3);

	auto e = set.Emplace(4);
	auto f = set.Emplace(5);
	auto g = set.Emplace(6);
	auto h = set.Emplace(7);

	set.Remove(b);
	set.Remove(g);

	b = set.Emplace(1);
	g = set.Emplace(6);
	return 0;
}
