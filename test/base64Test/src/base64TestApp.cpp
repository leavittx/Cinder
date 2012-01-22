#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Base64.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class base64TestApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

std::string toString( Buffer b )
{
	char *temp = new char[b.getDataSize()+1];
	memcpy( temp, b.getData(), b.getDataSize() );
	temp[b.getDataSize()] = 0;
	return string( temp );	
}

void base64TestApp::setup()
{
	assert( toBase64( "any carnal pleasure." ) == "YW55IGNhcm5hbCBwbGVhc3VyZS4=" );
	assert( toBase64( "any carnal pleasure" ) == "YW55IGNhcm5hbCBwbGVhc3VyZQ==" );
	assert( toBase64( "any carnal pleasur" ) == "YW55IGNhcm5hbCBwbGVhc3Vy" );
	assert( toBase64( "any carnal pleasu" ) == "YW55IGNhcm5hbCBwbGVhc3U=" );
	assert( toBase64( "any carnal pleas" ) == "YW55IGNhcm5hbCBwbGVhcw==" );	
	
	for( int a = 0; a < 1000; ++a ) { 
		std::string test;
		for( int t = 0; t < a; ++t )
			test += 'A' + ( t % 26 );
		app::console() << "Len: " << a << std::endl;
		for( int i = 0; i < 100; ++i ) { // test word wraps
			std::string crunk = toBase64( test, i );
			Buffer b = fromBase64(crunk);
			assert( toString( b ) == test );
		}
	}
	app::console() << "Tests passed" << std::endl;
}

void base64TestApp::mouseDown( MouseEvent event )
{
}

void base64TestApp::update()
{
}

void base64TestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP_BASIC( base64TestApp, RendererGl )
