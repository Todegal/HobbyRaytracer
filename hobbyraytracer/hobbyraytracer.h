#pragma once

// STL

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <memory>
#include <random>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <string>

// GLM

#include <glm\glm.hpp>
#include <glm\common.hpp>
#include <glm\geometric.hpp>
#include <glm\vec3.hpp>
#include <glm\mat3x3.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\random.hpp>
#include <glm\gtc\epsilon.hpp>
#include <glm\gtx\common.hpp>
#include "glm/gtx/string_cast.hpp"

// UTILITY FUNCTIONS

static bool nearZero(glm::vec3 e) {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
}