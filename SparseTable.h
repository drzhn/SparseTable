#ifndef SPARSE_TABLE_H
#define SPARSE_TABLE_H

#include "SparseSet.h"
#include "SparseTableChunk.h"
#include "Assert.h"

#include <cstdint>
#include <memory>

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

		ASSERT_DESC(tableIndex != -1, "SparseTable is full, increase MaxTablesCount");

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

		m_count++;

		return tableIndex * TableSize + itemIndex;
	}

	void Remove(int32_t key)
	{
		const int32_t tableIndex = key / TableSize;
		const int32_t itemIndex = key % TableSize;

		ASSERT(m_tables.ContainsKey(tableIndex));
		auto& table = m_tables.At(tableIndex);
		ASSERT(table.ContainsKey(itemIndex));

		table.Remove(itemIndex);
		m_count--;

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

	T& At(int32_t key)
	{
		ASSERT(ContainsKey(key));
		const int32_t tableIndex = key / TableSize;
		const int32_t itemIndex = key % TableSize;
		return m_tables.At(tableIndex).At(itemIndex);
	}

	bool ContainsKey(int32_t key)
	{
		const int32_t tableIndex = key / TableSize;
		const int32_t itemIndex = key % TableSize;

		if (m_tables.ContainsKey(tableIndex))
		{
			auto& table = m_tables.At(tableIndex);
			if (table.ContainsKey(itemIndex))
			{
				return true;
			}
		}
		return false;
	}

	void Clear()
	{
		for (auto& table : m_tables)
		{
			table.Clear();
		}
		m_tables.Clear();
		m_nonFullTables.Clear();
		m_count = 0;
	}

	int32_t GetSize() const
	{
		return m_count;
	}

private:
	SparseTableChunk<TableT, MaxTablesCount> m_tables;
	SparseSet<MaxTablesCount> m_nonFullTables;

	int32_t m_count = 0;

	// Iterators

private:
	class SparseTableIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;


		SparseTableIterator(T* currentItem, SparseTableChunk<TableT, MaxTablesCount>& tables) :
			m_currentItem(currentItem),
			m_currentItemIndex(0),
			m_currentTableIndex(0),
			m_tables(tables)
		{
			// we consider non-null current item as first item in the first table
		}

		T& operator*() const
		{
			return *m_currentItem;
		}

		SparseTableIterator& operator++()
		{
			m_currentItemIndex += 1;


			if (m_currentItemIndex >= m_tables[m_currentTableIndex].GetSize())
			{
				m_currentItemIndex = 0;
				m_currentTableIndex += 1;
			}
			if (m_currentTableIndex >= m_tables.GetSize())
			{
				m_currentItem = nullptr;
				return *this;
			}

			m_currentItem = &m_tables[m_currentTableIndex][m_currentItemIndex];
			return *this;
		}

		//SparseTableIterator operator++(int)
		//{
		//	SparseTableIterator tmp = *this;
		//	++(*this);
		//	return tmp;
		//}

		bool operator==(const SparseTableIterator& other) const
		{
			return m_currentItem == other.m_currentItem;
		}

		bool operator!=(const SparseTableIterator& other) const
		{
			return !(*this == other);
		}

	private:
		T* m_currentItem;
		//TableT* m_currentTable;

		int32_t m_currentItemIndex;
		int32_t m_currentTableIndex;

		SparseTableChunk<TableT, MaxTablesCount>& m_tables;
	};

public:
	SparseTableIterator begin()
	{
		return m_tables.GetSize() > 0 ? SparseTableIterator(&m_tables[0][0], m_tables) : end();
	}

	SparseTableIterator end()
	{
		return SparseTableIterator(nullptr, m_tables);
	}
};
#endif // SPARSE_TABLE_H
