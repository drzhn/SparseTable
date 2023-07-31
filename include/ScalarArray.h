#ifndef SCALAR_ARRAY_H
#define SCALAR_ARRAY_H

#include <type_traits>

template <typename T, size_t Size> requires std::is_trivial_v<T>
class ScalarArray
{
public:
	ScalarArray() = default;
	~ScalarArray() = default;

	ScalarArray& operator=(ScalarArray&& other) = delete;
	ScalarArray(ScalarArray&& other) = delete;
	ScalarArray(const ScalarArray& other) = delete;
	ScalarArray& operator=(const ScalarArray& other) = delete;

	T& operator[](int index)
	{
		return m_data[index];
	}

private:
	T m_data[Size];
};
#endif // SCALAR_ARRAY_H
