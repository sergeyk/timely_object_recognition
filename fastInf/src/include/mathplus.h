/* Copyright 2009 Ariel Jaimovich, Ofer Meshi, Ian McGraw and Gal Elidan */


/*
This file is part of FastInf library.

FastInf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FastInf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FastInf.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This is C header file mathplus.h */

#include <math.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_2PI
#define M_2PI       6.28318530717958647693
#endif
#ifndef M_PI_4
#define M_PI_4      .785398163397448309616
#endif
#ifndef M_1_PI
#define M_1_PI      .318309886183790671538
#endif
#ifndef M_2_PI
#define M_2_PI      .636619772367581343076
#endif
#ifndef M_PISQUARE
#define M_PISQUARE  9.86960440108935861883
#endif
#ifndef M_180_PI
#define M_180_PI    57.2957795130823208768
#endif
#ifndef M_PI_180
#define M_PI_180    1.74532925199432957692e-2
#endif
#ifndef M_E
#define M_E         2.71828182845904523536
#endif
#ifndef M_1_E
#define M_1_E       .367879441171442321596
#endif
#ifndef M_EULER
#define M_EULER     .577215664901532860607
#endif
#ifndef M_LN2
#define M_LN2       .693147180559945309417
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_1_LN2
#define M_1_LN2     1.44269504088896340736
#endif
#ifndef M_1_LN10
#define M_1_LN10    .434294481903251827651
#endif
#ifndef M_LNPI
#define M_LNPI      1.14472988584940017414
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   .707106781186547524401
#endif
#ifndef M_SQRT2PI
#define M_SQRT2PI   2.50662827463100050242
#endif
#ifndef M_SQRTPI
#define M_SQRTPI    1.77245385090551602730
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_1_SQRTPI
#define M_1_SQRTPI  .564189583547756286948
#endif
#ifndef M_LOG2E
#define M_LOG2E     M_1_LN2
#endif
#ifndef M_LOG10E
#define M_LOG10E    M_1_LN10
#endif

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#ifndef MAX
#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) ((x) <= (y) ? (x) : (y))
#endif
#define SGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define FLOOR(x) ((int) ((x) + (1 - (int) (x))) - (1 - (int) (x)))
#define CEIL(x) ((2 + (int) (x)) - (int) ((2 + (int) (x)) - (x)))
#define ROUND(x) ((x) >= 0 ? (int) ((x) + .5) : -((int) (.5 - (x))))

/* cubic root function */
#define CBRT(x) ((x) > 0 ? exp(log(x) * .333333333333333333333) : ((x) < 0 ? -exp(log(-(x)) * .333333333333333333333) : 0))

/* log Gamma function LGAMMA(x) := log(gamma(x)) */
#define LGAMMA(x) (log((((((.0112405826571654074 / ((x) + 5.00035898848319255) + .502197227033920907) / ((x) + 3.99999663000075089) + 2.09629553538949977) / ((x) + 3.00000004672652415) + 2.25023047535618168) / ((x) + 1.99999999962010231) + .851370813165034183) / ((x) + 1.00000000000065532) + .122425977326879918) / (x) + .0056360656189756065) + ((x) - .5) * log((x) + 6.09750757539068576) - (x))

/* error function ERFC(x) := integral_x^infinity M_2_SQRTPI*exp(-t*t) dt */
#define ERFC(x) (exp(-((x) * (x))) * (x) * (1.66642447174307753e-7 / ((x) * (x) + 14.8455557345597957) + 9.71013284010551623e-6 / ((x) * (x) + 10.4765104356545238) + 1.91395813098742864e-4 / ((x) * (x) + 7.41471251099335407) + .00221290116681517573 / ((x) * (x) + 4.95867777128246701) + .0156907543161966709 / ((x) * (x) + 2.99957952311300634) + .0681866451424939493 / ((x) * (x) + 1.53039662058770397) + .18158112513463707 / ((x) * (x) + .550942780056002085) + .296316885199227378 / ((x) * (x) + .0612158644495538758)) + ((x) < 6.10399733098688199 ? 2 / (exp((x) * 12.6974899965115684) + 1) : 0))

#define lgamma(x) LGAMMA(x)
#define cbrt(x) CBRT(x)
#define erfc(x) ERFC(x)
#define erf(x) ERFC(x)
#define max(x,y) MAX(x,y)
#define min(x,y) MIN(x,y)
