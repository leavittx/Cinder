/*
 Copyright (c) 2011, The Cinder Project, All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Portions Copyright (c) 2004, Laminar Research.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 Portions Copyright Geometric Tools LLC, Redmond WA 98052
 Boost Software License - Version 1.0 - August 17th, 2003

 Permission is hereby granted, free of charge, to any person or organization
 obtaining a copy of the software and accompanying documentation covered by
 this license (the "Software") to use, reproduce, display, distribute,
 execute, and transmit the Software, and to prepare derivative works of the
 Software, and to permit third-parties to whom the Software is furnished to
 do so, all subject to the following:

 The copyright notices in the Software and this entire statement, including
 the above license grant, this restriction and the following disclaimer,
 must be included in all copies of the Software, in whole or in part, and
 all derivative works of the Software, unless such copies or derivative
 works are solely in the form of machine-executable object code generated by
 a source language processor.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/


#include "cinder/CinderMath.h"

using namespace glm;

namespace cinder {


/////////////////////////////////////////////////////////////////////////////////////////////////
// solveCubic
template<typename T>
int solveCubic( T a, T b, T c, T d, T result[3] )
{
	if( a == 0 )
		return solveQuadratic( b, c, d, result );

	T f = ((3 * c / a) - ((b * b) / (a * a))) / 3;
	T g = ((2 * b * b * b) / (a * a * a) - (9 * b * c) / (a * a) + (27 * d) / (a)) / 27;
	T h = g * g / 4 + f * f * f / 27;

	if( f == 0 && g == 0 && h == 0 ) {
		result[0] = -math<T>::cbrt( d / a );
		return 1;
	}
	else if( h > 0 ) {
		// 1 root
		T r = -( g / 2 ) + math<T>::sqrt( h );
		T s = math<T>::cbrt( r );
		T t = -(g / 2) - math<T>::sqrt( h );
		T u = math<T>::cbrt( t );

		result[0] = (s + u) - (b / (3 * a));
		return 1;
	}
	else { // 3 roots
		T i = math<T>::sqrt( (g * g / 4) - h );
		T j = math<T>::cbrt( i );
		T k = math<T>::acos( -(g / (2 * i)) );
		T l = -j;
		T m = math<T>::cos( k / 3 );
		T n = math<T>::sqrt(3) * math<T>::sin( k / 3 );
		T p = -b / (3 * a);
		result[0] = 2 * j * math<T>::cos(k / 3) - (b / (3 * a));
		result[1] = l * (m + n) + p;
		result[2] = l * (m - n) + p;
		return 3;
	}
}
template int solveCubic( float a, float b, float c, float d, float result[3] );
template int solveCubic( double a, double b, double c, double d, double result[3] );

namespace {
float PointOnEllipseBisector( int numComponents, const vec2 &extents, const vec2 &y, vec2& x )
{
	vec2 z;
	float sumZSqr = 0;
	int i;
	for( i = 0; i < numComponents; ++i ) {
		z[i] = y[i] / extents[i];
		sumZSqr += z[i] * z[i];
	}

	if( sumZSqr == 1 ) {
		// The point is on the hyperellipsoid.
		for (i = 0; i < numComponents; ++i)
			x[i] = y[i];

		return 0;
	}

	float emin = extents[numComponents - 1];
	vec2 pSqr, numerator;
	for( i = 0; i < numComponents; ++i ) {
		float p = extents[i] / emin;
		pSqr[i] = p * p;
		numerator[i] = pSqr[i] * z[i];
	}

	// The maximum number of bisections required for Real before the interval
	// endpoints are equal (as floating-point numbers).
	int const jmax = std::numeric_limits<float>::digits - std::numeric_limits<float>::min_exponent;

	float s = 0, smin = z[numComponents - 1] - 1, smax;
	if( sumZSqr < 1 )
		// The point is strictly inside the hyperellipsoid.
		smax = 0;
	else
		// The point is strictly outside the hyperellipsoid.
		//smax = LengthRobust(numerator) - (Real)1;
		smax = length( numerator ) - 1;

	for( int j = 0; j < jmax; ++j ) {
		s = (smin + smax) * 0.5f;
		if (s == smin || s == smax)
			break;

		float g = -1;
		for( i = 0; i < numComponents; ++i ) {
			float ratio = numerator[i] / (s + pSqr[i]);
			g += ratio * ratio;
		}

		if( g > 0 )
			smin = s;
		else if( g < 0 )
			smax = s;
		else
			break;
	}

	float sqrDistance = 0;
	for( i = 0; i < numComponents; ++i ) {
		x[i] = pSqr[i] * y[i] / (s + pSqr[i]);
		float diff = x[i] - y[i];
		sqrDistance += diff*diff;
	}
	return sqrDistance;
}

float PointOnEllipseSqrDistanceSpecial( const vec2 &extents, const vec2 &y, vec2 &x )
{
    float sqrDistance = 0;

    vec2 ePos, yPos, xPos;
    int numPos = 0;
    for( int i = 0; i < 2; ++i ) {
        if( y[i] > 0 ) {
            ePos[numPos] = extents[i];
            yPos[numPos] = y[i];
            ++numPos;
        }
        else
            x[i] = 0;
    }

    if( y[2 - 1] > 0 )
        sqrDistance = PointOnEllipseBisector( numPos, ePos, yPos, xPos );
    else {  // y[N-1] = 0
        float numer[1], denom[1];
        float eNm1Sqr = extents[2 - 1] * extents[2 - 1];
        for( int i = 0; i < numPos; ++i)
        {
            numer[i] = ePos[i] * yPos[i];
            denom[i] = ePos[i] * ePos[i] - eNm1Sqr;
        }

        bool inSubHyperbox = true;
        for( int i = 0; i < numPos; ++i) {
            if( numer[i] >= denom[i]) {
                inSubHyperbox = false;
                break;
            }
        }

        bool inSubHyperellipsoid = false;
        if( inSubHyperbox ) {
            // yPos[] is inside the axis-aligned bounding box of the
            // subhyperellipsoid.  This intermediate test is designed to guard
            // against the division by zero when ePos[i] == e[N-1] for some i.
            float xde[1];
            float discr = 1;
            for( int i = 0; i < numPos; ++i)
            {
                xde[i] = numer[i] / denom[i];
                discr -= xde[i] * xde[i];
            }
            if( discr > 0 ) {
                // yPos[] is inside the subhyperellipsoid.  The closest
                // hyperellipsoid point has x[N-1] > 0.
                sqrDistance = 0;
                for( int i = 0; i < numPos; ++i)
                {
                    xPos[i] = ePos[i] * xde[i];
                    float diff = xPos[i] - yPos[i];
                    sqrDistance += diff*diff;
                }
                x[2 - 1] = extents[2 - 1] * sqrt(discr);
                sqrDistance += x[2 - 1] * x[2 - 1];
                inSubHyperellipsoid = true;
            }
        }

        if( ! inSubHyperellipsoid ) {
            // yPos[] is outside the subhyperellipsoid.  The closest
            // hyperellipsoid point has x[N-1] == 0 and is on the
            // domain-boundary hyperellipsoid.
            x[2 - 1] = 0;
            sqrDistance = PointOnEllipseBisector( numPos, ePos, yPos, xPos );
        }
    }

    // Fill in those x[] values that were not zeroed out initially.
    for( int i = 0, numPos = 0; i < 2; ++i ) {
        if( y[i] > 0 ) {
            x[i] = xPos[numPos];
            ++numPos;
        }
    }

    return sqrDistance;
}

float PointOnEllipseSqrDistance( const vec2 &extents, const vec2 &y, vec2 &x )
{
    // Determine negations for y to the first octant.
    bool negate[2];
    for( int i = 0; i < 2; ++i )
        negate[i] = y[i] < 0;

    // Determine the axis order for decreasing extents.
    std::pair<float, int> permute[2];
    for( int i = 0; i < 2; ++i ) {
        permute[i].first = -extents[i];
        permute[i].second = i;
    }
    std::sort( &permute[0], &permute[2] );

    int invPermute[2];
    for( int i = 0; i < 2; ++i )
        invPermute[permute[i].second] = i;

    vec2 locE, locY;
	int j;
    for( int i = 0; i < 2; ++i ) {
        j = permute[i].second;
        locE[i] = extents[j];
        locY[i] = std::abs(y[j]);
    }

    vec2 locX;
    float sqrDistance = PointOnEllipseSqrDistanceSpecial( locE, locY, locX );

    // Restore the axis order and reflections.
    for( int i = 0; i < 2; ++i ) {
        j = invPermute[i];
        if( negate[i] )
            locX[j] = -locX[j];
        x[i] = locX[j];
    }

    return sqrDistance;
}
} // anonymous namespace for closestPointOnEllipse

vec2 getClosestPointEllipse( const vec2& center, const vec2& axisA, const vec2& axisB, const vec2& testPoint )
{
	// Compute the coordinates of Y in the hyperellipsoid coordinate system.
	float lengthA = length( axisA );
	float lengthB = length( axisB );
	vec2 unitA = axisA / lengthA;
	vec2 unitB = axisB / lengthB;
	vec2 diff = testPoint - center;
	vec2 y( dot( diff, unitA ), dot( diff, unitB ) );

	// Compute the closest hyperellipsoid point in the axis-aligned
	// coordinate system.
	vec2 x;
	vec2 extents( lengthA, lengthB );
	PointOnEllipseSqrDistance( extents, y, x );

	// Convert back to the original coordinate system.
	vec2 result = center;
	result += x[0] * unitA;
	result += x[1] * unitB;

	return result;
}


} // namespace cinder