#include "hobbyraytracer.h"
#include "mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags



Assimp::Importer Mesh::importer;

Mesh::Mesh(std::string filepath, std::shared_ptr<Material> matPtr)
{
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	HittableList triangleStrip;

	if (assimpLoadFile(filepath, vertices, normals, uvs, indices))
	{
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			ITriangle triangle(
				std::array<glm::vec3, 3>({ vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]] }),
				std::array<glm::vec3, 3>({ normals[indices[i]], normals[indices[i + 1]], normals[indices[i + 2]] }),
				std::array<glm::vec2, 3>({ uvs[indices[i]], uvs[indices[i + 1]], uvs[indices[i + 2]] }),
				matPtr
			);

			triangleStrip.add(std::make_shared<ITriangle>(triangle));
		}
			
		std::cout << "Indexed file: " << filepath << std::endl;
	}

	matPtr = matPtr;
	tree = std::make_shared<BVHNode>(triangleStrip);
}

bool Mesh::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	return tree->hit(r, t_min, t_max, rec);
}

bool Mesh::boundingBox(AABB& outputBox) const
{
	return tree->boundingBox(outputBox);
}

// Copied from https://github.com/Todegal/SummerGameChallenge/blob/master/SummerGameChallenge/src/Model.cpp
bool Mesh::assimpLoadFile(
    std::string path, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, std::vector<unsigned int>& indices)
{
	// Load the model as a scene
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	// If the file could not be loaded
	if (!scene)
	{
		std::cout << "Failed to load file: " << path << std::endl;
		return false;
	}

	// If there is a mesh in the scene
	if (scene->HasMeshes())
	{
		// Ensure the lists are empty
		vertices = {};
		normals = {};
		uvs = {};
		indices = {};

		// For every mesh in the scene
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			// Temporarly store the mesh
			const aiMesh* mesh = scene->mMeshes[i];

			// For each vertex in the mesh
			for (size_t j = 0; j < mesh->mNumVertices; j++)
			{
				// Push the vertex and normals back
				vertices.push_back({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });

				if (mesh->HasNormals())
				{
					normals.push_back({ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z });
				}
				else
				{
					normals.push_back({ 0.0f, 0.0f, 0.0f });
				}

				// If there are any textureCoordinates
				if (mesh->HasTextureCoords(0))
				{
					// Push them back
					uvs.push_back({ mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y });
				}
				else
				{
					uvs.push_back({ 0, 0 });
				}
			}

			// For each face in the mesh
			for (size_t j = 0; j < mesh->mNumFaces; j++)
			{
				aiFace face = mesh->mFaces[j];
				// For each point in the face
				for (size_t k = 0; k < face.mNumIndices; k++)
				{
					// Push back its index
					indices.push_back(face.mIndices[k]);
				}
			}
		}
		
		std::cout << "Loaded file: " << path << std::endl;

		return true;
	}

	return false;
}
