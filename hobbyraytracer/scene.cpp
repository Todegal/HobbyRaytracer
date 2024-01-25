#include "hobbyraytracer.h"

#include "scene.h"

#include "bvh.h"
#include "hittableList.h"
#include "aarect.h"
#include "rotateQuat.h"
#include "translate.h"
#include "scale.h"
#include "sphere.h"

template<typename T>
T Scene::getProperty(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        return prop.as<T>();
    }

    throw YAML::ParserException(node.Mark(), "Could not find required property: " + name);
}

template<>
static glm::vec3 Scene::getProperty<glm::vec3>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsSequence())
        {
            std::vector<float> vi = prop.as<std::vector<float>>();

            if (vi.size() != 3)
                throw YAML::ParserException(prop.Mark(), "Invalid size for vector 3: " + name);

            return glm::vec3(
                vi[0],
                vi[1],
                vi[2]
            );
        }

        throw YAML::ParserException(prop.Mark(), "Invalid value for vector 3: " + name);
    }

    throw YAML::ParserException(node.Mark(), "Could not find required property: " + name);
}

template<>
glm::vec2 Scene::getProperty<glm::vec2>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsSequence())
        {
            std::vector<float> vi = prop.as<std::vector<float>>();

            if (vi.size() != 2)
                throw YAML::ParserException(prop.Mark(), "Invalid size for vector 2: " + name);

            return glm::vec2(
                vi[0],
                vi[1]
            );
        }

        throw YAML::ParserException(prop.Mark(), "Invalid value for vector 2: " + name);
    }

    throw YAML::ParserException(node.Mark(), "Could not find required property: " + name);
}

template<>
MatVec3 Scene::getProperty<MatVec3>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsSequence())
        {
            return getProperty<glm::vec3>(name, node);
        }
        else
        {
            std::string textureName = getProperty<std::string>(name, node);
            if (textures.count(textureName) == 1)
            {
                return textures[textureName];
            }
            else {
                textures[textureName] = std::make_shared<ImageTexture>(textureName);
                return textures[textureName];
            }
        }
    }

    throw YAML::ParserException(node.Mark(), "Could not find required property: " + name);
}

template<>
MatScalar Scene::getProperty<MatScalar>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsScalar())
        {
            return getProperty<float>(name, node);
        }
        else
        {
            std::string name = getProperty<std::string>(name, node);
            if (textures.count(name) == 1)
            {
                return textures[name];
            }
            else {
                textures[name] = std::make_shared<ImageTexture>(name);
                return textures[name];
            }
        }
    }

    throw YAML::ParserException(node.Mark(), "Could not find required property: " + name);
}

