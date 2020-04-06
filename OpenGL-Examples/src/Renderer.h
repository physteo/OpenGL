#pragma once

#include "VertexArray.h"
#include "Transform.h"
#include "Material.h"
#include <GLCoreUtils.h>

class Renderer
{
public:
	virtual ~Renderer() {}

	virtual void init() = 0;
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void submit(VertexArray vertexArray, Transform transform, Material material) = 0;
	virtual void draw(const GLCore::Utils::Shader& shader) = 0;

	virtual unsigned getNumDrawCalls() const = 0;
	virtual float getAvgFlushBatchSize() const = 0;
	virtual void setMaxTextureUnits(int maxTextureUnitsIn) = 0;
};
