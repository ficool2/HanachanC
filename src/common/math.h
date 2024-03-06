#pragma once

#include <math.h>
#include <float.h>

#ifndef M_PI
	#define M_PI   3.14159265358979323846264338327950288
	#define M_PI_F 3.14159265358979323846264338327950288f
#endif

#define WII_MATH

enum
{
	axis_x,
	axis_y,
	axis_z,
};

typedef struct
{
	union
	{
		struct
		{
			float x, y;
		};

		float v[2];
	};
} vec2_t;

typedef struct
{
	union
	{
		struct
		{
			float x, y, z;
		};

		float v[3];
	};
} vec3_t;

typedef struct
{
	union
	{
		struct
		{
			float x, y, z, w;
		};

		float v[4];
	};
} quat_t;

typedef struct
{
	union
	{
		struct
		{
			float m00, m01, m02;
			float m10, m11, m12;
			float m20, m21, m22;
		};

		float m[3 * 3];
	};
} mat33_t;

typedef struct 
{
	union
	{
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
		};

		float m[3 * 4];
	};
} mat34_t;

typedef struct 
{
	union
	{
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};

		float m[4 * 4];
	};
} mat44_t;

float clampf(float x, float min, float max);
float fracf(float x);
float degreesf(float x);
float radiansf(float x);
float anglenormf(float angle);
void  float_print_hex(const float f);

#ifdef WII_MATH
float wii_fminf(float x, float y);
float wii_fmaxf(float x, float y);
float wii_sqrtf(float x);
float wii_sinf(float x);
float wii_sinf_inner(float x);
float wii_cosf(float x);
float wii_cosf_inner(float x);
float wii_atan2f(float y, float x);

#define fminf  wii_fminf
#define fmaxf  wii_fmaxf
#define sqrtf  wii_sqrtf
#define sinf   wii_sinf
#define cosf   wii_cosf
#define atan2f wii_atan2f
#define sinf_inner wii_sinf_inner
#define cosf_inner wii_cosf_inner
#endif

extern const vec2_t vec2_zero;

void  vec3_add   (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_sub   (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_mul   (const vec3_t* a, const float   k, vec3_t* out);
void  vec3_muladd(const vec3_t* a, const float   k, const vec3_t* v, vec3_t* out);
void  vec3_div   (const vec3_t* a, const float   k, vec3_t* out);
float vec3_dot   (const vec3_t* a, const vec3_t* b);
void  vec3_cross (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_cross_plane(const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_proj_unit  (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_rej_unit   (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_lerp  (const vec3_t* a, const vec3_t* b, const float k, vec3_t* out);
float vec3_magsqr(const vec3_t* v);
float vec3_magu  (const vec3_t* v);
float vec3_mag   (const vec3_t* v);
float vec3_norm  (vec3_t* v);
void  vec3_sin   (const vec3_t* v, vec3_t* out);
void  vec3_cos   (const vec3_t* v, vec3_t* out);
void  vec3_min   (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_max   (const vec3_t* a, const vec3_t* b, vec3_t* out);
void  vec3_radians(const vec3_t* v, vec3_t* out);
void  vec3_degrees(const vec3_t* v, vec3_t* out);
void  vec3_print_hex(const vec3_t* v);

extern const vec3_t vec3_zero;
extern const vec3_t vec3_right;
extern const vec3_t vec3_up;
extern const vec3_t vec3_down;
extern const vec3_t vec3_front;
extern const vec3_t vec3_back;

void  quat_init_angles(quat_t* q, const vec3_t* angles);
void  quat_init_axis_angle(quat_t* q, const vec3_t* axis, const float angle);
void  quat_init_vecs(quat_t* q, const vec3_t* from, const vec3_t* to);
void  quat_addq  (const quat_t* q, const quat_t* p, quat_t* out);
void  quat_mulf  (const quat_t* q, const float   s, quat_t* out);
void  quat_mulv  (const quat_t* q, const vec3_t* v, quat_t* out);
void  quat_mulq  (const quat_t* q, const quat_t* p, quat_t* out);
float quat_dot   (const quat_t* q, const quat_t* p);
float quat_magsqr(const quat_t* q);
float quat_norm  (quat_t* q);
void  quat_rotate(const quat_t* q, const vec3_t* v, vec3_t* out);
void  quat_inv_rotate(const quat_t* q, const vec3_t* v, vec3_t* out);
void  quat_invert(const quat_t* q, quat_t* out);
void  quat_slerp (const quat_t* q, const quat_t* p, const float lerp, quat_t* out);
void  quat_vec3  (const quat_t* q, vec3_t* out);
void  quat_angles(const quat_t* q, vec3_t* out);
void  quat_print_hex(const quat_t* q);

extern const quat_t quat_identity;
extern const quat_t quat_back;

void mat33_init_mat34(mat33_t* m, const mat34_t* n);
void mat33_mulv (const mat33_t* m, const vec3_t* v, vec3_t* out);

void mat34_init_angles_pos(mat34_t* m, const vec3_t* angles, const vec3_t* pos);
void mat34_init_quat_pos  (mat34_t* m, const quat_t* q, const vec3_t* pos);
void mat34_init_axis_angle(mat34_t* m, const vec3_t* axis, const float angle);
void mat34_init_diag      (mat34_t* m, const vec3_t* diag);
void mat34_transpose(const mat34_t* m, mat34_t* o);
void mat34_mulv (const mat34_t* m, const vec3_t* v, vec3_t* out);
void mat34_mulm (const mat34_t* m, const mat34_t* n, mat34_t* o);
void mat34_front(const mat34_t* m, vec3_t* out);

void mat44_init_pos(mat44_t* m, const vec3_t* pos);
void mat44_init_angles_pos(mat44_t* m, const vec3_t* angles, const vec3_t* pos);
void mat44_init_view(mat44_t* m, const vec3_t* front, const vec3_t* right, const vec3_t* up, const vec3_t* pos);
void mat44_init_proj(mat44_t* m, float fov, float aspect, float znear, float zfar);
void mat44_transpose(const mat44_t* m, mat44_t* o);
void mat44_mulm(const mat44_t* m, const mat44_t* n, mat44_t* o);
