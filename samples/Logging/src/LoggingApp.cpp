#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Log.h"

class LoggingApp : public App {
	
	/**
	 * This sample shows three basic logging use cases:
	 * - File Logging
	 * - Rotating File Logging
	 * - System Logging
	 *
	 * Note: only one file logging type can be enabled at a time.
	 */
	
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	/** @breif enableFileLogging will enable logging to a given file.
	 * This file will not rotate, but you can control the file appending. 
	 */
	void enableFileLogging();
	
	/** @breif enableRotatingFileLogging will enable logging to a daily rotating file.
	 * The file will rotate daily at the first logging event past midnight.
	 */
	void enableRotatingFileLogging();
	
	/** @breif enableSysLogging will enable platform specific system logging.
 	 * Currently this means syslog (OS X) and EventLog (Windows).
	 */
	void enableSysLogging();
};

void LoggingApp::setup()
{
	/**
	 * Logging is only supported for one file at a time.  Calling both
	 * enableFileLogging and enableRotatingFileLogging at the same tile 
	 * will cause the most recent call to take precedence.
	 */
	
	//enableFileLogging();
	enableRotatingFileLogging();
	enableSysLogging();
}

void LoggingApp::enableFileLogging()
{
	/**
	 * This call will append log messages to the file `cinder.log` in the folder
	 * `/tmp/logging`.  If the folder path `/tmp/logging` does not exist,
	 * it will be created for you.
	 */
	
	log::manager()->enableFileLogging( "/tmp/logging/cinder.log", true );
}

void LoggingApp::enableRotatingFileLogging()
{
	/**
	 * This call will append log messages to a rotating file in the folder `/tmp/logging`.
	 * The filename `cinder.%Y.%m.%d.log` will be evaluated by strftime.  For example
	 * the filename could evaluate to `cinder.2015.08.28.log`.
	 */
	log::manager()->enableRotatingFileLogging( "/tmp/logging", "cinder.%Y.%m.%d.log" );
}

void LoggingApp::enableSysLogging()
{
	/**
	 * This call will enable system logging, which will default to the same logging 
	 * level as console/file logging.
	 */
	log::manager()->enableSystemLogging();
	
	/**
	 * This call will set the system logging level independent of the file/console
	 * logging level.  The system logging level can not be lower than the
	 * file/console logging.
	 */
	log::manager()->setSystemLoggingLevel(log::LEVEL_WARNING);
}

void LoggingApp::mouseDown( MouseEvent event )
{
	CI_LOG_V( "VERBOSE log at " << getElapsedSeconds() << " seconds" );
	CI_LOG_D( "DEBUG log at " 	<< getElapsedSeconds() << " seconds" );
	CI_LOG_I( "INFO log at "	<< getElapsedSeconds() << " seconds" );
	CI_LOG_W( "WARNING log at " << getElapsedSeconds() << " seconds" );
	CI_LOG_E( "ERROR log at " 	<< getElapsedSeconds() << " seconds" );
	CI_LOG_F( "FATAL log at " 	<< getElapsedSeconds() << " seconds" );
}

void LoggingApp::update()
{
}

void LoggingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( LoggingApp, RendererGl )
