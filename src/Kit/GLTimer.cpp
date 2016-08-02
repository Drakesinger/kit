#include "Kit/GLTimer.hpp"

kit::GLTimer::GLTimer()
{
  
  KIT_GL(glGenQueries(1, &this->m_glHandle));
}

kit::GLTimer::~GLTimer()
{
  
  KIT_GL(glDeleteQueries(1, &this->m_glHandle));
  this->m_glHandle = 0;
}

kit::GLTimer::Ptr kit::GLTimer::create()
{
  return std::make_shared<kit::GLTimer>();
}

void kit::GLTimer::start()
{
  
  KIT_GL(glBeginQuery(GL_TIME_ELAPSED, this->m_glHandle));
}

uint64_t kit::GLTimer::end()
{
  
  KIT_GL(glEndQuery(GL_TIME_ELAPSED));
  GLuint64 timenano = 0;
  KIT_GL(glGetQueryObjectui64v(this->m_glHandle, GL_QUERY_RESULT, &timenano));
  return timenano;// / 1,000,000.0;
}
