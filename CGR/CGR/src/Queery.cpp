#include "Queery.h"

Query::Query()
{
}

Query::Query(Query&& move)
{
	*this = std::move(move);
}

Query& Query::operator=(Query&& move)
{
	if (this != &move)
	{
		Clean();

		m_Query = move.m_Query;
		m_Target = move.m_Target;
		move.m_Query = 0U;
		move.m_Target = 0U;
	}

	return *this;
}

Query::~Query()
{
	Clean();
}

bool Query::Init(const GLenum target)
{
	GLuint query;
	glCreateQueries(target, 1, &query);

	if (query == 0U)
	{
		return false;
	}

	Clean();
	m_Query = query;
	m_Target = target;
	return true;
}

void Query::Clean()
{
	if (IsInit())
	{
		glDeleteQueries(1, &m_Query);
		m_Target = 0U;
	}
}

void Query::Start() const
{
	glBeginQuery(m_Target, m_Query);
}

void Query::End() const
{
	glEndQuery(m_Target);
}

GLuint Query::Result(const bool flushGPU) const
{
	int done = 0;
	while (!done)
	{
		glGetQueryObjectiv(m_Query, GL_QUERY_RESULT_AVAILABLE, &done);
	}

	const int param = flushGPU ? GL_QUERY_RESULT : GL_QUERY_RESULT_NO_WAIT;
	GLuint result{ 0 };
	glGetQueryObjectuiv(m_Query, param, &result);
	return result;
}