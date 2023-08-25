#pragma once

#include "Geometry.h"

#include <vector>

class Model
{
public:

	void SetVertices(std::vector<Vec3f> v);
	void SetIndices(std::vector<unsigned int> i);

	unsigned int GetNumVerts();
	unsigned int GetNumInds();
	Vec3f GetVert(unsigned int i);
	unsigned int GetInd(unsigned int i);

private:

	std::vector<Vec3f> vertices;
	std::vector<unsigned int> indices;
};