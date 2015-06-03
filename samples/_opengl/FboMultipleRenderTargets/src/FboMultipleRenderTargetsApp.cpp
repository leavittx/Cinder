// This sample shows how to use a ci::gl::Fbo with multiple color attachments.
// It renders a spinning cube into two color attachments, one green and one blue.
// See multipleOut.frag for how to output frag colors in the shader.

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;

class FboMultipleRenderTargetsApp : public App {
  public:
	virtual void	setup() override;
	virtual void	update() override;
	virtual void	draw() override;

  private:
	void			renderSceneToFbo();
	
	gl::FboRef			mFbo;
	gl::Texture2dRef	mTexAttachment0, mTexAttachment1;
	gl::GlslProgRef		mGlslMultipleOuts;
	mat4				mRotation;
	static const int	FBO_WIDTH = 256, FBO_HEIGHT = 256;
};

void FboMultipleRenderTargetsApp::setup()
{
	mTexAttachment0 = gl::Texture2d::create( FBO_WIDTH, FBO_HEIGHT );
	mTexAttachment1 = gl::Texture2d::create( FBO_WIDTH, FBO_HEIGHT );
	auto format = gl::Fbo::Format()
//			.samples( 4 ) // uncomment this to enable 4x antialiasing // FIXME: causes drawing to be all white
			.attachment( GL_COLOR_ATTACHMENT0, mTexAttachment0 )
			.attachment( GL_COLOR_ATTACHMENT1, mTexAttachment1 );
	mFbo = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, format );

	try {
		mGlslMultipleOuts = gl::GlslProg::create( loadAsset( "multipleOut.vert" ), loadAsset( "multipleOut.frag" ) );
	}
	catch( Exception &exc ) {
		CI_LOG_EXCEPTION( "failed to load shader", exc );
	}

	gl::enableDepthRead();
	gl::enableDepthWrite();	

	mRotation = mat4( 1 );
}

// Render our scene into the FBO (a cube)
void FboMultipleRenderTargetsApp::renderSceneToFbo()
{
	// setup our camera to render our scene
	CameraPersp cam( mFbo->getWidth(), mFbo->getHeight(), 60 );
	cam.setPerspective( 60, mFbo->getAspectRatio(), 1, 1000 );
	cam.lookAt( vec3( 2.8f, 1.8f, -2.8f ), vec3( 0 ) );

	// bind our framebuffer in a safe way:
	gl::ScopedFramebuffer fboScope( mFbo );

	// clear out both of the attachments of the FBO with black
	gl::clear();

	// setup the viewport to match the dimensions of the FBO, storing the previous state
	gl::ScopedViewport viewportScope( ivec2( 0 ), mFbo->getSize() );

	// store matrices before updating for CameraPersp
	gl::ScopedMatrices matScope;
	gl::setMatrices( cam );

	// set the modelview matrix to reflect our current rotation
	gl::multModelMatrix( mRotation );

	// render the torus with our multiple-output shader
	gl::ScopedGlslProg glslScope( mGlslMultipleOuts );
	gl::setDefaultShaderVars();
	gl::drawCube( vec3( 0 ), vec3( 2.2f ) );
}

void FboMultipleRenderTargetsApp::update()
{
	// Rotate the cube around an arbitrary axis
	mRotation *= rotate( 0.06f, normalize( vec3( 0.166f, 0.333f, 0.666f ) ) );

	// render into mFbo
	if( mFbo && mGlslMultipleOuts )
		renderSceneToFbo();
}

void FboMultipleRenderTargetsApp::draw()
{
	gl::clear( Color::gray( 0.35f ) );

	// draw the two textures we've created side-by-side
	gl::setMatricesWindow( getWindowSize() );
//	gl::draw( mFbo->getTexture(0), mFbo->getTexture(0)->getBounds() );
//	gl::draw( mFbo->getTexture(1), mFbo->getTexture(1)->getBounds() + vec2( mFbo->getTexture(0)->getWidth(), 0 ) );

	gl::draw( mTexAttachment0, mTexAttachment0->getBounds() );
	gl::draw( mTexAttachment1, mTexAttachment1->getBounds() + vec2( mTexAttachment0->getWidth(), 0 ) );
}

CINDER_APP( FboMultipleRenderTargetsApp, RendererGl )
