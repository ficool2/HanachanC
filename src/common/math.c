#include "../common.h"

#include "math.h"

#ifdef WII_MATH
#include "math_wii.h"
#endif

const vec2_t vec2_zero  = { 0.f, 0.f };

const vec3_t vec3_zero  = { 0.0f, 0.0f, 0.0f  };
const vec3_t vec3_right = { 1.0f, 0.0f, 0.0f  };
const vec3_t vec3_up    = { 0.0f, 1.0f, 0.0f  };
const vec3_t vec3_down  = { 0.0f, -1.0, 0.0f  };
const vec3_t vec3_front = { 0.0f, 0.0f, 1.0f  };
const vec3_t vec3_back  = { 0.0f, 0.0f, -1.0f };

const quat_t quat_identity = { 0.0f, 0.0f, 0.0f, 1.0f };
const quat_t quat_back     = { 0.0f, 1.0f, 0.0f, 0.0f };

float clampf(float x, float min, float max)
{
    return fminf(fmaxf(x, min), max);
}

float fracf(float x)
{
    float integral;
    return modff(x, &integral);
}

float degreesf(float x)
{
    return x * (float)(180.0 / M_PI);
}

float radiansf(float x)
{
    return x * (M_PI_F / 180.0f);
}

float anglenormf(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle < -180.0f)
        angle += 360.0f;
    else if (angle > 180.0f)
        angle -= 360.0f;
    return angle;
}

void float_print_hex(const float f)
{
    float_bits b = { .val = f };
    printf("%x\n", b.bits);
}


void vec3_add(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y; 
    out->z = a->z + b->z;
}

void vec3_sub(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;  
}

void vec3_mul(const vec3_t* a, const float k, vec3_t* out)
{
    out->x = a->x * k;
    out->y = a->y * k;
    out->z = a->z * k;
}

void vec3_muladd(const vec3_t* a, const float k, const vec3_t* v, vec3_t* out)
{
    out->x = a->x + v->x * k;
    out->y = a->y + v->y * k;
    out->z = a->z + v->z * k;
}

void vec3_div(const vec3_t* a, const float k, vec3_t* out)
{
    out->x = a->x / k;
    out->y = a->y / k;
    out->z = a->z / k;  
}

