#include "hobbyraytracer.h"
#include "mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <ranges>
#include <vector>

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
	}

	matPtr = matPtr;
	tree = std::make_shared<BVHNode>(triangleStrip);

	std::cout << "Indexed file: " << filepath << std::endl;
}

bool Mesh::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	return tree->hit(r, t_min, t_max, rec);
}

bool Mesh::boundingBox(AABB& outputBox)
{
	return tree->boundingBox(outputBox);
}

bool Mesh::assimpLoadFile(
    std::string path, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, std::vector<unsigned int>& indices)
{
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        std::vector<glm::vec3> meshVertices;
        std::vector<glm::vec3> meshNormals;
        std::vector<glm::vec2> meshUVs;
        std::vector<unsigned int> meshIndices;
        meshVertices.reserve(mesh->mNumVertices);

        if (mesh->HasNormals()) {
            meshNormals.reserve(mesh->mNumVertices);
        }
        if (mesh->HasTextureCoords(0)) {
            meshUVs.reserve(mesh->mNumVertices);
        }
        meshIndices.reserve(mesh->mNumFaces * 3); // Assume triangle faces

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            aiVector3D v = mesh->mVertices[j];
            meshVertices.push_back(glm::vec3(v.x, v.y, v.z));

            if (mesh->HasNormals()) {
                aiVector3D n = mesh->mNormals[j];
                meshNormals.push_back(glm::vec3(n.x, n.y, n.z));
            }
            else
            {
                meshNormals.push_back(glm::vec3(0, 0, 0));
            }

            if (mesh->HasTextureCoords(0)) {
                aiVector3D t = mesh->mTextureCoords[0][j];
                meshUVs.push_back(glm::vec2(t.x, t.y));
            }
            else
            {
                meshUVs.push_back(glm::vec2(0, 0));
            }

        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];

            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                meshIndices.push_back(face.mIndices[k]);
            }
        }

        vertices.insert(vertices.end(), meshVertices.begin(), meshVertices.end());
        normals.insert(normals.end(), meshNormals.begin(), meshNormals.end());
        uvs.insert(uvs.end(), meshUVs.begin(), meshUVs.end());
        indices.insert(indices.end(), meshIndices.begin(), meshIndices.end());
    }

    std::cout << "Loaded mesh: " << path << std::endl;

    return true;
}