int Scene::loadScene(std::string path)
{
	objects.clear();
	materials.clear();
    textures.clear();

    YAML::Node root;

    try {
        root = YAML::LoadFile(path);

        std::cout << "Loading scene: " << path << std::endl;

        if (YAML::Node filmNode = root["film"])
        {
            int w = getProperty<int>("width", filmNode);
            int h = getProperty<int>("height", filmNode);
            int samples = getProperty<int>("samples", filmNode);
            std::string ouputPath = getProperty<std::string>("output", filmNode);

            std::shared_ptr<Film> f = std::make_shared<Film>(w, h, samples, ouputPath);
            film = f;
        }
        else 
        {
            std::cout << "Must specify film descriptor!" << std::endl;
            return -1;
        }

        if (YAML::Node cameraNode = root["camera"])
        {
            glm::vec3 position = getProperty<glm::vec3>("position", cameraNode);
            glm::vec3 lookAt = getProperty<glm::vec3>("look_at", cameraNode);
            glm::vec3 up = getProperty<glm::vec3>("up", cameraNode);
            float fov = getProperty<float>("fov", cameraNode);
            float aperture = getProperty<float>("aperture", cameraNode);
            float focusDistance = getProperty<float>("focal_distance", cameraNode);

            Camera c(position, lookAt, up, fov, film->getAspectRatio(), aperture, focusDistance);
            camera = c;
        }
        else
        {
            std::cout << "Must specify camera descriptor!" << std::endl;
            return -1;
        }

        if (YAML::Node texturesNode = root["textures"])
        {
            for (auto texture : texturesNode)
            {
                std::string name = getProperty<std::string>("name", texture);
                if (textures.count(name) > 0)
                {
                    throw YAML::ParserException(texture.Mark(), "Texture name already exists!");
                }

                if (getProperty<std::string>("type", texture) == "solid")
                {
                    glm::vec3 colour = getProperty<glm::vec3>("colour", texture);
                    
                    textures[name] = std::make_shared<SolidColourTexture>(colour);
                }

                if (getProperty<std::string>("type", texture) == "image")
                {
                    std::string path = getProperty<std::string>("path", texture);

                    textures[name] = std::make_shared<ImageTexture>(path);
                }

                if (getProperty<std::string>("type", texture) == "checkered")
                {
                    glm::vec3 even = getProperty<glm::vec3>("even", texture);
                    glm::vec3 odd = getProperty<glm::vec3>("odd", texture);

                    textures[name] = std::make_shared<CheckeredTexture>(even, odd);
                }

                if (getProperty<std::string>("type", texture) == "environment")
                {
                    std::string path = getProperty<std::string>("path", texture);

                    textures[name] = std::make_shared<EnvironmentMap>(path);
                }
            }
        }

        if (YAML::Node bg = root["camera"]["background"])
        {
            if (bg.IsSequence())
            {
                background = std::make_shared<SolidColourTexture>(getProperty<glm::vec3>("background", root["camera"]));
            }
            else
            {
                std::string textureName = getProperty<std::string>("background", root["camera"]);
                if (textures.count(textureName) == 1)
                {
                    background = textures[textureName];
                }
                else {
                    textures[textureName] = std::make_shared<EnvironmentMap>(textureName);
                    background = textures[textureName];
                }
            }
        }
        else
        {
            throw YAML::ParserException(root["camera"].Mark(), "Could not find required property: background");
        }

        if (YAML::Node materialsNode = root["materials"])
        {
            for (auto material : materialsNode)
            {
                std::string name = getProperty<std::string>("name", material);
                MatVec3 albedo = getProperty<MatVec3>("albedo", material);

                if (getProperty<std::string>("type", material) == "diffuse_light")
                {
                    MatScalar strength = getProperty<MatScalar>("strength", material);
                    materials[name] = std::make_shared<DiffuseLight>(albedo, strength);
                    continue;
                }

                if (getProperty<std::string>("type", material) == "lambertian")
                {
                    materials[name] = std::make_shared<Lambertian>(albedo);
                    continue;
                }

                if (getProperty<std::string>("type", material) == "metal")
                {
                    MatScalar roughness = getProperty<MatScalar>("roughness", material);
                    materials[name] = std::make_shared<Metal>(albedo, roughness);
                    continue;
                }
            }
        }
        else
        {
            std::cout << "Couldn't find any material descriptors!" << std::endl;
        }

        if (YAML::Node objectsNode = root["objects"])
        {
            if (objectsNode.IsSequence())
            {
                for (auto it = objectsNode.begin(); it != objectsNode.end(); it++)
                {
                    auto object = *it;
                    std::shared_ptr<Hittable> o;

                    std::string materialKey = getProperty<std::string>("material", object);
                    std::shared_ptr<Material> m;

                    if (materials.count(materialKey) == 1) {
                        m = materials[materialKey];
                    }
                    else {
                        std::cout << "Material " << materialKey << " does not exist!" << std::endl;
                        continue;
                    }

                    if (getProperty<std::string>("type", object) == "mesh")
                    {
                        std::string path = getProperty<std::string>("path", object);

                        o = std::make_shared<Mesh>(path, m);
                    }

                    if (getProperty<std::string>("type", object) == "sphere")
                    {
                        glm::vec3 c = getProperty<glm::vec3>("center", object);
                        float radius = getProperty<float>("radius", object);

                        o = std::make_shared<Sphere>(c, radius, m);
                    }

                    if (getProperty<std::string>("type", object) == "yz_rect")
                    {
                        glm::vec2 Y = getProperty<glm::vec2>("y", object);
                        glm::vec2 Z = getProperty<glm::vec2>("z", object);
                        float k = getProperty<float>("k", object);

                        o = std::make_shared<YZRect>(Y.x, Y.y, Z.x, Z.y, k, m);
                    }

                    if (getProperty<std::string>("type", object) == "xz_rect")
                    {
                        glm::vec2 X = getProperty<glm::vec2>("x", object);
                        glm::vec2 Z = getProperty<glm::vec2>("z", object);
                        float k = getProperty<float>("k", object);

                        o = std::make_shared<XZRect>(X.x, X.y, Z.x, Z.y, k, m);
                    }

                    if (getProperty<std::string>("type", object) == "xy_rect")
                    {
                        glm::vec2 X = getProperty<glm::vec2>("x", object);
                        glm::vec2 Y = getProperty<glm::vec2>("y", object);
                        float k = getProperty<float>("k", object);

                        o = std::make_shared<XYRect>(X.x, X.y, Y.x, Y.y, k, m);
                    }

                    // HANDLE TRANSFORMATIONS
                    if (YAML::Node transformNode = object["transform"])
                    {
                        if (transformNode["rotate"])
                        {
                            glm::vec3 angles = getProperty<glm::vec3>("rotate", transformNode);
                            o = std::make_shared<RotateQuat>(o, glm::quat(glm::radians(angles)));
                        }

                        if (transformNode["scale"])
                        {
                            glm::vec3 factor = getProperty<glm::vec3>("scale", transformNode);
                            o = std::make_shared<Scale>(o, factor);
                        }

                        if (transformNode["translate"])
                        {
                            glm::vec3 offset = getProperty<glm::vec3>("translate", transformNode);
                            o = std::make_shared<Translate>(o, offset);
                        }
                    }

                    objects.add(o);
                }
            }
        }
        else
        {
            std::cout << "Couldn't find any object descriptors!" << std::endl;
        }

    }
    catch (const YAML::Exception& ex) {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    isLoaded = true;

    return 1;
}

std::shared_ptr<HittableList> Scene::getScene()
{
    return std::make_shared<HittableList>(objects);
}