float vec3_dot(const vec3_t* a, const vec3_t* b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

void vec3_cross(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
}

void vec3_cross_plane(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    if (fabs(vec3_dot(a, b)) == 1.0f)
    {
        *out = vec3_zero;
        return;
    }

    vec3_t perp;
    vec3_cross(b, a, &perp);
    vec3_cross(&perp, b, out);
}

void vec3_proj_unit(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    float dot = vec3_dot(a, b);
    vec3_mul(b, dot, out);
}

void vec3_rej_unit(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    vec3_t p;
    vec3_proj_unit(a, b, &p);
    vec3_sub(a, &p, out);
}

void vec3_lerp(const vec3_t* a, const vec3_t* b, const float k, vec3_t* out)
{
    vec3_t temp;
    vec3_mul(a, 1.0f - k, &temp);
    vec3_muladd(&temp, k, b, out);
}

float vec3_magsqr(const vec3_t* v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

float vec3_magu(const vec3_t* v)
{
    return sqrtf(vec3_magsqr(v));
}

float vec3_mag(const vec3_t* v)
{
    float magsqr = vec3_magsqr(v);
    if (magsqr <= FLT_EPSILON)
        return 0.0f;
    else
        return sqrtf(magsqr);
}

float vec3_norm(vec3_t* v)
{
    float mag = vec3_mag(v);
    if (mag != 0.0f)
        vec3_mul(v, 1.0f / mag, v);
    return mag;
}

void vec3_sin(const vec3_t* v, vec3_t* out)
{
    out->x = sinf(v->x);
    out->y = sinf(v->y);
    out->z = sinf(v->z);
}

void vec3_cos(const vec3_t* v, vec3_t* out)
{
    out->x = cosf(v->x);
    out->y = cosf(v->y);
    out->z = cosf(v->z);
}

void vec3_min(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    out->x = fminf(a->x, b->x);
    out->y = fminf(a->y, b->y);
    out->z = fminf(a->z, b->z);
}

void vec3_max(const vec3_t* a, const vec3_t* b, vec3_t* out)
{
    out->x = fmaxf(a->x, b->x);
    out->y = fmaxf(a->y, b->y);
    out->z = fmaxf(a->z, b->z);
}

void vec3_radians(const vec3_t* v, vec3_t* out)
{
    out->x = radiansf(v->x);
    out->y = radiansf(v->y);
    out->z = radiansf(v->z);
}

void vec3_degrees(const vec3_t* v, vec3_t* out)
{
    out->x = degreesf(v->x);
    out->y = degreesf(v->y);
    out->z = degreesf(v->z);
}

void vec3_print_hex(const vec3_t* v)
{
    float_bits b[3];
    b[0].val = v->x;
    b[1].val = v->y;
    b[2].val = v->z;
    printf("%x %x %x\n", b[0].bits, b[1].bits, b[2].bits);
}


void quat_init_angles(quat_t* q, const vec3_t* angles)
{
    vec3_t half_angles, sinh, cosh;
    vec3_mul(angles, 0.5f, &half_angles);
    vec3_sin(&half_angles, &sinh);
    vec3_cos(&half_angles, &cosh);
    
    q->x = cosh.z * cosh.y * sinh.x - sinh.z * sinh.y * cosh.x;
    q->y = cosh.z * sinh.y * cosh.x + sinh.z * cosh.y * sinh.x;
    q->z = sinh.z * cosh.y * cosh.x - cosh.z * sinh.y * sinh.x;
    q->w = cosh.z * cosh.y * cosh.x + sinh.z * sinh.y * sinh.x;
}

void quat_init_axis_angle(quat_t* q, const vec3_t* axis, const float angle)
{
    float half = angle * 0.5f;
    float s = sinf(half);
    float c = cosf(half);

    q->x = s * axis->x;
    q->y = s * axis->y;
    q->z = s * axis->z;
    q->w = c;
}

void quat_init_vecs(quat_t* q, const vec3_t* from, const vec3_t* to)
{
    float s = sqrtf(2.0f * (vec3_dot(from, to) + 1.0f));
    if (s <= FLT_EPSILON)
    {
        *q = quat_identity;
        return;
    }
    
    vec3_t cross;
    float recip = 1.0f / s;
    vec3_cross(from, to, &cross);

    q->x = recip * cross.x;
    q->y = recip * cross.y;
    q->z = recip * cross.z;
    q->w = 0.5f * s;
}

void quat_addq(const quat_t* q, const quat_t* p, quat_t* out)
{
    out->x = q->x + p->x;
    out->y = q->y + p->y;
    out->z = q->z + p->z;
    out->w = q->w + p->w;
}

void quat_mulf(const quat_t* q, const float s, quat_t* out)
{
    out->x = q->x * s;
    out->y = q->y * s;
    out->z = q->z * s;
    out->w = q->w * s;
}

void quat_mulv(const quat_t* q, const vec3_t* v, quat_t* out)
{
    out->x =   q->y * v->z - q->z * v->y + q->w * v->x;
    out->y =   q->z * v->x - q->x * v->z + q->w * v->y;
    out->z =   q->x * v->y - q->y * v->x + q->w * v->z;
    out->w = -(q->x * v->x + q->y * v->y + q->z * v->z);
}

void quat_mulq(const quat_t* q, const quat_t* p, quat_t* out)
{
    out->x = q->w * p->x + q->x * p->w + q->y * p->z - q->z * p->y;
    out->y = q->w * p->y + q->y * p->w + q->z * p->x - q->x * p->z;
    out->z = q->w * p->z + q->z * p->w + q->x * p->y - q->y * p->x;
    out->w = q->w * p->w - q->x * p->x - q->y * p->y - q->z * p->z;
}

float quat_dot(const quat_t* q, const quat_t* p)
{
    return q->x * p->x + q->y * p->y + q->z * p->z + q->w * p->w;
}

float quat_magsqr(const quat_t* q)
{
    return quat_dot(q, q);
}

float quat_norm(quat_t* q)
{
    float sq_norm = quat_magsqr(q);
    if (sq_norm <= FLT_EPSILON)
        return 0.0f;
    quat_mulf(q, 1.0f / sqrtf(sq_norm), q);
    return sq_norm;
}

void quat_rotate(const quat_t* q, const vec3_t* v, vec3_t* out)
{
    quat_t p, r, s;
    quat_invert(q, &r);
    quat_mulv  (q, v, &p);
    quat_mulq  (&p, &r, &s);
    quat_vec3  (&s, out);
}

void quat_inv_rotate(const quat_t* q, const vec3_t* v, vec3_t* out)
{
    quat_t p, r, s;
    quat_invert(q, &r);
    quat_mulv  (&r, v, &p);
    quat_mulq  (&p, q, &s);
    quat_vec3  (&s, out);
}

void quat_invert(const quat_t* q, quat_t* out)
{
    out->x = -q->x;
    out->y = -q->y;
    out->z = -q->z;
    out->w =  q->w;
}

void quat_slerp(const quat_t* q, const quat_t* p, const float lerp, quat_t* out)
{
    float dot = clampf(quat_dot(q, p), -1.0f, 1.0f);
    float angle = (float)acos((double)fabsf(dot));
    float sine = sinf(angle);
    float s, t;

    if (fabsf(sine) >= 1e-5)
    {
        float recip = 1.0f / sine;
        s = recip * sinf((angle - lerp * angle));
        t = recip * sinf(lerp * angle);
    }
    else
    {
        s = 1.0f - lerp;
        t = lerp;
    }

    t = copysignf(1.0f, dot) * t;

    quat_t qs, qt;
    quat_mulf(q, s, &qs);
    quat_mulf(p, t, &qt);
    quat_addq(&qs, &qt, out);
}

void quat_vec3(const quat_t* q, vec3_t* out)
{
    out->x = q->x;
    out->y = q->y;
    out->z = q->z;
}

void quat_angles(const quat_t* q, vec3_t* out)
{
    const float a0 = (q->w * q->w + q->x * q->x) - q->y * q->y - q->z * q->z;
    const float a1 = (q->x * q->y + q->w * q->z) * 2.0f;
    const float a2 = (q->x * q->z - q->w * q->y) * 2.0f;
    const float a3 = (q->y * q->z + q->w * q->x) * 2.0f;
    const float a4 = (q->w * q->w - q->x * q->x) - q->y * q->y + q->z * q->z;
    const float a5 = fabsf(a2);

    if (a5 > 0.999999f)
    {
        float t0 = (q->x * q->y - q->w * q->z) * 2.0f;
        float t1 = (q->x * q->z + q->w * q->y) * 2.0f;

        out->x = 0.0f;
        out->y = a2 / a5 * -(M_PI_F * 0.5f);
        out->z = atan2f(-t0, -a2 * t1);
    }
    else 
    {
        out->x = atan2f(a3, a4);
        out->y = asinf(-a2);
        out->z = atan2f(a1, a0);
    }
}

void quat_print_hex(const quat_t* q)
{
    float_bits b[4];
    b[0].val = q->x;
    b[1].val = q->y;
    b[2].val = q->z;
    b[3].val = q->w;
    printf("%x %x %x %x\n", b[0].bits, b[1].bits, b[2].bits, b[3].bits);
}


void mat33_init_mat34(mat33_t* m, const mat34_t* n)
{
    m->m00 = n->m00;
    m->m01 = n->m01;
    m->m02 = n->m02;
    m->m10 = n->m10;
    m->m11 = n->m11;
    m->m12 = n->m12;
    m->m20 = n->m20;
    m->m21 = n->m21;
    m->m22 = n->m22;
}

void mat33_mulv(const mat33_t* m, const vec3_t* v, vec3_t* out)
{
    out->x = m->m00 * v->x + m->m01 * v->y + m->m02 * v->z;
    out->y = m->m10 * v->x + m->m11 * v->y + m->m12 * v->z;
    out->z = m->m20 * v->x + m->m21 * v->y + m->m22 * v->z;
}

void mat34_init_angles_pos(mat34_t* m, const vec3_t* angles, const vec3_t* pos)
{
    vec3_t s, c;
    vec3_sin(angles, &s);
    vec3_cos(angles, &c);

    m->m00 = c.y * c.z;
    m->m01 = s.x * s.y * c.z - s.z * c.x;
    m->m02 = c.x * c.z * s.y + s.x * s.z;
    m->m03 = pos->x;
    m->m10 = s.z * c.y;
    m->m11 = s.x * s.y * s.z + c.x * c.z;
    m->m12 = s.z * c.x * s.y - s.x * c.z;
    m->m13 = pos->y;
    m->m20 = -s.y;
    m->m21 = s.x * c.y;
    m->m22 = c.x * c.y;
    m->m23 = pos->z;
}

void mat34_init_quat_pos(mat34_t* m, const quat_t* q, const vec3_t* pos)
{
    m->m00 = 1.0f - 2.0f * q->y * q->y - 2.0f * q->z * q->z;
    m->m01 = 2.0f * q->x * q->y - 2.0f * q->w * q->z;
    m->m02 = 2.0f * q->x * q->z + 2.0f * q->w * q->y;
    m->m03 = pos->x;

    m->m10 = 2.0f * q->x * q->y + 2.0f * q->w * q->z;
    m->m11 = 1.0f - 2.0f * q->x * q->x - 2.0f * q->z * q->z;
    m->m12 = 2.0f * q->y * q->z - 2.0f * q->w * q->x;
    m->m13 = pos->y;

    m->m20 = 2.0f * q->x * q->z - 2.0f * q->w * q->y;
    m->m21 = 2.0f * q->y * q->z + 2.0f * q->w * q->x;
    m->m22 = 1.0f - 2.0f * q->x * q->x - 2.0f * q->y * q->y;
    m->m23 = pos->z;
}

void mat34_init_axis_angle(mat34_t* m, const vec3_t* axis, const float angle)
{
    quat_t q;
    quat_init_axis_angle(&q, axis, angle);
    mat34_init_quat_pos(m, &q, &vec3_zero);
}

void mat34_init_diag(mat34_t* m, const vec3_t* diag)
{
    m->m00 = diag->x;
    m->m01 = 0.0f;
    m->m02 = 0.0f;
    m->m03 = 0.0f;

    m->m10 = 0.0f;
    m->m11 = diag->y;
    m->m12 = 0.0f;
    m->m13 = 0.0f;

    m->m20 = 0.0f;
    m->m21 = 0.0f;
    m->m22 = diag->z;
    m->m23 = 0.0f;
}

void mat34_transpose(const mat34_t* m, mat34_t* o)
{
    o->m00 = m->m00;
    o->m01 = m->m10;
    o->m02 = m->m20;
    o->m03 = 0.0f;

    o->m10 = m->m01;
    o->m11 = m->m11;
    o->m12 = m->m21;
    o->m13 = 0.0f;

    o->m20 = m->m02;
    o->m21 = m->m12;
    o->m22 = m->m22;
    o->m23 = 0.0f;
}

void mat34_mulv(const mat34_t* m, const vec3_t* v, vec3_t* out)
{
    for (int i = 0; i < 3; i++)
    {
        const float* row = &m->m[i * 4];
        float tmp0 = row[0] * v->x;
        tmp0 = (float)((double)row[2] * (double)v->z + (double)tmp0);
        float tmp1 = row[1] * v->y + row[3];
        out->v[i] = tmp0 + tmp1;
    }
}

void mat34_mulm(const mat34_t* m, const mat34_t* n, mat34_t* o)
{
    const float cols[4][4]  =
    {
        { n->m00, n->m10, n->m20, 0.0f },
        { n->m01, n->m11, n->m21, 0.0f },
        { n->m02, n->m12, n->m22, 0.0f },
        { n->m03, n->m13, n->m23, 1.0f },
    };

    for (int i = 0; i < 3; i++)
    {
        const float* row = &m->m[i * 4];
        for (int j = 0; j < 4; j++)
        {
            const float* col = cols[j];

            float acc = row[0] * col[0];
            acc = (float)((double)row[1] * (double)col[1] + (double)acc);
            acc = (float)((double)row[2] * (double)col[2] + (double)acc);
            acc = (float)((double)row[3] * (double)col[3] + (double)acc);

            o->m[i * 4 + j] = acc;
        }
    }
}

void mat34_front(const mat34_t* m, vec3_t* out)
{
    out->x = m->m02;
    out->y = m->m12;
    out->z = m->m22;
}

void mat44_init_pos(mat44_t* m, const vec3_t* pos)
{
    m->m00 = 1.0f;
    m->m10 = 0.0f;
    m->m20 = 0.0f;
    m->m30 = pos->x;

    m->m01 = 0.0f;
    m->m11 = 1.0f;
    m->m21 = 0.0f;
    m->m31 = pos->y;

    m->m02 = 0.0f;
    m->m12 = 0.0f;
    m->m22 = 1.0f;
    m->m32 = pos->z;

    m->m03 = 0.0f;
    m->m13 = 0.0f;
    m->m23 = 0.0f;
    m->m33 = 1.0f;
}


void mat44_init_angles_pos(mat44_t* m, const vec3_t* angles, const vec3_t* pos)
{
    vec3_t s, c;
    vec3_sin(angles, &s);
    vec3_cos(angles, &c);

    m->m00 = c.y * c.z;
    m->m01 = s.x * s.y * c.z - s.z * c.x;
    m->m02 = c.x * c.z * s.y + s.x * s.z;
    m->m03 = pos->x;
    m->m10 = s.z * c.y;
    m->m11 = s.x * s.y * s.z + c.x * c.z;
    m->m12 = s.z * c.x * s.y - s.x * c.z;
    m->m13 = pos->y;
    m->m20 = -s.y;
    m->m21 = s.x * c.y;
    m->m22 = c.x * c.y;
    m->m23 = pos->z;
    m->m30 = 0.0f;
    m->m31 = 0.0f;
    m->m32 = 0.0f;
    m->m33 = 1.0f;
}

void mat44_init_view(mat44_t* m, const vec3_t* front, const vec3_t* right, const vec3_t* up, const vec3_t* pos)
{
    m->m00 = -right->x;
    m->m01 = up->x;
    m->m02 = -front->x;
    m->m03 = 0.0f;

    m->m10 = -right->y;
    m->m11 = up->y;
    m->m12 = -front->y;
    m->m13 = 0.0f;

    m->m20 = -right->z;
    m->m21 = up->z;
    m->m22 = -front->z;
    m->m23 = 0.0f;
 
    m->m30 = -vec3_dot(right, pos);
    m->m31 = vec3_dot(up, pos);
    m->m32 = -vec3_dot(front, pos);
    m->m33 = 1.0f;
}

void mat44_init_proj(mat44_t* m, float fov, float aspect, float znear, float zfar)
{
    float h = tanf(radiansf(fov * 0.5f));

    m->m00 = 1.0f / (h * aspect);
    m->m01 = 0.0f;
    m->m02 = 0.0f;
    m->m03 = 0.0f;

    m->m10 = 0.0f;
    m->m11 = 1.0f / h;
    m->m12 = 0.0f;
    m->m13 = 0.0f;

    m->m20 = 0.0f;
    m->m21 = 0.0f;
    m->m22 = (zfar + znear) / (znear - zfar);
    m->m23 = -1.0f;

    m->m30 = 0.0f;
    m->m31 = 0.0f;
    m->m32 = 2.0f * zfar * znear / (znear - zfar);
    m->m33 = 1.0f;
}

void mat44_transpose(const mat44_t* m, mat44_t* o) 
{
    o->m00 = m->m00;
    o->m01 = m->m10;
    o->m02 = m->m20;
    o->m03 = m->m30;

    o->m10 = m->m01;
    o->m11 = m->m11;
    o->m12 = m->m21;
    o->m13 = m->m31;

    o->m20 = m->m02;
    o->m21 = m->m12;
    o->m22 = m->m22;
    o->m23 = m->m32;

    o->m30 = m->m03;
    o->m31 = m->m13;
    o->m32 = m->m23;
    o->m33 = m->m33;
}

void mat44_mulm(const mat44_t* m, const mat44_t* n, mat44_t* o)
{
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++)
                sum += m->m[i * 4 + k] * n->m[k * 4 + j];
            o->m[i * 4 + j] = sum;
        }
    }
}