#ifndef MODEL_H
#define MODEL_H

#include <map>
#include "renderer.h"
#include "external/obj_loader.h"

struct Vertex;

class Model
{
public:
	std::vector<Vertex> vertices;

	std::string name;
	objl::Loader loader;
	Model(const char* file);
};

#endif