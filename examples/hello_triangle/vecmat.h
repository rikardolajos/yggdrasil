/* Copyright 2024 Rikard Olajos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <math.h>

/* GCC and CLANG */
#if defined(__GNUC__) || defined(__clang__)

#include <x86intrin.h>
#define ALIGN(x) __attribute__((aligned(x)))

/* MSVC */
#elif defined(_MSC_VER)

#include <intrin.h>
#define ALIGN(x) __declspec(align(x))

#else
#error "VECMAT: Unknown compiler"
#endif


typedef ALIGN(16) union {
    struct {
        float x, y;
    };
    struct {
        float u, v;
    };
    float array[4];
    __m128 sse;
} vec2;

typedef ALIGN(16) union {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    float array[4];
    __m128 sse;
} vec3;

typedef ALIGN(16) union {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
    float array[4];
    __m128 sse;
} vec4;

typedef ALIGN(16) union {
    vec4 cols[4];
    float cr[4][4];
    float array[16];
} mat4;


#ifdef VECMAT_USE_GENERICS

/* Add two generic types together */
#define add(a, b)                                                              \
    _Generic((a),                                                              \
        vec2: _Generic((b), vec2: vec2_add, default: vec2_add),                \
        vec3: _Generic((b), vec3: vec3_add, default: vec3_add),                \
        vec4: _Generic((b), vec4: vec4_add, default: vec4_add),                \
        mat4: _Generic((b), mat4: mat4_add, default: mat4_add))(a, b)

/* Subtract two generic types from each other */
#define sub(a, b)                                                              \
    _Generic((a),                                                              \
        vec2: _Generic((b), vec2: vec2_sub, default: vec2_sub),                \
        vec3: _Generic((b), vec3: vec3_sub, default: vec3_sub),                \
        vec4: _Generic((b), vec4: vec4_sub, default: vec4_sub),                \
        mat4: _Generic((b), mat4: mat4_sub, default: mat4_sub))(a, b)

/* Scalar multiplication of a generic type */
#define scale(f, a)                                                            \
    _Generic((a),                                                              \
        vec2: vec2_scale,                                                      \
        vec3: vec3_scale,                                                      \
        vec4: vec4_scale,                                                      \
        mat4: mat4_scale)(f, a)

/* Dot multiplication of two vectors */
#define dot(a, b)                                                              \
    _Generic((a),                                                              \
        vec2: _Generic((b), vec2: vec2_dot, default: vec2_dot),                \
        vec3: _Generic((b), vec3: vec3_dot, default: vec3_dot),                \
        vec4: _Generic((b), vec4: vec4_dot, default: vec4_dot))(a, b)

/* Cross multiplication of two vec3 */
#define cross(a, b) _Generic((a), vec3: _Generic((b), vec3: vec3_cross))(a, b)

/* Calculate norm of a generic type */
#define norm(a)                                                                \
    _Generic((a), vec2: vec2_norm, vec3: vec3_norm, vec4: vec4_norm)(a)

/* Calculate normalized vector with same direction */
#define normalize(a)                                                           \
    _Generic((a),                                                              \
        vec2: vec2_normalize,                                                  \
        vec3: vec3_normalize,                                                  \
        vec4: vec4_normalize)(a)

#endif /* VECMAT_USE_GENERICS */


#define MAT4_IDENTITY                                                          \
    (mat4)                                                                     \
    {                                                                          \
        {                                                                      \
            {1.0f, 0.0f, 0.0f, 0.0f},                                          \
            {0.0f, 1.0f, 0.0f, 0.0f},                                          \
            {0.0f, 0.0f, 1.0f, 0.0f},                                          \
            {0.0f, 0.0f, 0.0f, 1.0f},                                          \
        },                                                                     \
    }


/* Addition for vec2 */
static inline vec2 vec2_add(vec2 u, vec2 v)
{
    return (vec2){.sse = _mm_add_ps(u.sse, v.sse)};
}

