#include "EditorComponent.hpp"

kwe::EditorComponent::EditorComponent()
{

}

kwe::EditorComponent::~EditorComponent()
{

}

void kwe::EditorComponent::allocate(kwe::WorldEditorState* editorRef)
{
  this->m_editorReference = editorRef;
}
