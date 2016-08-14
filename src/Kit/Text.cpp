#include "Kit/Text.hpp"
#include "Kit/Texture.hpp"
#include "Kit/Font.hpp"
#include "Kit/Program.hpp"
#include "Kit/Shader.hpp"

#include <sstream>

kit::Program::Ptr kit::Text::m_renderProgram = nullptr;
uint32_t kit::Text::m_instanceCount = 0;

kit::Text::Ptr kit::Text::create(kit::Font::Ptr font, float fontsize, std::wstring text, glm::vec2 position)
{
  kit::Text::Ptr returner = std::make_shared<kit::Text>();
  returner->m_font = font;
  returner->m_fontSize = fontsize;
  returner->m_text = text;
  returner->m_position = position;
  returner->m_color = (glm::vec4(1.0, 1.0, 1.0, 1.0));
  returner->updateBuffers();
  return returner;
}

void kit::Text::renderShadowed(glm::ivec2 resolution, glm::vec2 shadowOffset, glm::vec4 shadowColor)
{
  glm::vec4 c = this->m_color;
  glm::vec2 p = this->m_position;

  this->setPosition(p + shadowOffset);
  this->setColor(shadowColor);
  this->render(resolution);

  this->setPosition(p);
  this->setColor(c);
  this->render(resolution);
}

void kit::Text::render(glm::ivec2 resolution)
{
  kit::GL::disable(GL_DEPTH_TEST);
  kit::GL::disable(GL_CULL_FACE);
  kit::GL::depthMask(GL_FALSE);
  kit::GL::enable(GL_BLEND);
  kit::GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  kit::Text::m_renderProgram->use();
  kit::Text::m_renderProgram->setUniform2f("uniform_resolution", glm::vec2(float(resolution.x), float(resolution.y)));
  
  glm::vec2 pos = this->m_position;
  
  if(this->m_hAlignment == Right)
  {
    pos.x -= this->m_width;
  }
  else if(this->m_hAlignment == Centered)
  {
    pos.x -=this->m_width / 2.0f;
  }
  
  if(this->m_vAlignment == Middle)
  {
    pos.y += this->m_font->getGlyphMap(this->m_fontSize)->getHeight() / 2.0f;
  }
  else if(this->m_vAlignment == Top)
  {
    pos.y += this->m_font->getGlyphMap(this->m_fontSize)->getHeight();
  }

  pos.x = glm::ceil(pos.x);
  pos.y = glm::ceil(pos.y);
  
  kit::Text::m_renderProgram->setUniform2f("uniform_position", pos);
  kit::Text::m_renderProgram->setUniform4f("uniform_color", kit::srgbDec(this->m_color));
  kit::Text::m_renderProgram->setUniformTexture("uniform_glyphmap", this->m_font->getGlyphMap(this->m_fontSize)->getTexture());
  
  kit::GL::bindVertexArray(this->m_glVertexArray);
  KIT_GL(glDrawElements(GL_TRIANGLES, this->m_indexCount, GL_UNSIGNED_INT, (void*)0));
  kit::Program::useFixed();
}

void kit::Text::setText(std::wstring text)
{
  if (text == this->m_text)
  {
    return;
  }
  this->m_text = text;
  this->updateBuffers();
}

std::wstring const & kit::Text::getText()
{
  return this->m_text;
}

void kit::Text::setFont(kit::Font::Ptr font)
{
  this->m_font = font;
  this->updateBuffers();
}

kit::Font::Ptr kit::Text::getFont()
{
  return this->m_font;
}

void kit::Text::setFontSize(float fontsize)
{
  this->m_fontSize = fontsize;
  this->updateBuffers();
}

float const & kit::Text::getFontSize()
{
  return this->m_fontSize;
}

void kit::Text::setPosition(glm::vec2 position)
{
  this->m_position = position;
}

glm::vec2 const & kit::Text::getPosition()
{
  return this->m_position;
}

