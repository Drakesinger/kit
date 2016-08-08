#include "Kit/ConvexHull.hpp"
#include "Kit/Types.hpp"

#include <fstream>

bool kit::Plane::inFront(glm::vec3 p)
{
  return (glm::dot(p - this->point, this->normal) >= 0.0f);
}

kit::ConvexHull::ConvexHull()
{

}


kit::ConvexHull::~ConvexHull()
{

}

kit::ConvexHull::Ptr kit::ConvexHull::load(const std::string&filename)
{
  auto returner = std::make_shared<kit::ConvexHull>();
  std::fstream f(filename.c_str(), std::ios_base::binary | std::ios_base::in);
  if (!f)
  {
    std::cout << "Couldn't load convex hull from file " << filename.c_str() << "!" << std::endl;
    return nullptr;
  }

  // Read points
  uint32_t numPoints = kit::readUint32(f);
  for (uint32_t i = 0; i < numPoints; i++)
  {
    returner->m_points.push_back(kit::readVec3(f));
  }

  // Read planes
  uint32_t numPlanes = kit::readUint32(f);
  for (uint32_t i = 0; i < numPlanes; i++)
  {
    kit::Plane newPlane;
    newPlane.point = kit::readVec3(f);
    newPlane.normal = kit::readVec3(f);
    returner->m_planes.push_back(newPlane);
  }

  f.close();

  return returner;
}

bool kit::ConvexHull::overlaps(kit::ConvexHull::Ptr hull)
{
  for (auto currPoint : this->getWorldPoints())
  {
    if (hull->overlaps(currPoint))
    {
      return true;
    }
  }

  for (auto currPoint : hull->getWorldPoints())
  {
    if (this->overlaps(currPoint))
    {
      return true;
    }
  }

  return false;
}

bool kit::ConvexHull::overlaps(glm::vec3 point)
{
  for (auto currPlane : this->getWorldPlanes())
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
  return this->m_points;
}

std::vector<glm::vec3> kit::ConvexHull::getWorldPoints()
{
  std::vector<glm::vec3> worldPoints;

  for (auto currPoint : this->m_points)
  {
    worldPoints.push_back(glm::vec3(this->getTransformMatrix() * glm::vec4(currPoint, 1.0f)));
  }

  return worldPoints;
}

std::vector<kit::Plane> & kit::ConvexHull::getLocalPlanes()
{
  return this->m_planes;
}

std::vector<kit::Plane>  kit::ConvexHull::getWorldPlanes()
{
  std::vector<kit::Plane> worldPlanes;

  for (auto currPlane : this->m_planes)
  {
    kit::Plane newPlane;
    newPlane.normal = glm::vec3(this->getTransformMatrix() * glm::vec4(currPlane.normal, 0.0f));
    newPlane.point = glm::vec3(this->getTransformMatrix() * glm::vec4(currPlane.point, 1.0f));
    worldPlanes.push_back(newPlane);
  }

  return worldPlanes;
}