/* Addition for vec3 */
static inline vec3 vec3_add(vec3 u, vec3 v)
{
    return (vec3){.sse = _mm_add_ps(u.sse, v.sse)};
}

/* Addition for vec4 */
static inline vec4 vec4_add(vec4 u, vec4 v)
{
    return (vec4){.sse = _mm_add_ps(u.sse, v.sse)};
}


/* Subtraction for vec2 */
static inline vec2 vec2_sub(vec2 u, vec2 v)
{
    return (vec2){.sse = _mm_sub_ps(u.sse, v.sse)};
}

/* Subtraction for vec3 */
static inline vec3 vec3_sub(vec3 u, vec3 v)
{
    return (vec3){.sse = _mm_sub_ps(u.sse, v.sse)};
}

/* Subtraction for vec4 */
static inline vec4 vec4_sub(vec4 u, vec4 v)
{
    return (vec4){.sse = _mm_sub_ps(u.sse, v.sse)};
}


/* Scalar multiplication for vec2 */
static inline vec2 vec2_scale(float f, vec2 u)
{
    return (vec2){.sse = _mm_mul_ps(_mm_set_ps1(f), u.sse)};
}

/* Scalar multiplication for vec3 */
static inline vec3 vec3_scale(float f, vec3 u)
{
    return (vec3){.sse = _mm_mul_ps(_mm_set_ps1(f), u.sse)};
}

/* Scalar multiplication for vec4 */
static inline vec4 vec4_scale(float f, vec4 u)
{
    return (vec4){.sse = _mm_mul_ps(_mm_set_ps1(f), u.sse)};
}


/* Dot multiplication for vec2 */
static inline float vec2_dot(vec2 u, vec2 v)
{
    vec2 f;
    _mm_store_ps1(f.array, _mm_dp_ps(u.sse, v.sse, 0x31));
    return f.x;
}

/* Dot multiplication for vec3 */
static inline float vec3_dot(vec3 u, vec3 v)
{
    vec3 f;
    _mm_store_ps1(f.array, _mm_dp_ps(u.sse, v.sse, 0x71));
    return f.x;
}

/* Dot multiplication for vec4 */
static inline float vec4_dot(vec4 u, vec4 v)
{
    vec4 f;
    _mm_store_ps1(f.array, _mm_dp_ps(u.sse, v.sse, 0xf1));
    return f.x;
}


/* Cross multiplication for vec3 */
static inline vec3 vec3_cross(vec3 u, vec3 v)
{
    __m128 tmp0 = _mm_shuffle_ps(u.sse, u.sse, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(v.sse, v.sse, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 tmp2 = _mm_mul_ps(tmp0, v.sse);
    __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    return (vec3){.sse = _mm_sub_ps(tmp3, tmp4)};
}


/* Calculate the norm of a vec2 */
static inline float vec2_norm(vec2 u)
{
    return sqrtf(vec2_dot(u, u));
}

/* Calculate the norm of a vec3 */
static inline float vec3_norm(vec3 u)
{
    return sqrtf(vec3_dot(u, u));
}

/* Calculate the norm of a vec4 */
static inline float vec4_norm(vec4 u)
{
    return sqrtf(vec4_dot(u, u));
}


/* Normalization of vec2 */
static inline vec2 vec2_normalize(vec2 u)
{
    float n = vec2_norm(u);
    return n > 0.0f ? vec2_scale(1 / n, u) : u;
}

/* Normalization of vec3 */
static inline vec3 vec3_normalize(vec3 u)
{
    float n = vec3_norm(u);
    return n > 0.0f ? vec3_scale(1 / n, u) : u;
}

/* Normalization of vec4 */
static inline vec4 vec4_normalize(vec4 u)
{
    float n = vec4_norm(u);
    return n > 0.0f ? vec4_scale(1 / n, u) : u;
}


/* Transpose matrix */
static inline mat4 mat4_transpose(mat4 m)
{
    _MM_TRANSPOSE4_PS(m.cols[0].sse, m.cols[1].sse, m.cols[2].sse,
                      m.cols[3].sse);
    return m;
}


/* Add two matrices together */
static inline mat4 mat4_add(mat4 m, mat4 n)
{
    mat4 res;
    for (int i = 0; i < 4; i++) {
        res.cols[i] = vec4_add(m.cols[i], n.cols[i]);
    }
    return res;
}


/* Subtract two matrices */
static inline mat4 mat4_sub(mat4 m, mat4 n)
{
    mat4 res;
    for (int i = 0; i < 4; i++) {
        res.cols[i] = vec4_sub(m.cols[i], n.cols[i]);
    }
    return res;
}


/* Scalar multiplication of matrix */
static inline mat4 mat4_scale(float f, mat4 m)
{
    mat4 res;
    for (int i = 0; i < 4; i++) {
        res.cols[i] = vec4_scale(f, m.cols[i]);
    }
    return res;
}


/* Matrix multiplication */
static inline mat4 mat4_mul(mat4 m, mat4 n)
{
    mat4 res;

    mat4 mt = mat4_transpose(m);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res.cr[i][j] = vec4_dot(mt.cols[i], n.cols[j]);
        }
    }

    return mat4_transpose(res);
}


