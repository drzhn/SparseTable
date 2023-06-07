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


int main()
{
	SparseTable<std::string, 2, 4> set;
	auto a = set.Emplace("Alice");
	auto b = set.Emplace("Bob");
	auto c = set.Emplace("Sasha");
	auto d = set.Emplace("George");

	auto e = set.Emplace("Pavel");
	auto f = set.Emplace("Irina");
	auto g = set.Emplace("Paul");
	auto h = set.Emplace("Clara");

	set.Remove(b);
	//set.Remove(g);

	//b = set.Emplace("Zurab");
	//g = set.Emplace("Ivan");

	for (auto&& str : set)
	{
		std::cout << str << std::endl;
	}

	set.Clear();


	a = set.Emplace("Alice");
	b = set.Emplace("Bob");
	c = set.Emplace("Sasha");
	d = set.Emplace("George");

	e = set.Emplace("Pavel");
	f = set.Emplace("Irina");
	g = set.Emplace("Paul");
	h = set.Emplace("Clara");

	set.Remove(b);

	for (auto&& str : set)
	{
		std::cout << str << std::endl;
	}

	set.Clear();

	return 0;
}
