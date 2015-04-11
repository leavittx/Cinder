/*
 Copyright (c) 2012, The Cinder Project: http://libcinder.org All rights reserved.
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

#include "cinder/Vector.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "cinder/Ray.h"

namespace cinder {

class Sphere;

// By default the camera is looking down -Z
class Camera {
  public:
	Camera() : mModelViewCached( false ), mProjectionCached( false ), mInverseModelViewCached( false ), mWorldUp( vec3( 0, 1, 0 ) ) {}
	virtual ~Camera() {}

	vec3		getEyePoint() const { return mEyePoint; }
	void		setEyePoint( const vec3 &aEyePoint );
	
	vec3		getWorldUp() const { return mWorldUp; }
	void		setWorldUp( const vec3 &aWorldUp );

	void		lookAt( const vec3 &target );
	void		lookAt( const vec3 &aEyePoint, const vec3 &target );
	void		lookAt( const vec3 &aEyePoint, const vec3 &target, const vec3 &aUp );
	vec3		getViewDirection() const { return mViewDirection; }
	void		setViewDirection( const vec3 &aViewDirection );

	quat		getOrientation() const { return mOrientation; }
	void		setOrientation( const quat &aOrientation );

	//! Returns the camera's vertical field of view measured in degrees.
	float	getFov() const { return mFov; }
	//! Sets the camera's vertical field of view measured in degrees.
	void	setFov( float aFov ) { mFov = aFov;  mProjectionCached = false; }
	//! Returns the camera's horizontal field of view measured in degrees.
	float	getFovHorizontal() const { return toDegrees( 2.0f * math<float>::atan( math<float>::tan( toRadians(mFov) * 0.5f ) * mAspectRatio ) ); }
	//! Sets the camera's horizontal field of view measured in degrees.
	void	setFovHorizontal( float verticalFov ) { mFov = toDegrees( 2.0f * math<float>::atan( math<float>::tan( toRadians(verticalFov) * 0.5f ) / mAspectRatio ) );  mProjectionCached = false; }
	//! Returns the camera's focal length, calculating it based on the field of view.
	float	getFocalLength() const;

	float	getAspectRatio() const { return mAspectRatio; }
	void	setAspectRatio( float aAspectRatio ) { mAspectRatio = aAspectRatio; mProjectionCached = false; }
	float	getNearClip() const { return mNearClip; }
	void	setNearClip( float aNearClip ) { mNearClip = aNearClip; mProjectionCached = false; }
	float	getFarClip() const { return mFarClip; }
	void	setFarClip( float aFarClip ) { mFarClip = aFarClip; mProjectionCached = false; }

	virtual void	getNearClipCoordinates( vec3 *topLeft, vec3 *topRight, vec3 *bottomLeft, vec3 *bottomRight ) const;
	virtual void	getFarClipCoordinates( vec3 *topLeft, vec3 *topRight, vec3 *bottomLeft, vec3 *bottomRight ) const;

	//! Returns the coordinates of the camera's frustum, suitable for passing to \c glFrustum
	void	getFrustum( float *left, float *top, float *right, float *bottom, float *near, float *far ) const;
	//! Returns whether the camera represents a perspective projection instead of an orthographic
	virtual bool isPersp() const = 0;
	
	virtual const mat4&	getProjectionMatrix() const { if( ! mProjectionCached ) calcProjection(); return mProjectionMatrix; }
	virtual const mat4&	getViewMatrix() const { if( ! mModelViewCached ) calcViewMatrix(); return mViewMatrix; }
	virtual const mat4&	getInverseViewMatrix() const { if( ! mInverseModelViewCached ) calcInverseView(); return mInverseModelViewMatrix; }

	Ray		generateRay( float u, float v, float imagePlaneAspectRatio ) const;
	void	getBillboardVectors( vec3 *right, vec3 *up ) const;

	//! Converts a world-space coordinate \a worldCoord to screen coordinates as viewed by the camera, based ona s screen which is \a screenWidth x \a screenHeight pixels.
 	vec2 worldToScreen( const vec3 &worldCoord, float screenWidth, float screenHeight ) const;
	//! Converts a world-space coordinate \a worldCoord to eye-space, also known as camera-space. -Z is along the view direction.
 	vec3 worldToEye( const vec3 &worldCoord )	{ return vec3( getViewMatrix() * vec4( worldCoord, 1 ) ); }
 	//! Converts a world-space coordinate \a worldCoord to the z axis of eye-space, also known as camera-space. -Z is along the view direction. Suitable for depth sorting.
 	float worldToEyeDepth( const vec3 &worldCoord ) const;
 	//! Converts a world-space coordinate \a worldCoord to normalized device coordinates
 	vec3 worldToNdc( const vec3 &worldCoord );

	float	calcScreenArea( const Sphere &sphere, const vec2 &screenSizePixels ) const;

  protected:
	void			calcMatrices() const;

	virtual void	calcViewMatrix() const;
	virtual void	calcInverseView() const;
	virtual void	calcProjection() const = 0;

	vec3	mEyePoint;
	vec3	mViewDirection;
	quat	mOrientation;
	vec3	mWorldUp;

	float	mFov; // vertical field of view in degrees
	float	mAspectRatio;
	float	mNearClip;		
	float	mFarClip;

	mutable vec3	mU;	// Right vector
	mutable vec3	mV;	// Readjust up-vector
	mutable vec3	mW;	// Negative view direction

	mutable mat4	mProjectionMatrix, mInverseProjectionMatrix;
	mutable bool	mProjectionCached;
	mutable mat4	mViewMatrix;
	mutable bool	mModelViewCached;
	mutable mat4	mInverseModelViewMatrix;
	mutable bool	mInverseModelViewCached;
	
	mutable float	mFrustumLeft, mFrustumRight, mFrustumTop, mFrustumBottom;
};

class CameraPersp : public Camera {
  public:
	CameraPersp();
	CameraPersp( int pixelWidth, int pixelHeight, float fov ); // constructs screen-aligned camera
	CameraPersp( int pixelWidth, int pixelHeight, float fov, float nearPlane, float farPlane ); // constructs screen-aligned camera
	
	void	setPerspective( float verticalFovDegrees, float aspectRatio, float nearPlane, float farPlane );
	
	/** Returns both the horizontal and vertical lens shift. 
		A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
		A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport. */
	void	getLensShift( float *horizontal, float *vertical ) const { *horizontal = mLensShift.x; *vertical = mLensShift.y; }
	/** Returns both the horizontal and vertical lens shift. 
		A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
		A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport. */
	vec2	getLensShift() const { return vec2( mLensShift.x, mLensShift.y ); }
	/** Sets both the horizontal and vertical lens shift. 
		A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
		A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport. */
	void	setLensShift( float horizontal, float vertical );
	/** Sets both the horizontal and vertical lens shift. 
		A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
		A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport. */
	void	setLensShift( const vec2 &shift ) { setLensShift( shift.x, shift.y ); }
	//! Returns the horizontal lens shift. A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
	float	getLensShiftHorizontal() const { return mLensShift.x; }
	/** Sets the horizontal lens shift. 
		A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport. */
	void	setLensShiftHorizontal( float horizontal ) { setLensShift( horizontal, mLensShift.y ); }
	//! Returns the vertical lens shift. A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport.
	float	getLensShiftVertical() const { return mLensShift.y; }
	/** Sets the vertical lens shift. 
		A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport. */
	void	setLensShiftVertical( float vertical ) { setLensShift( mLensShift.x, vertical ); }
	
	virtual bool	isPersp() const { return true; }

	//! Returns a Camera whose eyePoint is positioned to exactly frame \a worldSpaceSphere but is equivalent in other parameters (including orientation).
	CameraPersp		calcFraming( const Sphere &worldSpaceSphere ) const;

  protected:
	vec2	mLensShift;

	virtual void	calcProjection() const;
};

