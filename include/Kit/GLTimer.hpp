#ifndef KIT_GLTIMER_HPP
#define KIT_GLTIMER_HPP

#include "Kit/Export.hpp"

#include <memory>

namespace kit 
{
  class KITAPI GLTimer
  {
  public:
    typedef std::shared_ptr<GLTimer> Ptr;
    
    static Ptr create();
    
    GLTimer();
    ~GLTimer();
    
    void start();
    uint64_t end(); // Returns nanoseconds taken since start
    
  private:
    uint32_t m_glHandle;
  };
}

#endif