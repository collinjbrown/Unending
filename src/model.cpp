#include "model.h"

Model::Model(const char* file)
{
	loader.LoadFile(file);

	std::vector<objl::Vertex> objVertices = loader.LoadedVertices;

	for (int i = 0; i < objVertices.size(); i++)
	{
		objl::Vertex ver = objVertices[i];

		Vertex v =
		{
			ver.Position.X,
			ver.Position.Y,
			ver.Position.Z,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			ver.TextureCoordinate.X,
			ver.TextureCoordinate.Y,
			0
		};

		vertices.push_back(v);
	}
}