class CameraOrtho : public Camera {
  public:
	CameraOrtho();
	CameraOrtho( float left, float right, float bottom, float top, float nearPlane, float farPlane );

	void setOrtho( float left, float right, float bottom, float top, float nearPlane, float farPlane );

	virtual bool	isPersp() const { return false; }
	
  protected:
	virtual void	calcProjection() const;
};

class CameraStereo : public CameraPersp {
  public:
	CameraStereo() 
		: mConvergence(1.0f), mEyeSeparation(0.05f), mIsStereo(false), mIsLeft(true) {}
	CameraStereo( int pixelWidth, int pixelHeight, float fov )
		: CameraPersp( pixelWidth, pixelHeight, fov ), 
		mConvergence(1.0f), mEyeSeparation(0.05f), mIsStereo(false), mIsLeft(true) {} // constructs screen-aligned camera
	CameraStereo( int pixelWidth, int pixelHeight, float fov, float nearPlane, float farPlane )
		: CameraPersp( pixelWidth, pixelHeight, fov, nearPlane, farPlane ), 
		mConvergence(1.0f), mEyeSeparation(0.05f), mIsStereo(false), mIsLeft(true) {} // constructs screen-aligned camera

	//! Returns the current convergence, which is the distance at which there is no parallax.
	float			getConvergence() const { return mConvergence; }
	//! Sets the convergence of the camera, which is the distance at which there is no parallax.
	void			setConvergence( float distance, bool adjustEyeSeparation=false ) { 
		mConvergence = distance; mProjectionCached = false;

		if( adjustEyeSeparation )
			mEyeSeparation = mConvergence / 30.0f;
	}
	//! Returns the distance between the camera's for the left and right eyes.
	float			getEyeSeparation() const { return mEyeSeparation; }
	//! Sets the distance between the camera's for the left and right eyes. This affects the parallax effect. 
	void			setEyeSeparation( float distance ) { mEyeSeparation = distance; mModelViewCached = false; mProjectionCached = false; }
	//! Returns the location of the currently enabled eye camera.
	vec3			getEyePointShifted() const;
	
