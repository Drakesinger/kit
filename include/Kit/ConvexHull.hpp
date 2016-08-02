#ifndef KIT_CONVEXHULL_HPP
#define KIT_CONVEXHULL_HPP

#include "Kit/Transformable.hpp"
#include "Kit/Export.hpp"
#include "Kit/Types.hpp"

#include <memory>
#include <map>
#include <string>


/*

Check if point is inside a set of planes:

"I your destinations are convex then just check the point-plane distance of each polygon in the destination, and if the point is on the same side of all polygons then it's inside."

 - http://www.gamedev.net/topic/657395-determining-if-a-3d-point-is-in-a-3d-polygon/


 Get the collision normal:

"Since you can detect if the point is in your polygon, all you need to do find which face is the axis of minimum penetration.
This tells you what direction to press your two objects apart.
This direction is called the collision normal.

In order to know this you will be using the results of your plane tests.
When you do collision testing against your polygon you can compute the distance of the point to each face plane.
Keep track of the greatest signed value.
If the greatest signed distance is negative then you know the point lies within the polygon.
This greatest signed distance (which is still negative) comes from the plane which is the axis of minimum penetration.
This distance is your penetration depth."

 - http://gamedev.stackexchange.com/questions/67055/point-vs-convex-hull





  // Collision detection:
  def pointInside(p):
    pendepth = -999999;
    cplane = None

    for plane in m_planes:
      d = plane.getPointDistance(p)
      if d >= 0:
        return false

      if d > pendepth:
        maxdist = d
        cplane = plane

    return [true, cplane]

*/

/*
//Calculate a vector from the point on the plane to our test point
D3DXVECTOR3 vecTemp(vecTestPoint - vecPointOnPlane);

//Calculate the distance: dot product of the new vector with the plane's normal
float fDist(D3DXVec3Dot(&vecTemp, &vecNormal));

if(fDist > EPSILON)
{
//Point is in front of the plane
return 0;
}

*/

namespace kit 
{ 
  struct KITAPI Plane
  {
    glm::vec3 point;
    glm::vec3 normal;

    bool inFront(glm::vec3 p);
  };

  class KITAPI ConvexHull : public kit::Transformable
  {
  public:


    typedef std::shared_ptr<ConvexHull> Ptr;
    static Ptr load(std::string filename);

    bool overlaps(Ptr hull);
    bool overlaps(glm::vec3 point);

    ConvexHull();
    ~ConvexHull();

    std::vector<glm::vec3> & getLocalPoints();
    std::vector<glm::vec3> getWorldPoints();

    std::vector<Plane> & getLocalPlanes();
    std::vector<Plane> getWorldPlanes();
    
  private:
    std::vector<glm::vec3> m_points;
    std::vector<Plane> m_planes;
  };
}

#endif