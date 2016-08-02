#include "Kit/Console.hpp"

#include "Kit/Quad.hpp"
#include "Kit/Font.hpp"
#include "Kit/Text.hpp"
#include "Kit/Application.hpp"
#include "Kit/Window.hpp"

kit::Console::Console(kit::Application* app)
{
  this->m_application = app;
  this->m_heightCoeff = 0.0;
  this->m_isActive = false;
  this->m_lineOffset = 0;
  this->m_cursorPosition = 0;
  this->m_buffer.push_front(L"");
  this->m_bufferPosition = 0;
  
  this->m_quad = kit::Quad::create();
  this->m_quad->setColor(glm::vec4(0.005f, 0.02f, 0.05f, 0.9f));
  this->m_font = kit::Font::load("Inconsolata.otf");
  this->m_bufferText = kit::Text::create(this->m_font, 18.0f, L"");
  this->updateBufferText();
}

kit::Console::~Console()
{

}

kit::Console::Ptr kit::Console::create(kit::Application* app)
{
  return std::make_shared<kit::Console>(app);
}

void kit::Console::addLine(std::wstring s)
{
  kit::ConsoleLine newLine;
  newLine.string = s;
  newLine.text = kit::Text::create(this->m_font, 18.0f, s);
  this->m_lines.push_front(newLine);
  
  while(this->m_lines.size() > 127)
  {
    this->m_lines.pop_back();
  }
}

void kit::Console::hide()
{
  this->m_isActive = false;
}

void kit::Console::Show()
{
  this->m_isActive = true;
}

void kit::Console::render()
{
  glm::ivec2 winSize = this->m_application->getWindow()->getFramebufferSize();
  this->m_application->getWindow()->bind();

  if(this->m_heightCoeff > 0.0)
  {
    float effHeight =  -(1.0f-this->m_heightCoeff)*320.f;
    
    this->m_quad->setSize(glm::vec2(1.0f, 320.0f/float(winSize.y)));
    this->m_quad->setPosition(glm::vec2(0.0f, effHeight/float(winSize.y)));
    this->m_quad->render();
    
    this->m_bufferText->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    this->m_bufferText->setPosition(glm::vec2(3.0f, effHeight + 320.0f - 7.0f));
    this->m_bufferText->render(winSize);
    
    this->m_bufferText->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    this->m_bufferText->setPosition(glm::vec2(4.0f, effHeight + 320.0f - 8.0f));
    this->m_bufferText->render(winSize);
    
    if(this->m_lines.size() > 0)
    {
      auto it = this->m_lines.begin();
      
      for(uint32_t currLineOffset = 0; currLineOffset < this->m_lineOffset; currLineOffset++)
      {
        if(it != this->m_lines.end())
        {
          it++;
        }
      }
      
      float height = 320.0f - 32.0f;
      for(int count = 0; count < 9; count++)
      {
        if(it == this->m_lines.end())
        {
          break;
        }
        it->text->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        it->text->setPosition(glm::vec2(3.0f, effHeight + height - 7.0f));
        it->text->render(winSize);
        
        it->text->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        it->text->setPosition(glm::vec2(4.0f, effHeight + height - 8.0f));
        it->text->render(winSize);
        
        height -= 32.0f;
        it++;
      }
    }
  }
}

void kit::Console::update(const double& ms)
{
  if(this->m_isActive && this->m_heightCoeff < 1.0f)
  {
    this->m_heightCoeff += float(ms)*0.01f* (1.0f - this->m_heightCoeff);
    this->m_heightCoeff = (glm::min)(this->m_heightCoeff, 1.0f);
  }
  if(!this->m_isActive && this->m_heightCoeff > 0.0f)
  {
    this->m_heightCoeff -= float(ms)*0.01f*(glm::max)(1.0f - this->m_heightCoeff, 0.01f);
    this->m_heightCoeff = (glm::max)(this->m_heightCoeff, 0.0f);
  }
}

