#pragma once
class ScopedIndent
{
public:
	ScopedIndent();
	ScopedIndent(int indent);
	~ScopedIndent();

private:
	int m_indent;
};

