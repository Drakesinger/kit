#include "Kit/GLTimer.hpp"

#include "Kit/IncOpenGL.hpp"

kit::GLTimer::GLTimer()
{
  
  glGenQueries(1, &m_glHandle);
}

kit::GLTimer::~GLTimer()
{
  
  glDeleteQueries(1, &m_glHandle);
  m_glHandle = 0;
}

void kit::GLTimer::start()
{
  
  glBeginQuery(GL_TIME_ELAPSED, m_glHandle);
}

uint64_t kit::GLTimer::end()
{
  
  glEndQuery(GL_TIME_ELAPSED);
  uint64_t timenano = 0;
  glGetQueryObjectui64v(m_glHandle, GL_QUERY_RESULT, &timenano);
  return timenano;// / 1,000,000.0;
}
