#include <map>
#include <set>
#include <iostream>
#include <glm/geometric.hpp>

#include "mesh.h"
#include "image.h"

void Mesh::colorSeams(Image& img)
{
    for (auto s : seam) {
        img.drawLine(vtvec[s.first.first], vtvec[s.first.second], vec3(255, 0, 0));
        img.drawLine(vtvec[s.second.first], vtvec[s.second.second], vec3(0, 0, 255));
    }
}

void Mesh::colorCovered(Image& img)
{
    std::vector<int> cover;
    //generateTextureCoverageBuffer(img, cover, true);

    for (int y = 0; y < img.resy; ++y)
    for (int x = 0; x < img.resx; ++x) {
        if (cover[img.indexOf(x, y)])
            img.pixel(x, y) = vec3(255);
        else
            img.pixel(x, y) = vec3(0);
    }
}

void Mesh::mirrorV()
{
    for (auto& tc : vtvec)
        tc.y = 1 - tc.y;
}

void Mesh::denormalizeUV(const Image& img)
{
    for (auto& tc : vtvec) {
        tc.y = 1 - tc.y;
        tc *= vec2(img.resx, img.resy);
    }
}

void Mesh::normalizeUV(const Image& img)
{
     for (auto& tc : vtvec) {
        tc /= vec2(img.resx, img.resy);
        tc.y = 1 - tc.y;
    }
}

void Mesh::computeSeams()
{
    std::map<Edge, std::set<Edge>> edgeMap;
    for (const Face& f : face) {
        for (unsigned i = 0; i < f.pi.size(); ++i) {
            Edge e3 = f.edge3(i);
            Edge e2 = f.edge2(i);
            if (e3.first > e3.second) {
                std::swap(e3.first, e3.second);
                std::swap(e2.first, e2.second);
            }
            edgeMap[e3].insert(e2);
        }
    }

    seam.clear();
    for (const auto& entry : edgeMap) {
        const std::set<Edge>& s = entry.second;
        if (s.size() > 1) {
            auto e = s.begin();
            Edge e1 = *e;
            ++e;
            Edge e2 = *e;
            seam.push_back(Seam(e1, e2));
        }
    }
    std::cout << "Found " << seam.size() << " seams" << std::endl;
}

double Mesh::lengthUV(const Edge& e, vec2 sz) const
{
    return glm::distance(vtvec[e.first] * sz, vtvec[e.second] * sz);
}

double Mesh::maxLength(const Seam& s, vec2 sz) const
{
    return std::max(lengthUV(s.first, sz), lengthUV(s.second, sz));
}

vec2 Mesh::uvpos(const Edge& e, double t) const
{
    return glm::mix(vtvec[e.first], vtvec[e.second], t);
}
