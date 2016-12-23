#include "Kit/Text.hpp"

#include "Kit/IncOpenGL.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Font.hpp"
#include "Kit/Program.hpp"
#include "Kit/Shader.hpp"

#include <sstream>

kit::Program * kit::Text::m_renderProgram = nullptr;
uint32_t kit::Text::m_instanceCount = 0;

kit::Text::Text(kit::Font * font, float fontsize, std::wstring text, glm::vec2 position)
{
  glGenVertexArrays(1, &m_glVertexArray);
  glGenBuffers(1, &m_glVertexIndices);
  glGenBuffers(1, &m_glVertexBuffer);

  m_indexCount = 0;
  m_width = 0.0;
  m_hAlignment = Left;
  m_vAlignment = Bottom;
  
  kit::Text::m_instanceCount++;
  if(kit::Text::m_instanceCount == 1)
  {
    kit::Text::allocateShared();
  }
  
  m_font = font;

  m_fontSize = fontsize;
  m_text = text;
  m_position = position;
  m_color = (glm::vec4(1.0, 1.0, 1.0, 1.0));
  
  updateBuffers();
}

void kit::Text::renderShadowed(glm::ivec2 resolution, glm::vec2 shadowOffset, glm::vec4 shadowColor)
{
  glm::vec4 c = m_color;
  glm::vec2 p = m_position;

  setPosition(p + shadowOffset);
  setColor(shadowColor);
  render(resolution);

  setPosition(p);
  setColor(c);
  render(resolution);
}

void kit::Text::render(glm::ivec2 resolution)
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  kit::Text::m_renderProgram->use();
  kit::Text::m_renderProgram->setUniform2f("uniform_resolution", glm::vec2(float(resolution.x), float(resolution.y)));
  
  glm::vec2 pos = m_position;
  
  if(m_hAlignment == Right)
  {
    pos.x -= m_width;
  }
  else if(m_hAlignment == Centered)
  {
    pos.x -=m_width / 2.0f;
  }
  
  if(m_vAlignment == Middle)
  {
    pos.y += m_font->getGlyphMap(m_fontSize)->getHeight() / 2.0f;
  }
  else if(m_vAlignment == Top)
  {
    pos.y += m_font->getGlyphMap(m_fontSize)->getHeight();
  }

  pos.x = glm::ceil(pos.x);
  pos.y = glm::ceil(pos.y);
  
  kit::Text::m_renderProgram->setUniform2f("uniform_position", pos);
  kit::Text::m_renderProgram->setUniform4f("uniform_color", kit::srgbDec(m_color));
  kit::Text::m_renderProgram->setUniformTexture("uniform_glyphmap", m_font->getGlyphMap(m_fontSize)->getTexture());
  
  glBindVertexArray(m_glVertexArray);
  glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, (void*)0);
  kit::Program::useFixed();
}

void kit::Text::setText(std::wstring text)
{
  if (text == m_text)
  {
    return;
  }
  m_text = text;
  updateBuffers();
}

std::wstring const & kit::Text::getText()
{
  return m_text;
}

void kit::Text::setFont(kit::Font * font)
{
  m_font = font;
  updateBuffers();
}

kit::Font * kit::Text::getFont()
{
  return m_font;
}

void kit::Text::setFontSize(float fontsize)
{
  m_fontSize = fontsize;
  updateBuffers();
}

float const & kit::Text::getFontSize()
{
  return m_fontSize;
}

void kit::Text::setPosition(glm::vec2 position)
{
  m_position = position;
}

glm::vec2 const & kit::Text::getPosition()
{
  return m_position;
}

kit::Text::~Text()
{
  
  glDeleteBuffers(1, &m_glVertexIndices);
  glDeleteBuffers(1, &m_glVertexBuffer);
  glDeleteVertexArrays(1, &m_glVertexArray);
  
  kit::Text::m_instanceCount--;
  if(kit::Text::m_instanceCount == 0)
  {
    kit::Text::releaseShared();
  }
}

