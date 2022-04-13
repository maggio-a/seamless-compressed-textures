#ifndef LINE_H
#define LINE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

using namespace glm;

class vector;

struct Line2
{
    vec2 o;
    vec2 d;

    Line2(const vec2& origin, const vec2& direction) : o{origin}, d{direction} {}

    vec2 operator()(float t) const {
        return o + t * d;
    }
};

struct Line3
{
    vec3 o;
    vec3 d;

    Line3(const vec3& origin, const vec3& direction) : o{origin}, d{direction} {}

    vec3 operator()(float t) const {
        return o + t * d;
    }

    void normalize();
};

/* Returns the normalized best fitting line (computed using PCA) */
Line3 fitLine(const std::vector<vec3>& points);

#endif // LINE_H
