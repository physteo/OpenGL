#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

struct VertexArray
{
	inline unsigned int getID() const { return id; }
	inline int getNumIndices() const { return numIndices; }
	
	unsigned int id;
	int numIndices;

	bool operator==(const VertexArray& other) const
	{
		return id == other.id;
	}
};

struct VertexArrayHasher
{
	using argument_type = VertexArray;
	using result_type = unsigned long long;

	result_type operator()(const argument_type& f) const
	{
		return std::hash<unsigned long long>()(f.getID());
	}
};