void kit::Text::allocateShared()
{
  std::stringstream vss, pss;
  
  vss << "#version 410 core" << std::endl;
  vss << "layout(location = 0) in vec2 in_position;" << std::endl;
  vss << "layout(location = 1) in vec2 in_uv;" << std::endl;
  vss << "layout(location = 0) out vec2 out_uv;" << std::endl;
  vss << "uniform vec2 uniform_resolution;" << std::endl;
  vss << "uniform vec2 uniform_position;" << std::endl;
  vss << "void main()" << std::endl;
  vss << "{" << std::endl;
  vss << "  gl_Position.xy = (uniform_position + in_position)/uniform_resolution;" << std::endl;
  vss << "  gl_Position.xy = gl_Position.xy * 2.0 - 1.0;" << std::endl;
  vss << "  gl_Position.y = -gl_Position.y;" << std::endl;
  vss << "  gl_Position.z = 0.0;" << std::endl;
  vss << "  gl_Position.w = 1.0;" << std::endl;
  vss << "  out_uv = in_uv;" << std::endl;
  vss << "}" << std::endl;
  
  pss << "#version 410 core" << std::endl;
  pss << "layout(location = 0) in vec2 in_uv;" << std::endl;
  pss << "layout(location = 0) out vec4 out_color;" << std::endl;
  pss << "uniform sampler2D uniform_glyphmap;" << std::endl;
  pss << "uniform vec4 uniform_color;" << std::endl;
  pss << "void main()" << std::endl;
  pss << "{" << std::endl;
  pss << " out_color.a = texture(uniform_glyphmap, in_uv).r * uniform_color.a;" << std::endl;
  pss << " out_color.rgb = uniform_color.rgb;" << std::endl;
  pss << "}" << std::endl;
  
  auto vs = new kit::Shader(Shader::Type::Vertex);
  vs->sourceFromString(vss.str());
  vs->compile();
  
  auto ps = new kit::Shader(Shader::Type::Fragment);
  ps->sourceFromString(pss.str());
  ps->compile();
  
  m_renderProgram = new kit::Program();
  m_renderProgram->attachShader(vs);
  m_renderProgram->attachShader(ps);
  m_renderProgram->link();
  m_renderProgram->detachShader(ps);
  m_renderProgram->detachShader(vs);
  
  delete vs;
  delete ps;
}

void kit::Text::releaseShared()
{
  delete m_renderProgram;
}

void kit::Text::updateBuffers()
{
  
  if (m_text.size() == 0)
  {
    m_width = 0;
    m_indexCount = 0;
    return;
  }
  uint32_t numIndices = uint32_t(m_text.size() * 6);
  uint32_t numVertices = uint32_t(m_text.size() * 4) * 4;
  
  std::vector<float> vertices(numVertices , 0.0);
  std::vector<uint32_t> indices(numIndices, 0);

  uint32_t vi = 0;
  glm::vec2 pen(0.0, 0.0);
  float maxWidth = 0.0f;
  // Create vertices
  for(wchar_t const & currChar : m_text)
  {
    if(currChar == wchar_t('\n'))
    {
      if (pen.x > maxWidth)
      {
        maxWidth = pen.x;
      }

      pen.x = 0.0;
      pen.y += getLineAdvance();
      continue;
    }
    
    kit::Font::Glyph const & currGlyph = m_font->getGlyphMap(m_fontSize)->getGlyph(currChar);
    
    glm::vec2 glyphPos, glyphSize;
    
    glyphPos.x = pen.x + currGlyph.m_placement.x;
    glyphPos.y = pen.y - currGlyph.m_placement.y;
    
    glyphSize.x = (float)currGlyph.m_size.x;
    glyphSize.y = (float)currGlyph.m_size.y;
    
    vertices[vi++] = glyphPos.x;
    vertices[vi++] = glyphPos.y;
    vertices[vi++] = currGlyph.m_uv.x;
    vertices[vi++] = currGlyph.m_uv.y;
    
    vertices[vi++] = glyphPos.x + glyphSize.x;
    vertices[vi++] = glyphPos.y;
    vertices[vi++] = currGlyph.m_uv.z;
    vertices[vi++] = currGlyph.m_uv.y;
    
    vertices[vi++] = glyphPos.x + glyphSize.x;
    vertices[vi++] = glyphPos.y + glyphSize.y;
    vertices[vi++] = currGlyph.m_uv.z;
    vertices[vi++] = currGlyph.m_uv.w;
    
    vertices[vi++] = glyphPos.x;
    vertices[vi++] = glyphPos.y + glyphSize.y;
    vertices[vi++] = currGlyph.m_uv.x;
    vertices[vi++] = currGlyph.m_uv.w;
    
    pen.x += currGlyph.m_advance.x;
    pen.y += currGlyph.m_advance.y;
  }
  
  uint32_t ii = 0;
  for(uint32_t i = 0; i < m_text.size()*4; i+=4)
  {
    indices[ii++] = i;
    indices[ii++] = i+1;
    indices[ii++] = i+2;
    indices[ii++] = i;
    indices[ii++] = i+2;
    indices[ii++] = i+3;
  }
  
  m_indexCount = uint32_t(indices.size());

  glBindVertexArray(m_glVertexArray);

  // Upload indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glVertexIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

  // Upload vertices 
  glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

  // Total size
  uint32_t attributeSize = (sizeof(float)* 4);

  // Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)0); 
  
  // UV
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(float) * 2)); 
  
  glBindVertexArray(0);
  
  // Set the width to the current pen X
  if (pen.x > maxWidth)
  {
    maxWidth = pen.x;
  }
  m_width = maxWidth;
}

float kit::Text::getLineAdvance()
{
  return m_font->getGlyphMap(m_fontSize)->getLineAdvance();
}

float kit::Text::getHeight()
{
  return m_font->getGlyphMap(m_fontSize)->getHeight();
}

const glm::vec4& kit::Text::getColor()
{
  return m_color;
}

void kit::Text::setColor(glm::vec4 color)
{
  m_color = (color);
}

void kit::Text::setAlignment(kit::Text::HAlignment h, kit::Text::VAlignment v)
{
  m_hAlignment = h;
  m_vAlignment = v;
}

