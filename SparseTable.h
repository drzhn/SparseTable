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
	}

private:
	SparseTableChunk<TableT, MaxTablesCount> m_tables;
	SparseSet<MaxTablesCount> m_nonFullTables;
};
#endif // SPARSE_TABLE_H
