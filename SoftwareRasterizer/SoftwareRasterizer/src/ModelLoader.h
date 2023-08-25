#pragma once

#include "Geometry.h"
#include "Model.h"

#include <vector>

class ModelLoader
{
private:

	static ModelLoader* instance;

	ModelLoader();

	ModelLoader(const ModelLoader&) = delete;
	ModelLoader& operator=(const ModelLoader&) = delete;

public:

	static ModelLoader* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new ModelLoader();
		}

		return instance;
	}

	Model* Load(const char* fileName);
};