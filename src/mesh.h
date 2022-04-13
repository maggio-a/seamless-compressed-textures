#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class Image;

using namespace glm;

typedef std::pair<int, int> Edge;
typedef std::pair<Edge, Edge> Seam;

struct Face {
    std::vector<int> pi;
    std::vector<int> ti;

    void read(const std::vector<std::string>& tokens);

    Edge edge3(int i) const {
        return Edge(pi[i], pi[(i+1) % pi.size()]);
    }

    Edge edge2(int i) const {
        return Edge(ti[i], ti[(i+1) % ti.size()]);
    }
};

struct Material {
    std::string name;
    std::string texture; // empty string means no texture

    Material(const std::string& n, const std::string& t = std::string("")) : name{n}, texture{t} {}
};

struct Mesh {
    std::vector<vec3> vvec;
    std::vector<vec2> vtvec;
    std::vector<Face> face;
    std::vector<Seam> seam;
    std::vector<int> mat;

    std::vector<Material> material;
    std::map<std::string, int> materialMap; // name to material index

    std::vector<std::pair<int, int>> objects; // each object is a pair <first_Face_index, num_Faces>

    Mesh(const char *filename) {
        loadObjFile(filename);
    }

    Mesh() {}

    void computeSeams();

    void saveAsHalfEdgeMesh();

    void colorSeams(Image& img);
    void colorCovered(Image& img);

    void mirrorV();

    void denormalizeUV(const Image& img);
    void normalizeUV(const Image& img);

    double lengthUV(const Edge& e, vec2 sz = vec2(1, 1)) const;
    double maxLength(const Seam& s, vec2 sz = vec2(1, 1)) const;

    vec2 uvpos(const Edge& e, double t) const;

    int loadObjFile(const char *path);
    int saveObjFile(const char *meshName, const char *textureName, bool mirrorV = false);
};


#endif // MESH_H
