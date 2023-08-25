#include "ModelLoader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

ModelLoader* ModelLoader::instance = nullptr;

ModelLoader::ModelLoader() {}

Model* ModelLoader::Load(const char* fileName)
{
	// Initialize memory for model data.
	Model* model = new Model();
	std::vector<Vec3f> vertices;
	std::vector<unsigned int> indices;

	// Open file stream to read obj files.
	std::ifstream in;
	in.open(fileName, std::ifstream::in);

	// Ensure file can be read.
	if (in.fail())
	{
		return nullptr;
	}

	// Read file line-by-line while extracting model data.
	std::string line; 
	while (!in.eof())
	{
		// Get the next line from the file.
		std::getline(in, line);

		// Convert line to string stream for processing.
		std::istringstream iss(line.c_str());
		
		// Define a char variable to hold trash data.
		char trash;

		// Check if this line is a vertex or face.
		if (!line.compare(0, 2, "v "))
		{
			// Trash the 'v' char.
			iss >> trash;

			// Declare a new vec3 to hold the vertex.
			Vec3f v;

			// Get the 3 floats and save them in v.
			for (int i = 0; i < 3; i++)
			{
				iss >> v.raw[i];
			}

			// Add the new vertex to the list.
			vertices.push_back(v);
		}
		else if (!line.compare(0, 2, "f "))
		{
			// Trash the extra face data (for now).
			int itrash, idx;

			// Trash the 'f' char.
			iss >> trash;

			// Get first vertex index from each face and then trash the rest (for now).
			while (iss >> idx >> trash >> itrash >> trash >> itrash)
			{
				// OBJ format starts at 1.
				idx--;

				// Add index to the list.
				indices.push_back((unsigned int)idx);
			}
		}

		iss.str("");
	}

	// Print the number of verts and faces.
	std::cout << "Model loaded - verts: " << vertices.size() << ", faces: " << indices.size() << std::endl;

	// Copy memory to model class.
	model->SetVertices(vertices);
	model->SetIndices(indices);

	// Return a pointer to the new model object.
	return model;
}