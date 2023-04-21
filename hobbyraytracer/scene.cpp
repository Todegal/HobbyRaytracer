#include "hobbyraytracer.h"

#include "scene.h"

#include <yaml-cpp/yaml.h>

#include "bvh.h"
#include "hittableList.h"
#include "aarect.h"
#include "rotateQuat.h"
#include "translate.h"
#include "scale.h"

template<typename T>
static T getProperty(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        return prop.as<T>();
    }

    throw YAML::ParserException(YAML::Mark(), "Could not find required property: " + name);
}

template<>
static glm::vec3 getProperty<glm::vec3>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsSequence())
        {
            return glm::vec3(
                prop[0].as<float>(),
                prop[1].as<float>(),
                prop[2].as<float>()
            );
        }

        throw YAML::ParserException(YAML::Mark(), "Invalid value for vector: " + name);
    }

    throw YAML::ParserException(YAML::Mark(), "Could not find required property: " + name);
}

template<>
static glm::vec2 getProperty<glm::vec2>(std::string name, YAML::Node node)
{
    if (YAML::Node prop = node[name])
    {
        if (prop.IsSequence())
        {
            return glm::vec2(
                prop[0].as<float>(),
                prop[1].as<float>()
            );
        }

        throw YAML::ParserException(YAML::Mark(), "Invalid value for vector: " + name);
    }

    throw YAML::ParserException(YAML::Mark(), "Could not find required property: " + name);
}

int Scene::loadScene(std::string path)
{
	objects.clear();
	materials.clear();

    YAML::Node root;

    try {
        root = YAML::LoadFile(path);

        if (YAML::Node filmNode = root["film"])
        {
            int w = getProperty<int>("width", filmNode);
            int h = getProperty<int>("height", filmNode);
            int samples = getProperty<int>("samples", filmNode);
            std::string ouputPath = getProperty<std::string>("output", filmNode);

            Film f(w, h, samples, ouputPath);
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

            Camera c(position, lookAt, up, fov, film.getAspectRatio(), aperture, focusDistance);
            camera = c;

            background = getProperty<glm::vec3>("background", cameraNode);
        }
        else
        {
            std::cout << "Must specify camera descriptor!" << std::endl;
            return -1;
        }

        if (YAML::Node materialsNode = root["materials"])
        {
            for (auto material : materialsNode)
            {
                std::string name = getProperty<std::string>("name", material);
                glm::vec3 albedo = getProperty<glm::vec3>("albedo", material);

                if (getProperty<std::string>("type", material) == "diffuse_light")
                {
                    float strength = getProperty<float>("strength", material);
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
                    float roughness = getProperty<float>("roughness", material);
                    materials[name] = std::make_shared<Metal>(albedo, roughness);
                    continue;
                }
            }
        }
        else
        {
            std::cout << "Couldn't find any material descriptors!" << std::endl;
            return -1;
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
                        std::string path = getProperty<std::string>("file_path", object);

                        o = std::make_shared<Mesh>(path, m);
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

    std::cout << "Loaded scene: " << path << std::endl;

    return 1;
}

std::shared_ptr<HittableList> Scene::getScene(Camera& cam, glm::vec3& b, Film& f)
{
    cam = camera;
    b = background;
    f = film;
    
    return std::make_shared<HittableList>(objects);
}