kit::Text::Text()
{
  
  KIT_GL(glGenVertexArrays(1, &this->m_glVertexArray));
  KIT_GL(glGenBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glGenBuffers(1, &this->m_glVertexBuffer));

  this->m_indexCount = 0;
  this->m_width = 0.0;
  this->m_hAlignment = Left;
  this->m_vAlignment = Bottom;
  
  kit::Text::m_instanceCount++;
  if(kit::Text::m_instanceCount == 1)
  {
    kit::Text::allocateShared();
  }
}

kit::Text::~Text()
{
  
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexIndices));
  KIT_GL(glDeleteBuffers(1, &this->m_glVertexBuffer));
  KIT_GL(glDeleteVertexArrays(1, &this->m_glVertexArray));
  
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
  
  auto vs = kit::Shader::create(Shader::Type::Vertex);
  vs->sourceFromString(vss.str());
  vs->compile();
  
  auto ps = kit::Shader::create(Shader::Type::Fragment);
  ps->sourceFromString(pss.str());
  ps->compile();
  
  kit::Text::m_renderProgram = kit::Program::create();
  kit::Text::m_renderProgram->attachShader(vs);
  kit::Text::m_renderProgram->attachShader(ps);
  kit::Text::m_renderProgram->link();
  kit::Text::m_renderProgram->detachShader(ps);
  kit::Text::m_renderProgram->detachShader(vs);
}

void kit::Text::releaseShared()
{

}

void kit::Text::updateBuffers()
{
  
  if (this->m_text.size() == 0)
  {
    this->m_width = 0;
    this->m_indexCount = 0;
    return;
  }
  uint32_t numIndices = uint32_t(this->m_text.size() * 6);
  uint32_t numVertices = uint32_t(this->m_text.size() * 4) * 4;
  
  std::vector<GLfloat> vertices(numVertices , 0.0);
  std::vector<GLuint> indices(numIndices, 0);

  uint32_t vi = 0;
  glm::vec2 pen(0.0, 0.0);
  float maxWidth = 0.0f;
  // Create vertices
  for(wchar_t const & currChar : this->m_text)
  {
    if(currChar == wchar_t('\n'))
    {
      if (pen.x > maxWidth)
      {
        maxWidth = pen.x;
      }

      pen.x = 0.0;
      pen.y += this->getLineAdvance();
      continue;
    }
    
    kit::Font::Glyph const & currGlyph = this->m_font->getGlyphMap(this->m_fontSize)->getGlyph(currChar);
    
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
  for(uint32_t i = 0; i < this->m_text.size()*4; i+=4)
  {
    indices[ii++] = i;
    indices[ii++] = i+1;
    indices[ii++] = i+2;
    indices[ii++] = i;
    indices[ii++] = i+2;
    indices[ii++] = i+3;
  }
  
  this->m_indexCount = uint32_t(indices.size());

  kit::GL::bindVertexArray(this->m_glVertexArray);

  // Upload indices
  kit::GL::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glVertexIndices);
  KIT_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW));

  // Upload vertices 
  kit::GL::bindBuffer(GL_ARRAY_BUFFER, this->m_glVertexBuffer);
  KIT_GL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW));

  // Total size
  uint32_t attributeSize = (sizeof(GLfloat)* 4);

  // Positions
  KIT_GL(glEnableVertexAttribArray(0));
  KIT_GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*)0)); 
  
  // UV
  KIT_GL(glEnableVertexAttribArray(1));
  KIT_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributeSize, (void*) (sizeof(GLfloat) * 2))); 
  
  kit::GL::bindVertexArray(0);
  
  // Set the width to the current pen X
  if (pen.x > maxWidth)
  {
    maxWidth = pen.x;
  }
  this->m_width = maxWidth;
}

float kit::Text::getLineAdvance()
{
  return this->m_font->getGlyphMap(this->m_fontSize)->getLineAdvance();
}

float kit::Text::getHeight()
{
  return this->m_font->getGlyphMap(this->m_fontSize)->getHeight();
}

const glm::vec4& kit::Text::getColor()
{
  return this->m_color;
}

void kit::Text::setColor(glm::vec4 color)
{
  this->m_color = (color);
}

void kit::Text::setAlignment(kit::Text::HAlignment h, kit::Text::VAlignment v)
{
  this->m_hAlignment = h;
  this->m_vAlignment = v;
}

