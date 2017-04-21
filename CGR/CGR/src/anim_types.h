#ifndef __ANIM_TYPES_H__
#define __ANIM_TYPES_H__

#include "types.h"
#include <vector>

typedef double double3[3];

// Num pre-calculated normals
#define NUM_VERT_NORMALS		162

// precalculated normal vectors
#define SHADEDOT_QUANT			16

// magic number "IDP2" or 844121161
#define MD2_IDENT				(('2'<<24) + ('P'<<16) + ('D'<<8) + 'I')

// model version
#define	MD2_VERSION				8

// maximum number of vertices for a MD2 model
#define MAX_MD2_VERTS			2048

// md2 header
typedef struct
{
	int     ident;              // magic number. must be equal to "IDP2"
	int     version;            // md2 version. must be equal to 8

	int     skinwidth;          // width of the texture
	int     skinheight;         // height of the texture
	int     framesize;          // size of one frame in bytes

	int     num_skins;          // number of textures
	int     num_xyz;            // number of vertices
	int     num_st;             // number of texture coordinates
	int     num_tris;           // number of triangles
	int     num_glcmds;         // number of opengl commands
	int     num_frames;         // total number of frames

	int     ofs_skins;          // offset to skin names (64 bytes each)
	int     ofs_st;             // offset to s-t texture coordinates
	int     ofs_tris;           // offset to triangles
	int     ofs_frames;         // offset to frame data
	int     ofs_glcmds;         // offset to opengl commands
	int     ofs_end;            // offset to end of file

} md2_t;

// md2 compressed vertex
typedef struct
{
	unsigned char   v[3];                // compressed vertex (x, y, z) coordinates
	unsigned char   lightnormalindex;    // index to a normal vector for the lighting

} vertex_t;

// md2 frame data (vertices are stored here)
typedef struct
{
	float       scale[3];       // scale values
	float       translate[3];   // translation vector
	char        name[16];       // frame name
	vertex_t    verts[1];       // first vertex of this frame

} frame_t;

// animation data - first and last frame and FPS to know how fast should it run
typedef struct
{
	int		first_frame;			// first frame of the animation
	int		last_frame;				// number of frames
	int		fps;					// number of frames per second

} anim_t;

// status animation
typedef struct
{
	int		startframe;				// first frame
	int		endframe;				// last frame
	int		fps;					// frame per second for this animation

	float	curr_time;				// current time
	float	old_time;				// old time
	float	interpol;				// percent of interpolation

	int		type;					// animation type

	int		curr_frame;				// current frame
	int		next_frame;				// next frame

} animState_t;

// animation list
typedef enum {
	STAND,
	RUN,
	ATTACK,
	PAIN_A,
	PAIN_B,
	PAIN_C,
	JUMP,
	FLIP,
	SALUTE,
	FALLBACK,
	WAVE,
	POINTING, // Had to rename this from POINT to POINTING because of POINT class in windows.h
	CROUCH_STAND,
	CROUCH_WALK,
	CROUCH_ATTACK,
	CROUCH_PAIN,
	CROUCH_DEATH,
	DEATH_FALLBACK,
	DEATH_FALLFORWARD,
	DEATH_FALLBACKSLOW,
	BOOM,

	MAX_ANIMATIONS

} animType_t;

struct AnimVert
{
	Vec3 pos;
	Vec3 norm;
};

struct AnimData
{
	uint32 vbo;
	std::vector<AnimVert> buffer;
	Vec3 min, max, centre;
};

#endif