	//! Enables the left eye camera.
	void			enableStereoLeft() { mIsStereo = true; mIsLeft = true; }
	bool			isStereoLeftEnabled() const { return mIsStereo && mIsLeft; }
	//! Enables the right eye camera.
	void			enableStereoRight() { mIsStereo = true; mIsLeft = false; }
	bool			isStereoRightEnabled() const { return mIsStereo && !mIsLeft; }
	//! Disables stereoscopic rendering, converting the camera to a standard CameraPersp.
	void			disableStereo() { mIsStereo = false; }
	bool			isStereoEnabled() const { return mIsStereo; }

	virtual void	getNearClipCoordinates( vec3 *topLeft, vec3 *topRight, vec3 *bottomLeft, vec3 *bottomRight ) const;
	virtual void	getFarClipCoordinates( vec3 *topLeft, vec3 *topRight, vec3 *bottomLeft, vec3 *bottomRight ) const;
	
	virtual const mat4&	getProjectionMatrix() const override;
	virtual const mat4&	getViewMatrix() const override;
	virtual const mat4&	getInverseViewMatrix() const override;

  protected:
	mutable mat4	mProjectionMatrixLeft, mInverseProjectionMatrixLeft;
	mutable mat4	mProjectionMatrixRight, mInverseProjectionMatrixRight;
	mutable mat4	mViewMatrixLeft, mInverseModelViewMatrixLeft;
	mutable mat4	mViewMatrixRight, mInverseModelViewMatrixRight;

	void	calcViewMatrix() const override;
	void	calcInverseView() const override;
	void	calcProjection() const override;
	
  private:
	bool			mIsStereo;
	bool			mIsLeft;

	float			mConvergence;
	float			mEyeSeparation;
};

} // namespace cinder