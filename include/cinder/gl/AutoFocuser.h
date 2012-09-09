/*
 Copyright (c) 2012, The Cinder Project, All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Portions of this code (C) Paul Houx
 All rights reserved.

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
*/

#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

// forward declarations
namespace cinder {
	class Camera; class CameraPersp; class CameraStereo;
} // namespace cinder

namespace cinder { namespace gl {

class AutoFocuser {
 public:
	AutoFocuser() 
		: mSpeed(1.0f), mDepth(1.0f) {}
	~AutoFocuser() { destroyBuffers(); }

	/** Attempts to set an ideal focal length and eye separation. 
		\a cam is the CameraStereo you use to render the scene and which should be auto-focussed.
		If your autoFocusSpeed is less than 1.0, repeatedly call this function from your update() method.
	*/
	void					autoFocus( CameraStereo &cam );

	//! Returns the speed at which auto-focussing takes place.
	float					getAutoFocusSpeed() const { return mSpeed; }

	/** Sets the speed at which auto-focussing takes place. A value of 1.0 will immediately focus on the measured value.
		Lower values will gradually adjust the focal length.
		If your autoFocusSpeed is less than 1.0, repeatedly call the autoFocus() function from your update() method.
	*/
	void					setAutoFocusSpeed( float factor ) { mSpeed = math<float>::clamp( factor, 0.01f, 1.0f); }

	//! Returns the auto-focus depth. 
	float					getAutoFocusDepth() const { return mDepth; }

	/** Sets the auto-focus depth. A value of 1.0 will adjust the focal length in such a way that the nearest objects
		are at the plane of the screen and cause no parallax. Lower values will cause the nearest objects to appear behind your 
		screen (positive parallax). Values greater than 1.0 will cause objects to appear in front of your screen (negative parallax).
		Avoid values much greater than 1.0 to reduce eye strain.
	*/
	void					setAutoFocusDepth( float factor ) { mDepth = math<float>::max( factor, 0.01f); }

	//!
	Vec2f					getNearestPixel() const { return Vec2f( mNearest.x, mNearest.y ); }
	float					getNearestDepth() const { return mNearest.z; }

	//!
	inline Area				getAutoFocusArea() const;

	//!
	void					draw();
private:
	void					createBuffers( const Area &area );
	void					destroyBuffers();
public:
	//! width and height of the auto focus sample 
	static const int		AF_WIDTH = 64;
	static const int		AF_HEIGHT = 64;
private:
	float					mSpeed;
	float					mDepth;

	Fbo						mFboSmall;
	Fbo						mFboLarge;
	std::vector<GLfloat>	mBuffer; 

	//! keeps track of the nearest depth and pixel
	Vec3f					mNearest;
};

} } // namespace cinder::gl 
