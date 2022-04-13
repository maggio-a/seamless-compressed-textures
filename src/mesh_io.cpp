#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#include "mesh.h"


static bool whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static std::string rtrim(const std::string& s)
{
    auto r = s.crbegin();
    while (r != s.rend()) {
        if (whitespace(*r)) {
            r++;
        }
        else
            break;
    }

    return s.substr(0, s.rend() - r);
}


std::vector<std::string> parse_face_indices(std::string token);

std::vector<std::string> tokenize_line(std::ifstream& ifs)
{
    std::vector<std::string> tokens;
    std::string line;
    do {
        std::string input;
        std::getline(ifs, input);
        line = rtrim(input);
    } while ((line.empty() || line[0] == '#') && (ifs.eof() == false));

    if ((line.empty() || line[0] == '#'))
        return {};

    auto it = line.begin();
    std::string tok;
    while (it != line.end()) {
        assert(*it != '#');
        if (whitespace(*it) == false) {
            tok.push_back(*it);
        } else {
            if (tok.size() > 0) {
                tokens.push_back(tok);
                tok.clear();
            }
        }
        it++;
    }

    if (tok.size() > 0)
        tokens.push_back(tok);

    return tokens;
}

std::vector<std::string> parse_face_indices(std::string token)
{
    std::vector<std::string> index_string_vec;
    std::string index_string;
    auto it = token.begin();
    while (it != token.end()) {
        if (*it != '/') {
            index_string.push_back(*it);
        } else {
            index_string_vec.push_back(index_string);
            index_string.clear();
        }
        it++;
    }
    if (index_string.size() > 0 || token.back() == '/')
        index_string_vec.push_back(index_string);
    return index_string_vec;
}

void Face::read(const std::vector<std::string>& tokens)
{
    assert(tokens.size() > 3);
    for (unsigned i = 1; i < tokens.size(); ++i) {
        std::vector<std::string> index_string_vec = parse_face_indices(tokens[i]);
        if (index_string_vec.size() > 0)
            pi.push_back(std::atoi(index_string_vec[0].c_str()) - 1);
        if (index_string_vec.size() > 1)
            ti.push_back(std::atoi(index_string_vec[1].c_str()) - 1);
        else
            assert(0 && "No texture coordinate for vertex");
    }
}

int Mesh::loadObjFile(const char *path)
{
    std::ifstream ifs(path);
    assert(ifs.is_open());

    int nHalfEdge = 0;

    int currentMaterial = -1;

    while (!ifs.eof()) {
        std::vector<std::string> tokens = tokenize_line(ifs);

        if (tokens.size() == 0)
            break;

        if (tokens[0] == "v") {
            assert(tokens.size() > 3);
            vec3 p;
            p.x = std::atof(tokens[1].c_str());
            p.y = std::atof(tokens[2].c_str());
            p.z = std::atof(tokens[3].c_str());
            vvec.push_back(p);
        } else if (tokens[0] == "vt") {
            assert(tokens.size() > 2);
            vec2 t;
            t.x = std::atof(tokens[1].c_str());
            t.y = std::atof(tokens[2].c_str());
            vtvec.push_back(t);
        } else if (tokens[0] == "f") {
            assert(tokens.size() > 3);
            Face f;
            f.read(tokens);
            face.push_back(f);
            mat.push_back(currentMaterial);
            nHalfEdge += f.pi.size();
        } else if (tokens[0] == "vn") {
            // do nothing
        } else if (tokens[0] == "mtllib") {

        } else if (tokens[0] == "usemtl") {
            std::string materialName = tokens[1];
            if (materialMap.count(materialName) > 0) {
                currentMaterial = materialMap[materialName];
            } else {
                currentMaterial = material.size();
                material.push_back(Material(materialName));
            }
        } else {
            std::cout << "ignoring line starting with " << tokens[0] << std::endl;
        }
    }

    std::cout << "Mesh has " << nHalfEdge << " half-edges" << std::endl;

    return 0;
}

int Mesh::saveObjFile(const char *meshName, const char *textureName, bool mirrorV)
{
    std::string objFilename = std::string(meshName) + ".obj";
    std::string mtlFilename = std::string(meshName) + ".mtl";

    std::ofstream matFile(mtlFilename);
    if (!matFile) {
        std::cerr << "Error writing material file " << mtlFilename << std::endl;
        return -1;
    }
    matFile << "newmtl Material_0"       << std::endl;
    matFile << "Ka 1.0000 1.0000 1.0000" << std::endl;
    matFile << "Kd 1.0000 1.0000 1.0000" << std::endl;
    matFile << "Ks 0.0000 0.0000 0.0000" << std::endl;
    matFile << "d 1"                     << std::endl;
    matFile << "Ns 0.0000"               << std::endl;
    matFile << "illum 1"                 << std::endl;
    matFile << "map_Kd "  << textureName << std::endl;
    matFile.close();

    std::ofstream meshFile(objFilename);
    if (!meshFile) {
        std::cerr << "Error writing obj file " << objFilename << std::endl;
        return -1;
    }
    meshFile << "mtllib ./" << mtlFilename << std::endl;
    for (auto v : vvec) {
        meshFile << "v " << v.x << " " << v.y << " " << v.z << std::endl;
    }
    for (auto vt : vtvec) {
        meshFile << "vt " << vt.x << " " << (mirrorV ? (1 - vt.y) : vt.y) << std::endl;
    }
    meshFile << "usemtl  Material_0" << std::endl;
    for (auto f : face) {
        meshFile << "f";
        for (unsigned i = 0; i < f.pi.size(); ++i) {
            meshFile << " " << f.pi[i] + 1 << "/" << f.ti[i] + 1;
        }
        meshFile << std::endl;
    }
    meshFile.close();

    return 0;
}


