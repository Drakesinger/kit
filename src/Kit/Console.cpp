#include "Kit/Console.hpp"

#include "Kit/Quad.hpp"
#include "Kit/Font.hpp"
#include "Kit/Text.hpp"
#include "Kit/Application.hpp"
#include "Kit/Window.hpp"

kit::ConsoleLine::ConsoleLine(const std::wstring& s)
{
  string = s;
  text = new kit::Text(kit::Font::getSystemFont(), 18.0f, s);
}

kit::ConsoleLine::~ConsoleLine()
{
}

kit::Console::Console(kit::Application* app)
{
  m_application = app;
  m_heightCoeff = 0.0;
  m_isActive = false;
  m_lineOffset = 0;
  m_cursorPosition = 0;
  m_buffer.push_front(L"");
  m_bufferPosition = 0;
  
  m_quad = new kit::Quad();
  m_quad->setColor(glm::vec4(0.005f, 0.02f, 0.05f, 0.9f));
  m_bufferText = new kit::Text(kit::Font::getSystemFont(), 18.0f, L"");
  updateBufferText();
}

kit::Console::~Console()
{
  delete m_quad;
  delete m_bufferText;
  for(auto & c : m_lines)
    delete c.text;

  m_lines.clear();
}

void kit::Console::addLine(std::wstring s)
{
  m_lines.push_front(kit::ConsoleLine(s));
  
  std::wcout << L"Added console line " << s << std::endl;
  
  while(m_lines.size() > 127)
  {
    m_lines.pop_back();
  }
}

void kit::Console::hide()
{
  m_isActive = false;
}

void kit::Console::Show()
{
  m_isActive = true;
}

void kit::Console::render()
{

  if(m_heightCoeff > 0.0)
  {
    glm::ivec2 winSize = m_application->getWindow()->getFramebufferSize();
    m_application->getWindow()->bind();
    
    float effHeight =  -(1.0f-m_heightCoeff)*320.f;
    
    m_quad->setSize(glm::vec2(1.0f, 320.0f/float(winSize.y)));
    m_quad->setPosition(glm::vec2(0.0f, effHeight/float(winSize.y)));
    m_quad->render();
    
    m_bufferText->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_bufferText->setPosition(glm::vec2(3.0f, effHeight + 320.0f - 7.0f));
    m_bufferText->render(winSize);
    
    m_bufferText->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_bufferText->setPosition(glm::vec2(4.0f, effHeight + 320.0f - 8.0f));
    m_bufferText->render(winSize);
    
    if(m_lines.size() > 0)
    {
      size_t i = m_lineOffset;
      
      float height = 320.0f - 32.0f;
      for(int count = 0; count < 9; count++)
      {
        if(i >= m_lines.size())
        {
          break;
        }
        
        auto & c = m_lines[i];
        
        c.text->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        c.text->setPosition(glm::vec2(3.0f, effHeight + height - 7.0f));
        c.text->render(winSize);
        
        c.text->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        c.text->setPosition(glm::vec2(4.0f, effHeight + height - 8.0f));
        c.text->render(winSize);
        
        height -= 32.0f;
        i++;
      }
    }
  }
}

void kit::Console::update(const double& ms)
{
  if(m_isActive && m_heightCoeff < 1.0f)
  {
    m_heightCoeff += float(ms)*0.01f* (1.0f - m_heightCoeff);
    m_heightCoeff = (glm::min)(m_heightCoeff, 1.0f);
  }
  if(!m_isActive && m_heightCoeff > 0.0f)
  {
    m_heightCoeff -= float(ms)*0.01f*(glm::max)(1.0f - m_heightCoeff, 0.01f);
    m_heightCoeff = (glm::max)(m_heightCoeff, 0.0f);
  }
}

