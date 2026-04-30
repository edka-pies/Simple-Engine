#include "UITools.h"
#include "EditorCore.h"

static constexpr int defaultIndent = 4;

ScopedIndent::ScopedIndent() : m_indent(defaultIndent)
{
	ImGui::Indent(m_indent);
}

ScopedIndent::ScopedIndent(int indent) : m_indent(indent)
{
	ImGui::Indent(m_indent);
}

ScopedIndent::~ScopedIndent()
{
	ImGui::Indent(-m_indent);
}