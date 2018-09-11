/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED matrix routines
 *
 * Based on matrix code written originally by Benny Bobaganoosh for his 3d software
 * renderer (https://github.com/BennyQBD/3DSoftwareRenderer).
 *
 *      FULL ATTRIBUTION:
 *      -----------------
 *      Copyright (c) 2014, Benny Bobaganoosh
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright notice, this
 *          list of conditions and the following disclaimer.
 *      2. Redistributions in binary form must reproduce the above copyright notice,
 *          this list of conditions and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *      ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *      ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *      (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *      LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *      ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <cmath>
#include "../../core/geometry.h"
#include "../../core/display.h"
#include "../../core/fixedpt.h"
#include "../../core/types.h"

/*
 * TODOS
 *
 * - the code is c-like by design, but maybe convert to c++ later.
 *
 */

void g_transform_vert(vertex4_s *const v, const matrix44_s *const m)
{
    const i32 x0 = m->data[0] * v->x + m->data[4] * v->y + m->data[8] * v->z + m->data[12] * v->w;
    const i32 y0 = m->data[1] * v->x + m->data[5] * v->y + m->data[9] * v->z + m->data[13] * v->w;
    const i32 z0 = m->data[2] * v->x + m->data[6] * v->y + m->data[10] * v->z + m->data[14] * v->w;
    const i32 w0 = m->data[3] * v->x + m->data[7] * v->y + m->data[11] * v->z + m->data[15] * v->w;

    v->x = x0;
    v->y = y0;
    v->z = z0;
    v->w = w0;

    return;
}

void g_mul_two_mats(const matrix44_s *const m1, const matrix44_s *const m2, matrix44_s *const dstMat)
{
    k_assert(((m1 != m2) &&
              (m1 != dstMat) &&
              (m2 != dstMat)), "Can't do in-place matrix multiplication.");

    for(uint i = 0; i < 4; i++)
    {
        for(uint j = 0; j < 4; j++)
        {
            dstMat->data[i + (j * 4)] = m1->data[i + (0 * 4)] * m2->data[0 + (j * 4)] +
                                        m1->data[i + (1 * 4)] * m2->data[1 + (j * 4)] +
                                        m1->data[i + (2 * 4)] * m2->data[2 + (j * 4)] +
                                        m1->data[i + (3 * 4)] * m2->data[3 + (j * 4)];
        }
    }

    return;
}

// NOTE: At the moment, the x,y,z coordinates must arrive as u16 fixed-point values,
// where 0 = 0 degrees, and ~0u = 359 degrees.
//
void g_make_rot_mat(matrix44_s *const dstMat,
                    u16 x, u16 y, u16 z)
{
    matrix44_s rx, ry, rz, tmp;

    x = FP_UNPACK_FOR_SINE_LUT(x);
    y = FP_UNPACK_FOR_SINE_LUT(y);
    z = FP_UNPACK_FOR_SINE_LUT(z);

    rx.data[0] = 1;       rx.data[4] = 0;        rx.data[8]  = 0;        rx.data[12] = 0;
    rx.data[1] = 0;       rx.data[5] = COS(x);	 rx.data[9]  = -SIN(x);  rx.data[13] = 0;
    rx.data[2] = 0;       rx.data[6] = SIN(x);   rx.data[10] = COS(x);   rx.data[14] = 0;
    rx.data[3] = 0;       rx.data[7] = 0;        rx.data[11] = 0;        rx.data[15] = 1;

    ry.data[0] = COS(y);  ry.data[4] = 0;        ry.data[8]  = -SIN(y);  ry.data[12] = 0;
    ry.data[1] = 0;       ry.data[5] = 1;        ry.data[9]  = 0;        ry.data[13] = 0;
    ry.data[2] = SIN(y);  ry.data[6] = 0;        ry.data[10] = COS(y);   ry.data[14] = 0;
    ry.data[3] = 0;       ry.data[7] = 0;        ry.data[11] = 0;        ry.data[15] = 1;

    rz.data[0] = COS(z);  rz.data[4] = -SIN(z);	 rz.data[8]  = 0;        rz.data[12] = 0;
    rz.data[1] = SIN(z);  rz.data[5] = COS(z);	 rz.data[9]  = 0;        rz.data[13] = 0;
    rz.data[2] = 0;       rz.data[6] = 0;        rz.data[10] = 1;        rz.data[14] = 0;
    rz.data[3] = 0;       rz.data[7] = 0;        rz.data[11] = 0;        rz.data[15] = 1;

    g_mul_two_mats(&rz, &ry, &tmp);
    g_mul_two_mats(&rx, &tmp, dstMat);

    return;
}

void g_make_transl_mat(matrix44_s *const dstMat,
                       const i32 x, const i32 y, const i32 z)
{
    dstMat->data[0] = 1;	dstMat->data[4] = 0;	dstMat->data[8]  = 0;	dstMat->data[12] = x;
    dstMat->data[1] = 0;	dstMat->data[5] = 1;	dstMat->data[9]  = 0;	dstMat->data[13] = y;
    dstMat->data[2] = 0;	dstMat->data[6] = 0;	dstMat->data[10] = 1;	dstMat->data[14] = z;
    dstMat->data[3] = 0;	dstMat->data[7] = 0;	dstMat->data[11] = 0;	dstMat->data[15] = 1;

    return;
}

void g_make_persp_mat(matrix44_s *const dstMat,
                      const real fov, const real aspectRatio,
                      const i32 zNear, const i32 zFar)
{
    const real tanHalfFOV = tan(fov / 2);
    const real zRange = zNear - zFar;

    dstMat->data[0] = 1.0f / (tanHalfFOV * aspectRatio); dstMat->data[4] = 0;					dstMat->data[8] = 0;                         dstMat->data[12] = 0;
    dstMat->data[1] = 0;                                 dstMat->data[5] = 1.0f / tanHalfFOV;	dstMat->data[9] = 0;                         dstMat->data[13] = 0;
    dstMat->data[2] = 0;                                 dstMat->data[6] = 0;					dstMat->data[10] = (-zNear -zFar)/zRange;	 dstMat->data[14] = 2 * zFar * zNear / zRange;
    dstMat->data[3] = 0;                                 dstMat->data[7] = 0;					dstMat->data[11] = 1;                        dstMat->data[15] = 0;

    return;
}

void g_make_scaling_mat(matrix44_s *const dstMat,
                        const i32 x, const i32 y, const i32 z)
{
    dstMat->data[0] = x;	dstMat->data[4] = 0;	dstMat->data[8]  = 0;	dstMat->data[12] = 0;
    dstMat->data[1] = 0;	dstMat->data[5] = y;	dstMat->data[9]  = 0;	dstMat->data[13] = 0;
    dstMat->data[2] = 0;	dstMat->data[6] = 0;	dstMat->data[10] = z;	dstMat->data[14] = 0;
    dstMat->data[3] = 0;	dstMat->data[7] = 0;	dstMat->data[11] = 0;	dstMat->data[15] = 1;

    return;
}

void g_make_screen_space_mat(matrix44_s *const dstMat,
                             const real halfWidth, const real halfHeight)
{
    dstMat->data[0] = halfWidth; dstMat->data[4] = 0;             dstMat->data[8]  = 0;     dstMat->data[12] = halfWidth - 0.5f;
    dstMat->data[1] = 0;         dstMat->data[5] = -halfHeight;   dstMat->data[9]  = 0;     dstMat->data[13] = halfHeight - 0.5f;
    dstMat->data[2] = 0;         dstMat->data[6] = 0;             dstMat->data[10] = 1;     dstMat->data[14] = 0;
    dstMat->data[3] = 0;         dstMat->data[7] = 0;             dstMat->data[11] = 0;     dstMat->data[15] = 1;

    return;
}