/* Matrix inverse */
static inline mat4 mat4_inverse(mat4 m)
{
    mat4 a;

    a.array[0] = m.array[5] * m.array[10] * m.array[15] -
                 m.array[5] * m.array[11] * m.array[14] -
                 m.array[9] * m.array[6] * m.array[15] +
                 m.array[9] * m.array[7] * m.array[14] +
                 m.array[13] * m.array[6] * m.array[11] -
                 m.array[13] * m.array[7] * m.array[10];

    a.array[4] = -m.array[4] * m.array[10] * m.array[15] +
                 m.array[4] * m.array[11] * m.array[14] +
                 m.array[8] * m.array[6] * m.array[15] -
                 m.array[8] * m.array[7] * m.array[14] -
                 m.array[12] * m.array[6] * m.array[11] +
                 m.array[12] * m.array[7] * m.array[10];

    a.array[8] = m.array[4] * m.array[9] * m.array[15] -
                 m.array[4] * m.array[11] * m.array[13] -
                 m.array[8] * m.array[5] * m.array[15] +
                 m.array[8] * m.array[7] * m.array[13] +
                 m.array[12] * m.array[5] * m.array[11] -
                 m.array[12] * m.array[7] * m.array[9];

    a.array[12] = -m.array[4] * m.array[9] * m.array[14] +
                  m.array[4] * m.array[10] * m.array[13] +
                  m.array[8] * m.array[5] * m.array[14] -
                  m.array[8] * m.array[6] * m.array[13] -
                  m.array[12] * m.array[5] * m.array[10] +
                  m.array[12] * m.array[6] * m.array[9];

    a.array[1] = -m.array[1] * m.array[10] * m.array[15] +
                 m.array[1] * m.array[11] * m.array[14] +
                 m.array[9] * m.array[2] * m.array[15] -
                 m.array[9] * m.array[3] * m.array[14] -
                 m.array[13] * m.array[2] * m.array[11] +
                 m.array[13] * m.array[3] * m.array[10];

    a.array[5] = m.array[0] * m.array[10] * m.array[15] -
                 m.array[0] * m.array[11] * m.array[14] -
                 m.array[8] * m.array[2] * m.array[15] +
                 m.array[8] * m.array[3] * m.array[14] +
                 m.array[12] * m.array[2] * m.array[11] -
                 m.array[12] * m.array[3] * m.array[10];

    a.array[9] = -m.array[0] * m.array[9] * m.array[15] +
                 m.array[0] * m.array[11] * m.array[13] +
                 m.array[8] * m.array[1] * m.array[15] -
                 m.array[8] * m.array[3] * m.array[13] -
                 m.array[12] * m.array[1] * m.array[11] +
                 m.array[12] * m.array[3] * m.array[9];

    a.array[13] = m.array[0] * m.array[9] * m.array[14] -
                  m.array[0] * m.array[10] * m.array[13] -
                  m.array[8] * m.array[1] * m.array[14] +
                  m.array[8] * m.array[2] * m.array[13] +
                  m.array[12] * m.array[1] * m.array[10] -
                  m.array[12] * m.array[2] * m.array[9];

    a.array[2] = m.array[1] * m.array[6] * m.array[15] -
                 m.array[1] * m.array[7] * m.array[14] -
                 m.array[5] * m.array[2] * m.array[15] +
                 m.array[5] * m.array[3] * m.array[14] +
                 m.array[13] * m.array[2] * m.array[7] -
                 m.array[13] * m.array[3] * m.array[6];

    a.array[6] = -m.array[0] * m.array[6] * m.array[15] +
                 m.array[0] * m.array[7] * m.array[14] +
                 m.array[4] * m.array[2] * m.array[15] -
                 m.array[4] * m.array[3] * m.array[14] -
                 m.array[12] * m.array[2] * m.array[7] +
                 m.array[12] * m.array[3] * m.array[6];

    a.array[10] = m.array[0] * m.array[5] * m.array[15] -
                  m.array[0] * m.array[7] * m.array[13] -
                  m.array[4] * m.array[1] * m.array[15] +
                  m.array[4] * m.array[3] * m.array[13] +
                  m.array[12] * m.array[1] * m.array[7] -
                  m.array[12] * m.array[3] * m.array[5];

    a.array[14] = -m.array[0] * m.array[5] * m.array[14] +
                  m.array[0] * m.array[6] * m.array[13] +
                  m.array[4] * m.array[1] * m.array[14] -
                  m.array[4] * m.array[2] * m.array[13] -
                  m.array[12] * m.array[1] * m.array[6] +
                  m.array[12] * m.array[2] * m.array[5];

    a.array[3] = -m.array[1] * m.array[6] * m.array[11] +
                 m.array[1] * m.array[7] * m.array[10] +
                 m.array[5] * m.array[2] * m.array[11] -
                 m.array[5] * m.array[3] * m.array[10] -
                 m.array[9] * m.array[2] * m.array[7] +
                 m.array[9] * m.array[3] * m.array[6];

    a.array[7] = m.array[0] * m.array[6] * m.array[11] -
                 m.array[0] * m.array[7] * m.array[10] -
                 m.array[4] * m.array[2] * m.array[11] +
                 m.array[4] * m.array[3] * m.array[10] +
                 m.array[8] * m.array[2] * m.array[7] -
                 m.array[8] * m.array[3] * m.array[6];

    a.array[11] = -m.array[0] * m.array[5] * m.array[11] +
                  m.array[0] * m.array[7] * m.array[9] +
                  m.array[4] * m.array[1] * m.array[11] -
                  m.array[4] * m.array[3] * m.array[9] -
                  m.array[8] * m.array[1] * m.array[7] +
                  m.array[8] * m.array[3] * m.array[5];

    a.array[15] = m.array[0] * m.array[5] * m.array[10] -
                  m.array[0] * m.array[6] * m.array[9] -
                  m.array[4] * m.array[1] * m.array[10] +
                  m.array[4] * m.array[2] * m.array[9] +
                  m.array[8] * m.array[1] * m.array[6] -
                  m.array[8] * m.array[2] * m.array[5];

    float det = m.array[0] * a.array[0] + m.array[1] * a.array[4] +
                m.array[2] * a.array[8] + m.array[3] * a.array[12];

    mat4 res;

    for (int i = 0; i < 4; i++) {
        res.cols[i] = vec4_scale(1 / det, a.cols[i]);
    }

    return res;
}