void kit::Console::handleEvent(const kit::WindowEvent& evt)
{
  if(evt.type == kit::WindowEvent::KeyPressed || evt.type == kit::WindowEvent::KeyRepeated)
  {
    if(this->m_isActive)
    {
      if(evt.keyboard.key == kit::Left)
      {
        if(this->m_cursorPosition > 0)
        {
          this->m_cursorPosition--;
          this->updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Right)
      {
        if(this->m_cursorPosition < this->m_buffer.begin()->size())
        {
          this->m_cursorPosition++;
          this->updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Backspace)
      {
        if(this->m_buffer.begin()->size() > 0)
        {
          //this->m_buffer.pop_back();
          if(this->m_cursorPosition > 0)
          {
            this->m_buffer.begin()->erase(this->m_buffer.begin()->begin() + this->m_cursorPosition-1);
            this->m_cursorPosition--;
            this->m_bufferSave.assign((*this->m_buffer.begin()));
            this->updateBufferText();
          }
        }
      }
      else if(evt.keyboard.key == kit::Delete)
      {
        if((*this->m_buffer.begin()).size() > 0)
        {
          if(this->m_cursorPosition < (*this->m_buffer.begin()).size())
          {
            this->m_buffer.begin()->erase(this->m_buffer.begin()->begin() + this->m_cursorPosition);
            
            if(this->m_cursorPosition > this->m_buffer.begin()->size())
            {
              this->m_cursorPosition = (uint32_t)this->m_buffer.begin()->size();
            }
            
            this->m_bufferSave.assign((*this->m_buffer.begin()));
            
            this->updateBufferText();
          }
        }
      }
      else if(evt.keyboard.key == kit::Enter)
      {
        if(this->m_buffer.begin()->size() > 0)
        {
          std::string buf = kit::wideToString((*this->m_buffer.begin()));
          this->addLine(std::wstring(L"> ") + (*this->m_buffer.begin()));
          this->m_buffer.push_front(L"");
          while(this->m_buffer.size() > 127)
          {
            this->m_buffer.pop_back();
          }
          this->m_bufferPosition = 0;
          this->m_cursorPosition = 0;
          this->updateBufferText();
          
          this->m_application->evaluate(buf);
        }
      }
      else if(evt.keyboard.key == kit::Home)
      {
        this->m_cursorPosition = 0;
        this->updateBufferText();
      }
      else if(evt.keyboard.key == kit::End)
      {
        this->m_cursorPosition = (uint32_t)this->m_buffer.begin()->size();
        this->updateBufferText();
      }
      else if(evt.keyboard.key == kit::Page_up)
      {
        if(this->m_lineOffset + 9 < this->m_lines.size())
        {
          this->m_lineOffset++;
        }
      }
      else if(evt.keyboard.key == kit::Page_down)
      {
        if(this->m_lineOffset > 0)
        {
          this->m_lineOffset--;
        }
      }
      else if(evt.keyboard.key == kit::Up)
      {
        if(this->m_bufferPosition + 1 < this->m_buffer.size())
        {
          this->m_bufferPosition++;
          if(this->m_bufferPosition == 1)
          {
            this->m_bufferSave.assign((*this->m_buffer.begin()));
          }
          if(this->m_bufferPosition != 0)
          {
            this->m_buffer.begin()->assign((*this->getCurrentBuffer()));
          }
          this->m_cursorPosition = (uint32_t)this->m_buffer.begin()->size();
          this->updateBufferText();
        }
      }
      else if(evt.keyboard.key == kit::Down)
      {
        if(this->m_bufferPosition > 0)
        {
          this->m_bufferPosition--;
          if(this->m_bufferPosition != 0)
          {
            this->m_buffer.begin()->assign((*this->getCurrentBuffer()));
          }
          else
          {
            this->m_buffer.begin()->assign(this->m_bufferSave);
          }
          this->m_cursorPosition = (uint32_t)this->m_buffer.begin()->size();
          this->updateBufferText();
        }
      }
    }
  }
  if(evt.type == kit::WindowEvent::TextEntered)
  {
    if(this->m_isActive)
    {
      //this->m_buffer.push_back((wchar_t)evt.keyboard.unicode);
      this->m_buffer.begin()->insert(this->m_buffer.begin()->begin() + this->m_cursorPosition, (wchar_t)evt.keyboard.unicode);
      this->m_bufferSave.assign((*this->m_buffer.begin()));
      this->m_cursorPosition++;
      this->updateBufferText();
    }
  }

}

bool kit::Console::isActive()
{
  return this->m_isActive;
}

void kit::Console::updateBufferText()
{
  std::wstring buftext = (*this->m_buffer.begin());
  buftext.insert(this->m_cursorPosition, L"_");
  this->m_bufferText->setText(std::wstring(L"> ") +buftext);
}

std::list<std::wstring>::iterator kit::Console::getCurrentBuffer()
{
  std::list<std::wstring>::iterator returner = this->m_buffer.begin();
  
  if(this->m_buffer.size() == 1)
  {
    return returner;
  }
  
  for(unsigned int i = 0; i < this->m_bufferPosition; i++)
  {
    returner++;
    if(returner == this->m_buffer.end())
    {
      returner--;
      break;
    }
  }
  
  return returner;
}
