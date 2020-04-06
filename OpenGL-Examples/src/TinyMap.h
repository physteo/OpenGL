#pragma once

#include <vector>

class TinyMap
{
public:
	TinyMap();
	TinyMap(size_t maxValue, size_t maxKey);

	inline int getValue(unsigned int key) { return m_value[key]; }

	inline unsigned int getKey(int unit) { return m_key[unit]; }

	inline size_t size() const { return m_numActive; }

	void insert(const std::pair<unsigned int, int>& keyValue);

	void clear();

private:

	std::vector<int> m_value;
	std::vector<unsigned int> m_key;

	size_t m_maxKey;
	size_t m_maxValue;
	size_t m_numActive;
};
