#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/ImageIo.h"
#include "cinder/Json.h"
#include "cinder/Url.h"

#include "Earth.h"
#include "POV.h"

#include "Resources.h"

// For transparency stuff
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")


using namespace ci;
using namespace ci::app;
using namespace std;


namespace
{
  // Use window compositing for transparency
  // https://docs.microsoft.com/en-us/windows/win32/dwm/blur-ovw
  inline bool enableBlurBehind(HWND hwnd)
  {
    HRESULT hr = S_OK;

    // Create and populate the Blur Behind structure
    DWM_BLURBEHIND bb = { 0 };

    // Enable Blur Behind and apply to the entire client area
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = true;
    bb.hRgnBlur = NULL; // Apply to the entire client area

                        // Apply Blur Behind
    hr = DwmEnableBlurBehindWindow(hwnd, &bb);
    if (SUCCEEDED(hr))
    {
      return true;
    }
    return false;
  }
}


class EarthquakeApp : public App {
public:
	static void prepareSettings( Settings *settings );

	void setup();
	void update();
	void draw();

	void keyDown( KeyEvent event );
	void mouseMove( MouseEvent event );
	void mouseWheel( MouseEvent event );

	void parseEarthquakes( const string &url );

public:
	gl::Texture2dRef  mStars;
	gl::BatchRef      mStarSphere;

	POV               mPov;
	Earth             mEarth;

	vec2              mLastMouse;
	vec2              mCurrentMouse;

	vec3              mBillboardUp, mBillboardRight;

	bool              mSaveFrames;
	bool              mShowStars;
	bool              mShowEarth;
	bool              mShowText;
	bool              mShowQuakes;
	int               windowLeft;
	int               windowTop;
	int               windowWidth;
	int               windowHeight;
	bool              isBigScreen;
};

void EarthquakeApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
	settings->setResizable( true );
	settings->setFullScreen( false );
    settings->setBorderless( true );
	settings->setTransparent( true );
}

void EarthquakeApp::setup()
{
  //enableBlurBehind((HWND)getWindow()->getNative());
	windowLeft = this->getWindowPosX();
	windowTop = this->getWindowPosY();
	windowWidth = this->getWindowWidth();
	windowHeight = this->getWindowHeight();
	isBigScreen = false;
	// Load the texture and create the sphere for the background.
	mStars = gl::Texture2d::create( loadImage( loadResource( RES_STARS_PNG ) ) );
	mStarSphere = gl::Batch::create( geom::Sphere().radius( 15000 ).subdivisions( 30 ), gl::getStockShader( gl::ShaderDef().texture() ) );

	// Initialize state.
	mSaveFrames = false;
	mShowStars = false;
	mShowEarth = true;
	mShowQuakes = true;
	mShowText = true;

	// Create the camera controller.
	mPov = POV( this, ci::vec3( 0.0f, 0.0f, 1000.0f ), ci::vec3( 0.0f, 0.0f, 0.0f ) );

	// Load the latest earthquake information.
	parseEarthquakes( "http://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/2.5_week.geojson" );
}

void EarthquakeApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		// Toggle full screen.
		//setFullScreen( !isFullScreen() );
		if (!isBigScreen) {
			DisplayRef display = Display::getMainDisplay();
			ivec2 displaySize = display->getSize();
			this->setWindowPos(0, 0);
			this->setWindowSize(displaySize.x, displaySize.y + 1);
		} else {
			this->setWindowPos(windowLeft, windowTop);
			this->setWindowSize(windowWidth, windowHeight);
		}
		isBigScreen = !isBigScreen;
	}
	else if( event.getCode() == app::KeyEvent::KEY_ESCAPE ) {
		if( isFullScreen() )
			setFullScreen( false );
		else
			quit();
	}
	else if (event.getChar() == 'b') {
		this->getWindow()->setBorderless(!this->getWindow()->isBorderless());
	}
	else if( event.getChar() == 's' ) {
		mSaveFrames = !mSaveFrames;
	}
	else if( event.getChar() == 'e' ) {
		mShowEarth = !mShowEarth;
	}
	else if( event.getChar() == 't' ) {
		mShowText = !mShowText;
	}
	else if( event.getChar() == 'q' ) {
		mShowQuakes = !mShowQuakes;
	}
	else if( event.getChar() == 'm' ) {
		mEarth.setMinMagToRender( -1.0f );
	}
	else if( event.getChar() == 'M' ) {
		mEarth.setMinMagToRender( 1.0f );
	}
	else if( event.getCode() == app::KeyEvent::KEY_UP ) {
		mPov.adjustDist( -10.0f );
	}
	else if( event.getCode() == app::KeyEvent::KEY_DOWN ) {
		mPov.adjustDist( 10.0f );
	}
}

void EarthquakeApp::mouseWheel( MouseEvent event )
{
	mPov.adjustDist( event.getWheelIncrement() * -5.0f );
}

void EarthquakeApp::mouseMove( MouseEvent event )
{
	static bool firstMouseMove = true;

	if( !firstMouseMove ) {
		mLastMouse = mCurrentMouse;
	}
	else {
		mLastMouse = event.getPos();
		firstMouseMove = false;
	}

	mCurrentMouse = event.getPos();

	mPov.adjustAngle( ( mLastMouse.x - mCurrentMouse.x ) * 0.01f, mCurrentMouse.y - ( getWindowHeight() * 0.5f ) );
}

void EarthquakeApp::update()
{
	mPov.update();
	mEarth.update();
}

void EarthquakeApp::draw()
{
  // Won't work with transparency (have no idea why)
  //gl::clearColor(ColorAf(0.f, 0.f, 0.f, 0.f));
  //gl::clear();

  // Will work with transparency
  gl::clear(ColorAf(0.0f, 0.0f, 0.0f, 0.0f));

	gl::ScopedDepth       depth( true );
	gl::ScopedColor       color( 1, 1, 1 );

	// Draw stars.
	if( mShowStars ) {
		gl::ScopedTextureBind tex0( mStars, 0 );
		gl::ScopedFaceCulling cull( true, GL_FRONT );
		mStarSphere->draw();
	}

	// Draw Earth.
	if( mShowEarth ) {
		mEarth.draw();
	}

	// Draw quakes.
	if( mShowQuakes ) {
		mEarth.drawQuakes();
	}

	// Draw labels.
	if( mShowText ) {
		mEarth.drawQuakeLabelsOnSphere( mPov.mEyeNormal, mPov.mDist );
	}

	if( mSaveFrames ) {
		static int currentFrame = 0;
		writeImage( getHomeDirectory() / "CinderScreengrabs" / ( "Highoutput_" + toString( currentFrame++ ) + ".png" ), copyWindowSurface() );
	}
}

void EarthquakeApp::parseEarthquakes( const string &url )
{
	try {
		const JsonTree json( loadUrl( url ) );
		for( auto &feature : json["features"].getChildren() ) {
			auto &coords = feature["geometry"]["coordinates"];
			float mag = feature["properties"]["mag"].getValue<float>();
			const string &title = feature["properties"]["title"].getValue();

			mEarth.addQuake( coords[0].getValue<float>(), coords[1].getValue<float>(), mag, title );
		}
	}
	catch( ci::Exception &exc ) {
		console() << "Failed to parse json, what: " << exc.what() << std::endl;
	}

	// Test to see if quakes show up in the right spot:
	// mEarth.addQuake( 37.7833f, -122.4167f, 8.6f, "San Francisco" );

	mEarth.setQuakeLocTips();
}

CINDER_APP( EarthquakeApp, RendererGl( RendererGl::Options().msaa( 16 ) ), &EarthquakeApp::prepareSettings )