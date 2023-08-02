#include <map>
#include <chrono>
#include <iostream>
#include <unordered_map>

#include "SparseTable.h"

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

class ScopedClock
{
public:
	ScopedClock() = delete;

	ScopedClock(const char* msg):
		m_timePoint(std::chrono::high_resolution_clock::now()),
		m_msg(msg)
	{
	}

	~ScopedClock()
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> dur = now - m_timePoint;
		std::cout << m_msg << ": " << dur.count() << " ms" << std::endl;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> m_timePoint;
	const char* m_msg;
};


int main()
{
	SparseTable<std::string, 2, 4> set;
	auto a = set.Emplace("Alice");
	auto b = set.Emplace("Bob");
	auto c = set.Emplace("Sasha");
	auto d = set.Emplace("George");

	auto e = set.Emplace("Pavel");
	auto f = set.Emplace("Irina");
	auto g = set.Emplace("Grisha");
	auto h = set.Emplace("Clara");

	set.Remove(g);

	g = set.Emplace("Ivan");

	for (auto&& str : set)
	{
		std::cout << str << std::endl;
	}

	set.Clear();


	//a = set.Emplace("Alice");
	//b = set.Emplace("Bob");
	//c = set.Emplace("Sasha");
	//d = set.Emplace("George");

	//e = set.Emplace("Pavel");
	//f = set.Emplace("Irina");
	//g = set.Emplace("Paul");
	//h = set.Emplace("Clara");

	//

	//set.Remove(b);

	//for (auto&& str : set)
	//{
	//	std::cout << str << std::endl;
	//}

	//set.Clear();


	constexpr int maxIndex = 1'000'001;

	std::unordered_map<int, std::string> testMap;
	SparseTable<std::string, 1, maxIndex> testStaticSet; // one big table with 1M elements
	SparseTable<std::string, 10'001, 100> testDynamicSet; // 10K tables with 100 elements

	{
		auto timer = ScopedClock("Insertion to map...................");

		for (int i = 0; i < maxIndex; i++)
		{
			testMap.insert({i, "Alice"});
		}
	}

	{
		auto timer = ScopedClock("Insertion to static sparse table...");

		for (int i = 0; i < maxIndex; i++)
		{
			testStaticSet.Emplace("Alice");
		}
	}

	{
		auto timer = ScopedClock("Insertion to dynamic sparse table..");

		for (int i = 0; i < maxIndex; i++)
		{
			testDynamicSet.Emplace("Alice");
		}
	}


	{
		auto timer = ScopedClock("Foreach on map.....................");

		for (int i = 0; i < maxIndex; i++)
		{
			testMap.at(i).replace(0, 1, "B");
		}
	}


	{
		auto timer = ScopedClock("Foreach on static sparse table.....");

		for (auto && item: testStaticSet)
		{
			item.replace(0, 1, "B");
		}
		//for (int i = 0; i < maxIndex; i++)
		//{
		//	testStaticSet.At(i).replace(0, 1, "B");
		//}
	}

	{
		auto timer = ScopedClock("Foreach on dynamic sparse table....");

		for (auto&& item : testDynamicSet)
		{
			item.replace(0, 1, "B");
		}
		//for (int i = 0; i < maxIndex; i++)
		//{
		//	testDynamicSet.At(i).replace(0, 1, "B");
		//}
	}

	//{
	//	auto timer = ScopedClock("Full removing from map.............");

	//	for (int i = 0; i < maxIndex; i++)
	//	{
	//		testMap.erase(i);
	//	}
	//}


	//{
	//	auto timer = ScopedClock("Full removing from static sparse table......");

	//	for (int i = 0; i < maxIndex; i++)
	//	{
	//		testStaticSet.Remove(i);
	//	}
	//}

	//{
	//	auto timer = ScopedClock("Full removing from dynamic sparse table.....");

	//	for (int i = 0; i < maxIndex; i++)
	//	{
	//		testDynamicSet.Remove(i);
	//	}
	//}

	{
		auto timer = ScopedClock("Random removing from map...........");

		for (int i = 0; i < maxIndex; i++)
		{
			int indexToRemove = i;
			if (i%2 == 1)
			{
				indexToRemove = maxIndex - 1 - i;
			}
			testMap.erase(indexToRemove);
		}
	}


	{
		auto timer = ScopedClock("Random removing from static sparse table......");

		for (int i = 0; i < maxIndex; i++)
		{
			int indexToRemove = i;
			if (i % 2 == 1)
			{
				indexToRemove = maxIndex - 1 - i;
			}
			testStaticSet.Remove(indexToRemove);
		}
	}

	{
		auto timer = ScopedClock("Random removing from dynamic sparse table.....");

		for (int i = 0; i < maxIndex; i++)
		{
			int indexToRemove = i;
			if (i % 2 == 1)
			{
				indexToRemove = maxIndex - 1 - i;
			}
			testDynamicSet.Remove(indexToRemove);
		}
	}


	return 0;
}
