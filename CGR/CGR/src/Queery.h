#ifndef __QUEERY_H__
#define __QUEERY_H__

#include "gl_headers.h"
#include "types.h"

class Query final
{
public:
	Query();
	Query(Query&& move);
	Query& operator=(Query&& move);
	~Query();

	bool IsInit() const;
	GLuint GetId() const;

	bool Init(const GLenum target);
	void Clean();
	void Start() const;
	void End() const;
	GLuint Result(const bool flushGPU) const;

private:
	GLuint m_Query;
	GLenum m_Target;
};

INLINE bool Query::IsInit() const
{
	return m_Query != 0U;
}

INLINE GLuint Query::GetId() const
{
	return m_Query;
}

#endif