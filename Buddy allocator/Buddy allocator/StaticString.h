#pragma once
#include <cstddef>
#include <cstring>
#include <iostream>
#include <algorithm>

template<std::size_t SIZE>
class StaticString
{
private:
	char data[SIZE];
public:
	StaticString();
	StaticString(const char* data);
	StaticString(char c);

	StaticString& append(const StaticString& other);
	StaticString& operator+=(const StaticString& other);
	StaticString operator+(const StaticString& other) const;

	bool operator==(const StaticString& other) const;
	bool operator!=(const StaticString& other) const;

	std::size_t size() const;
	StaticString& reverse();
	StaticString& dropLast();
	char& operator[](std::size_t index);

	char* begin();
	char* end();

	friend std::ostream& operator<<(std::ostream& out, const StaticString& object) {
		out << object.data;
		return out;
	}
};

template<std::size_t SIZE>
inline StaticString<SIZE>::StaticString() {
	data[0] = '\0';
}

template<std::size_t SIZE>
inline StaticString<SIZE>::StaticString(const char* data):StaticString() {
	if (strlen(data) + 1 <= SIZE) {
		strcpy_s(this->data, data);
	}
}

template<std::size_t SIZE>
inline StaticString<SIZE>::StaticString(char c) {
	this->data[0] = c;
	this->data[1] = '\0';
}

template<std::size_t SIZE>
inline StaticString<SIZE>& StaticString<SIZE>::append(const StaticString& other) {
	if (this->size() + other.size() + 1 <= SIZE) {
		strcat_s(this->data, other.data);
	}
	return *this;
}

template<std::size_t SIZE>
inline StaticString<SIZE>& StaticString<SIZE>::operator+=(const StaticString& other)
{
	return this->append(other);
}

template<std::size_t SIZE>
inline StaticString<SIZE> StaticString<SIZE>::operator+(const StaticString& other) const {
	StaticString<SIZE> ans(*this);
	ans += other;
	return ans;
}

template<std::size_t SIZE>
inline bool StaticString<SIZE>::operator==(const StaticString& other) const {
	return strcmp(this->data, other.data) == 0;
}

template<std::size_t SIZE>
inline bool StaticString<SIZE>::operator!=(const StaticString& other) const
{
	return !(*this==other);
}

template<std::size_t SIZE>
inline std::size_t StaticString<SIZE>::size() const {
	return strlen(this->data);
}

template<std::size_t SIZE>
inline StaticString<SIZE>& StaticString<SIZE>::reverse()
{
	std::reverse(begin(), end());
	return *this;
}

template<std::size_t SIZE>
inline StaticString<SIZE>& StaticString<SIZE>::dropLast() {
	const std::size_t length = size();
	if (length > 0) {
		data[length - 1] == '\0';
	}
	return *this;
}

template<std::size_t SIZE>
inline char& StaticString<SIZE>::operator[](std::size_t index) {
	return this->data[index];
}

template<std::size_t SIZE>
inline char* StaticString<SIZE>::begin() {
	return this->data;
}

template<std::size_t SIZE>
inline char* StaticString<SIZE>::end()
{
	return this->data + size();
}

template<std::size_t SIZE>
StaticString<SIZE> operator+(const char* lhs, const StaticString<SIZE>& rhs) {
	StaticString<SIZE> ans;
	if (strlen(lhs) + rhs.size() + 1 <= SIZE) {
		ans = lhs;
		ans += rhs;
	}
	return ans;
}