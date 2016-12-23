#include "Kit/ConvexHull.hpp"
#include "Kit/Types.hpp"

#include <fstream>

bool kit::Plane::inFront(glm::vec3 p)
{
  return (glm::dot(p - point, normal) >= 0.0f);
}

kit::ConvexHull::ConvexHull(const std::string&filename)
{
  std::fstream f(filename.c_str(), std::ios_base::binary | std::ios_base::in);
  if (!f)
  {
    std::cout << "Couldn't load convex hull from file " << filename.c_str() << "!" << std::endl;
    KIT_THROW("couldn't create hull");
  }

  // Read points
  uint32_t numPoints = kit::readUint32(f);
  for (uint32_t i = 0; i < numPoints; i++)
  {
    m_points.push_back(kit::readVec3(f));
  }

  // Read planes
  uint32_t numPlanes = kit::readUint32(f);
  for (uint32_t i = 0; i < numPlanes; i++)
  {
    kit::Plane newPlane;
    newPlane.point = kit::readVec3(f);
    newPlane.normal = kit::readVec3(f);
    m_planes.push_back(newPlane);
  }

  f.close();
}


kit::ConvexHull::~ConvexHull()
{

}

bool kit::ConvexHull::overlaps(kit::ConvexHull * hull)
{
  for (auto currPoint : getWorldPoints())
  {
    if (hull->overlaps(currPoint))
    {
      return true;
    }
  }

  for (auto currPoint : hull->getWorldPoints())
  {
    if (overlaps(currPoint))
    {
      return true;
    }
  }

  return false;
}

bool kit::ConvexHull::overlaps(glm::vec3 point)
{
  for (auto currPlane : getWorldPlanes())
  {
    if (currPlane.inFront(point))
    {
      return false;
    }
  }

  return true;
}

std::vector<glm::vec3> & kit::ConvexHull::getLocalPoints()
{
  return m_points;
}

std::vector<glm::vec3> kit::ConvexHull::getWorldPoints()
{
  std::vector<glm::vec3> worldPoints;

  for (auto currPoint : m_points)
  {
    worldPoints.push_back(glm::vec3(getTransformMatrix() * glm::vec4(currPoint, 1.0f)));
  }

  return worldPoints;
}

std::vector<kit::Plane> & kit::ConvexHull::getLocalPlanes()
{
  return m_planes;
}

std::vector<kit::Plane>  kit::ConvexHull::getWorldPlanes()
{
  std::vector<kit::Plane> worldPlanes;

  for (auto currPlane : m_planes)
  {
    kit::Plane newPlane;
    newPlane.normal = glm::vec3(getTransformMatrix() * glm::vec4(currPlane.normal, 0.0f));
    newPlane.point = glm::vec3(getTransformMatrix() * glm::vec4(currPlane.point, 1.0f));
    worldPlanes.push_back(newPlane);
  }

  return worldPlanes;
}
