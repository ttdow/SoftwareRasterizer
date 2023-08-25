#include "Model.h"

unsigned int Model::GetNumVerts()
{
	return this->vertices.size();
}

unsigned int Model::GetNumInds()
{
	return this->indices.size();
}

Vec3f Model::GetVert(unsigned int i)
{
	return this->vertices[i];
}

unsigned int Model::GetInd(unsigned int i)
{
	return this->indices[i];
}

void Model::SetVertices(std::vector<Vec3f> v)
{
	unsigned int size = v.size();

	this->vertices.reserve(size);
	this->vertices = v;
}

void Model::SetIndices(std::vector<unsigned int> i)
{
	unsigned int size = i.size();

	this->indices.reserve(size);
	this->indices = i;
}