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
