#include "TinyMap.h"

TinyMap::TinyMap() : TinyMap(256, 256)
{
}

TinyMap::TinyMap(size_t maxValue, size_t maxKey)
{
	m_maxValue = maxValue;
	m_maxKey = maxKey;
	clear();
}

void TinyMap::clear()
{
	m_value.resize(m_maxValue);
	std::fill(m_value.begin(), m_value.end(), -1);

	m_key.resize(m_maxKey);
	std::fill(m_key.begin(), m_key.end(), m_maxKey);

	m_numActive = 0;
}

void TinyMap::insert(const std::pair<unsigned int, int>& keyValue)
{
	m_value[keyValue.first] = keyValue.second;
	m_key[keyValue.second] = keyValue.first;
	++m_numActive;
}