void kit::Console::handleEvent(const kit::WindowEvent& evt)
{
  if(evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyRepeated)
  {
    if(m_isActive)
    {
      if(evt.keyboard.key == kit::Left)
      {
        if(m_cursorPosition > 0)
        {
          m_cursorPosition--;
          updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Right)
      {
        if(m_cursorPosition < m_buffer.begin()->size())
        {
          m_cursorPosition++;
          updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Backspace)
      {
        if(m_buffer.begin()->size() > 0)
        {
          //m_buffer.pop_back();
          if(m_cursorPosition > 0)
          {
            m_buffer.begin()->erase(m_buffer.begin()->begin() + m_cursorPosition-1);
            m_cursorPosition--;
            m_bufferSave.assign((*m_buffer.begin()));
            updateBufferText();
          }
        }
      }
      else if(evt.keyboard.key == kit::Delete)
      {
        if((*m_buffer.begin()).size() > 0)
        {
          if(m_cursorPosition < (*m_buffer.begin()).size())
          {
            m_buffer.begin()->erase(m_buffer.begin()->begin() + m_cursorPosition);
            
            if(m_cursorPosition > m_buffer.begin()->size())
            {
              m_cursorPosition = (uint32_t)m_buffer.begin()->size();
            }
            
            m_bufferSave.assign((*m_buffer.begin()));
            
            updateBufferText();
          }
        }
      }
      else if(evt.keyboard.key == kit::Enter)
      {
        if(m_buffer.begin()->size() > 0)
        {
          std::string buf = kit::wideToString((*m_buffer.begin()));
          addLine(std::wstring(L"> ") + (*m_buffer.begin()));
          m_buffer.push_front(L"");
          while(m_buffer.size() > 127)
          {
            m_buffer.pop_back();
          }
          m_bufferPosition = 0;
          m_cursorPosition = 0;
          updateBufferText();
          
          m_application->evaluate(buf);
        }
      }
      else if(evt.keyboard.key == kit::Home)
      {
        m_cursorPosition = 0;
        updateBufferText();
      }
      else if(evt.keyboard.key == kit::End)
      {
        m_cursorPosition = (uint32_t)m_buffer.begin()->size();
        updateBufferText();
      }
      else if(evt.keyboard.key == kit::Page_up)
      {
        if(m_lineOffset + 9 < m_lines.size())
        {
          m_lineOffset++;
        }
      }
      else if(evt.keyboard.key == kit::Page_down)
      {
        if(m_lineOffset > 0)
        {
          m_lineOffset--;
        }
      }
      else if(evt.keyboard.key == kit::Up)
      {
        if(m_bufferPosition + 1 < m_buffer.size())
        {
          m_bufferPosition++;
          if(m_bufferPosition == 1)
          {
            m_bufferSave.assign((*m_buffer.begin()));
          }
          if(m_bufferPosition != 0)
          {
            m_buffer.begin()->assign((*getCurrentBuffer()));
          }
          m_cursorPosition = (uint32_t)m_buffer.begin()->size();
          updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Down)
      {
        if(m_bufferPosition > 0)
        {
          m_bufferPosition--;
          if(m_bufferPosition != 0)
          {
            m_buffer.begin()->assign((*getCurrentBuffer()));
          }
          else
          {
            m_buffer.begin()->assign(m_bufferSave);
          }
          m_cursorPosition = (uint32_t)m_buffer.begin()->size();
          updateBufferText();
        }
      }
    }
  }
  if(evt.type == kit::WindowEvent::TextEntered)
  {
    if(m_isActive)
    {
      //m_buffer.push_back((wchar_t)evt.keyboard.unicode);
      m_buffer.begin()->insert(m_buffer.begin()->begin() + m_cursorPosition, (wchar_t)evt.keyboard.unicode);
      m_bufferSave.assign((*m_buffer.begin()));
      m_cursorPosition++;
      updateBufferText();
    }
  }

}

bool kit::Console::isActive()
{
  return m_isActive;
}

void kit::Console::updateBufferText()
{
  std::wstring buftext = (*m_buffer.begin());
  buftext.insert(m_cursorPosition, L"_");
  m_bufferText->setText(std::wstring(L"> ") +buftext);
}

std::list<std::wstring>::iterator kit::Console::getCurrentBuffer()
{
  std::list<std::wstring>::iterator returner = m_buffer.begin();
  
  if(m_buffer.size() == 1)
  {
    return returner;
  }
  
  for(unsigned int i = 0; i < m_bufferPosition; i++)
  {
    returner++;
    if(returner == m_buffer.end())
    {
      returner--;
      break;
    }
  }
  
  return returner;
}
