#ifndef VEC3_H
#define VEC3_H


/* PREAMBLE: from VCG syntax to GLSL-ish syntax */
//#include <vcg/space/deprecated_point2.h>
//#include <vcg/space/deprecated_point3.h>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
using namespace glm;

typedef unsigned int uint;

typedef double scalar;

inline scalar getx(vec2 v){return v.x;}
inline scalar gety(vec2 v){return v.y;}
inline scalar getx(vec3 v){return v.x;}
inline scalar gety(vec3 v){return v.y;}
inline scalar getz(vec3 v){return v.z;}

inline vec3 getx(mat3 v){return v[0];}
inline vec3 gety(mat3 v){return v[1];}
inline vec3 getz(mat3 v){return v[2];}

//typedef vcg::Point3<scalar> vec3;
//typedef vcg::Point2<scalar> vec2;

/*inline scalar getx(vec2 v){return v.X();}
inline scalar gety(vec2 v){return v.Y();}
inline scalar getx(vec3 v){return v.X();}
inline scalar gety(vec3 v){return v.Y();}
inline scalar getz(vec3 v){return v.Z();}
inline scalar dot(vec2 a, vec2 b){ return a*b;}
inline scalar dot(vec3 a, vec3 b){ return a*b;}
inline scalar cross(vec2 a, vec2 b){ return a^b;}
inline vec3 cross(vec3 a, vec3 b){ return a^b;}*/
//inline vec3 normalize(vec3 a){ return a.normalized();}
//inline scalar length(const vec3 &a){ return a.Norm();}
inline scalar length2(const vec3 &a){ return dot(a,a);}
//inline scalar distance(vec3 a, vec3 b){ return length(a-b);}
inline scalar distance2(vec3 a, vec3 b){ return dot(a-b,a-b);}

inline vec3 ortogolalize( vec3 a, vec3 n) { return cross(n, cross(a,n) ); }


inline vec3 swizzleZXY( vec3 a ) {return vec3(a.z,a.x,a.y); }

#endif // VEC3_H
