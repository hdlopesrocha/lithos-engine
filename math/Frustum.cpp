#include "math.hpp"

Frustum::Frustum(glm::mat4 m)
{
	m = glm::transpose(m);
	m_planes[Left]   = m[3] + m[0];
	m_planes[Right]  = m[3] - m[0];
	m_planes[Bottom] = m[3] + m[1];
	m_planes[Top]    = m[3] - m[1];
	m_planes[Near]   = m[3] + m[2];
	m_planes[Far]    = m[3] - m[2];

	glm::vec3 crosses[Combinations] = {
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Right])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Bottom])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Bottom])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Top]),    glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Top]),    glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Near]),   glm::vec3(m_planes[Far]))
	};

	m_points[0] = intersection<Left,  Bottom, Near>(crosses);
	m_points[1] = intersection<Left,  Top,    Near>(crosses);
	m_points[2] = intersection<Right, Bottom, Near>(crosses);
	m_points[3] = intersection<Right, Top,    Near>(crosses);
	m_points[4] = intersection<Left,  Bottom, Far>(crosses);
	m_points[5] = intersection<Left,  Top,    Far>(crosses);
	m_points[6] = intersection<Right, Bottom, Far>(crosses);
	m_points[7] = intersection<Right, Top,    Far>(crosses);

}

ContainmentType Frustum::test(const AbstractBoundingBox &box) {
    glm::vec3 minp = box.getMin();
    glm::vec3 maxp = box.getMax();

    // Build the 8 corner points of the AABB (homogeneous w=1)
    glm::vec4 corners[8] = {
        glm::vec4(minp.x, minp.y, minp.z, 1.0f),
        glm::vec4(maxp.x, minp.y, minp.z, 1.0f),
        glm::vec4(minp.x, maxp.y, minp.z, 1.0f),
        glm::vec4(maxp.x, maxp.y, minp.z, 1.0f),
        glm::vec4(minp.x, minp.y, maxp.z, 1.0f),
        glm::vec4(maxp.x, minp.y, maxp.z, 1.0f),
        glm::vec4(minp.x, maxp.y, maxp.z, 1.0f),
        glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f)
    };

    bool allInside = true;

    // For each frustum plane, count how many AABB corners are outside (dot < 0)
    for (int i = 0; i < Count; ++i) {
        int outside = 0;
        for (int j = 0; j < 8; ++j) {
            if (glm::dot(m_planes[i], corners[j]) < 0.0f) ++outside;
        }

        // If all corners are outside of a plane, the box is disjoint
        if (outside == 8) return ContainmentType::Disjoint;

        if (outside > 0) allInside = false;
    }

    if (allInside) return ContainmentType::Contains;

    return ContainmentType::Intersects;
}


template<Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
glm::vec3 Frustum::intersection(glm::vec3* crosses)
{
	float D = glm::dot(glm::vec3(m_planes[a]), crosses[ij2k<b, c>::k]);
	glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
		glm::vec3(m_planes[a].w, m_planes[b].w, m_planes[c].w);
	return res * (-1.0f / D);
}