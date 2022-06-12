#include "hobbyraytracer.h"
#include "rotateY.h"

RotateY::RotateY(std::shared_ptr<Hittable> p, float angle)
    : ptr(p)
{
    float radians = glm::radians(angle);
    sinTheta = glm::sin(radians);
    cosTheta = glm::cos(radians);

    hasBox = ptr->boundingBox(bBox);

    glm::vec3 min(INFINITY);
    glm::vec3 max(-INFINITY);

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                float x = i * bBox.getMax().x + (1 - i) * bBox.getMin().x;
                float y = j * bBox.getMax().y + (1 - j) * bBox.getMin().y;
                float z = k * bBox.getMax().z + (1 - k) * bBox.getMin().z;

                glm::vec3 tester = {
                    cosTheta * x + sinTheta * z,
                    y,
                    -sinTheta * x + cosTheta * z
                };
                
                for (int c = 0; c < 3; c++)
                {
                    min[c] = glm::min(min[c], tester[c]);
                    max[c] = glm::max(max[c], tester[c]);
                }
            }
        }
    }

    bBox = AABB(min, max);
}

bool RotateY::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 origin = r.getOrigin(); 
    glm::vec3 direction = r.getDirection();

    origin[0] = cosTheta * r.getOrigin()[0] - sinTheta * r.getOrigin()[2];
    origin[2] = sinTheta * r.getOrigin()[0] + cosTheta * r.getOrigin()[2];

    direction[0] = cosTheta * r.getDirection()[0] - sinTheta * r.getDirection()[2];
    direction[2] = sinTheta * r.getDirection()[0] + cosTheta * r.getDirection()[2];

    ray rotatedR(origin, direction);

    if (!ptr->hit(rotatedR, t_min, t_max, rec))
    {
        return false;
    }

    glm::vec3 p = rec.p;
    glm::vec3 normal = rec.normal;

    p[0] = cosTheta * rec.p[0] + sinTheta * rec.p[2];
    p[2] = -sinTheta * rec.p[0] + cosTheta * rec.p[2];

    normal[0] = cosTheta * rec.normal[0] + sinTheta * rec.normal[2];
    normal[2] = -sinTheta * rec.normal[0] + cosTheta * rec.normal[2];

    rec.p = p;
    rec.setFaceNormal(rotatedR, normal);

    return true;
}

bool RotateY::boundingBox(AABB& outputBox) const
{
    outputBox = bBox;
    return hasBox;
}