/* Scale transform matrix */
static inline mat4 mat4_trs_scale(vec3 v1)
{
    return (mat4){v1.x, 0, 0, 0, 0, v1.y, 0, 0, 0, 0, v1.z, 0, 0, 0, 0, 1};
}


/* Rotation transform matrix */
static inline mat4 mat4_trs_rotate(float angle, vec3 axis)
{
    axis = vec3_normalize(axis);
    return mat4_transpose((mat4){{
        {powf(axis.x, 2) + (1 - powf(axis.x, 2)) * cosf(angle),
         axis.x * axis.y * (1 - cosf(angle)) - axis.z * sinf(angle),
         axis.x * axis.z * (1 - cosf(angle)) + axis.y * sinf(angle), 0},
        {axis.x * axis.y * (1 - cosf(angle)) + axis.z * sinf(angle),
         powf(axis.y, 2) + (1 - powf(axis.y, 2)) * cosf(angle),
         axis.y * axis.z * (1 - cosf(angle)) - axis.x * sinf(angle), 0},
        {axis.x * axis.z * (1 - cosf(angle)) - axis.y * sinf(angle),
         axis.y * axis.z * (1 - cosf(angle)) + axis.x * sinf(angle),
         powf(axis.z, 2) + (1 - powf(axis.z, 2)) * cosf(angle), 0},
        {0, 0, 0, 1},
    }});
}


