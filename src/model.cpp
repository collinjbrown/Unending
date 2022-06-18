#include "model.h"

#include <iostream>

#include "external/fast_obj.h"

void Model::LoadObjFile(const char* path)
{
	fastObjMesh* mesh = fast_obj_read(path);

	for (int i = 0; i < mesh->group_count; i++)
	{
		fastObjGroup& group = mesh->groups[i];
		
		int idx = 0;
		for (int j = 0; j < group.face_count; j++)
		{
			int fv = mesh->face_vertices[group.face_offset + j];

			for (int k = 0; k < fv; k++)
			{
				fastObjIndex mi = mesh->indices[group.index_offset + idx];

				if (mi.p && mi.t && mi.n)
				{
					glm::vec3 pos = glm::vec3(mesh->positions[(mi.p * 3) + 0], mesh->positions[(mi.p * 3) + 1], mesh->positions[(mi.p * 3) + 2]);
					this->vertices.push_back(pos);

					glm::vec2 uv = glm::vec2(mesh->texcoords[(mi.t * 2) + 0], mesh->texcoords[(mi.t * 2) + 1]);
					this->uvs.push_back(uv);

					glm::vec3 norm = glm::vec3(mesh->normals[(mi.n * 3) + 0], mesh->normals[(mi.n * 3) + 1], mesh->normals[(mi.n * 3) + 2]);
					this->normals.push_back(norm);
				}

				idx++;
			}
		}
	}

	fast_obj_destroy(mesh);
}

Model::Model(const char* path)
{
	LoadObjFile(path);
}