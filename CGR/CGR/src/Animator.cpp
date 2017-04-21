#include "Animator.h"
#include "Time.h"
#include "AnimMesh.h"

int Animator::m_Id = ANIM_COMPONENT;

Animator::Animator(GameObject* go) :
	Component(go)
{
}

Animator::~Animator()
{
}

void Animator::Start()
{
}

void Animator::Update()
{
	m_AnimState.curr_time += Time::DeltaTime();

	if (m_AnimState.curr_time - m_AnimState.old_time > (1.0f / float(m_AnimState.fps)))
	{
		m_AnimState.old_time = m_AnimState.curr_time;

		m_AnimState.curr_frame = m_AnimState.next_frame;
		m_AnimState.next_frame++;
		if (m_AnimState.next_frame > m_AnimState.endframe)
			m_AnimState.next_frame = m_AnimState.startframe;
	}

	m_AnimState.interpol = float(m_AnimState.fps) * (m_AnimState.curr_time - m_AnimState.old_time);
}

void Animator::StartAnimation(animType_t type)
{
	animState_t res;
	res.startframe = AnimMesh::animlist[type].first_frame;
	res.endframe =   AnimMesh::animlist[type].last_frame;
	res.curr_frame = AnimMesh::animlist[type].first_frame;
	res.next_frame = AnimMesh::animlist[type].first_frame + 1;

	res.fps = AnimMesh::animlist[type].fps;
	res.type = type;

	res.curr_time = 0.0f;
	res.old_time = 0.0f;

	res.interpol = 0.0f;

	m_AnimState = res;
}