/* Translation transform matrix */
static inline mat4 mat4_trs_translate(vec3 v1)
{
    return (mat4){{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {v1.x, v1.y, v1.z, 1.0f},
    }};
}


/* Rotate v around axis with angle */
static inline vec3 vec3_rotate(vec3 v, vec3 axis, float angle)
{
    axis = vec3_normalize(axis);
    vec3 q = {
        sinf(angle / 2.0f) * axis.x,
        sinf(angle / 2.0f) * axis.y,
        sinf(angle / 2.0f) * axis.z,
    };
    vec3 t = vec3_cross(vec3_scale(2.0f, q), v);
    return vec3_add(vec3_add(v, vec3_scale(cosf(angle / 2.0f), t)),
                    vec3_cross(q, t));
}


/* Return a orthographic projection matrix */
static inline mat4 mat4_orthographic(float right, float top, float near,
                                     float far)
{
    return mat4_transpose((mat4){{
        {1.0f / right, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f / top, 0.0f, 0.0f},
        {0.0f, 0.0f, (-2) / (far - near), -(far + near) / (far - near)},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }});
}


/* Return a perspective projection matrix */
static inline mat4 mat4_perspective(int width, int height, float zNear,
                                    float zFar, float fov)
{
    float f = tanf(1.57079632679f - 0.5f * fov);
    float aspect = (float)width / (float)height;
    float depth = (-zFar) / (zFar - zNear);
    return mat4_transpose((mat4){{{f / aspect, 0.0f, 0.0f, 0.0f},
                                  {0.0f, -f, 0.0f, 0.0f},
                                  {0.0f, 0.0f, depth, zNear * depth},
                                  {0.0f, 0.0f, -1.0f, 0.0f}}});
}


/* Return a rotation matrix oriented towards the target */
static inline mat4 mat4_lookat(vec3 pos, vec3 target, vec3 up)
{
    vec3 f = vec3_normalize(vec3_sub(pos, target));
    vec3 u = vec3_normalize(up);
    vec3 s = vec3_normalize(vec3_cross(f, u));
    u = vec3_cross(s, f);

    return mat4_transpose((mat4){{{s.x, s.y, s.z, -vec3_dot(s, pos)},
                                  {u.x, u.y, u.z, -vec3_dot(u, pos)},
                                  {f.x, f.y, f.z, -vec3_dot(f, pos)},
                                  {0.0f, 0.0f, 0.0f, 1.0f}}});
}
