#include "line.h"

#include <vector>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

void Line3::normalize()
{
    d = glm::normalize(d);
}

Line3 fitLine(const std::vector<vec3>& points)
{
    assert(points.size() > 0);

    if (points.size() == 1) {
        return Line3(vec3(points[0].x, points[0].y, points[0].z), vec3(1, 0, 0));
    } else if (points.size() == 2) {
        float d = glm::distance(points[1], points[0]);
        if (d > 0)
            return Line3(glm::mix(points[0], points[1], 0.5), glm::normalize(points[1] - points[0]));
        else
            return Line3(vec3(points[0].x, points[0].y, points[0].z), vec3(1, 0, 0));
    }

    assert(points.size() > 2);

    Eigen::MatrixXd X(points.size(), 3);
    for (unsigned i = 0; i < points.size(); ++i)
        X.row(i) = Eigen::Vector3d(points[i].x, points[i].y, points[i].z);


    Eigen::Vector3d c = X.colwise().mean();
    X.rowwise() -= c.transpose();

    Eigen::Matrix3d M = X.transpose() * X;

    // compute principal component
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es;
    es.compute(M);
    assert(es.info() == Eigen::Success);

    int k;
    es.eigenvalues().maxCoeff(&k);
    Eigen::VectorXd p = es.eigenvalues();
    Eigen::VectorXd pc = es.eigenvectors().col(k);

    Line3 l(vec3(c.x(), c.y(), c.z()), vec3(pc.x(), pc.y(), pc.z()));

    assert(std::isfinite(l.o.x));
    assert(std::isfinite(l.o.y));
    assert(std::isfinite(l.o.z));
    assert(std::isfinite(l.d.x));
    assert(std::isfinite(l.d.y));
    assert(std::isfinite(l.d.z));
    assert(l.d != vec3(0));

    l.normalize();

    return l;
}
