
#include "ofxSmartShader.h"

ofxSmartShader::ofxSmartShader() {
	bWatchingFiles = false;
}

ofxSmartShader::~ofxSmartShader() {
	disableWatchFiles();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxSmartShader::load(string _shaderName ) {
	return load( _shaderName + ".vert", _shaderName + ".frag", _shaderName + ".geom" );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxSmartShader::load(string _vertName, string _fragName, string _geomName) {
	unload();
	
    ofShader::setGeometryOutputCount(geometryOutputCount);
    ofShader::setGeometryInputType(geometryInputType);
    ofShader::setGeometryOutputType(geometryOutputType);

	// hackety hack, clear errors or shader will fail to compile
	GLuint err = glGetError();
	
	lastTimeCheckMillis = ofGetElapsedTimeMillis();
	setMillisBetweenFileCheck( 2 * 1000 );
	enableWatchFiles();
	
	loadShaderNextFrame = false;
	
	vertexShaderFilename = _vertName;
	fragmentShaderFilename = _fragName;
	geometryShaderFilename = _geomName;
	
	vertexShaderFile.clear();
	fragmentShaderFile.clear();
	geometryShaderFile.clear();
	
	vertexShaderFile   = ofFile( ofToDataPath( vertexShaderFilename ) );
	fragmentShaderFile = ofFile( ofToDataPath( fragmentShaderFilename ) );
	geometryShaderFile = ofFile( ofToDataPath( geometryShaderFilename ) );
	
	ofBuffer vertexShaderBuffer = ofBufferFromFile( ofToDataPath( vertexShaderFilename ) );
	ofBuffer fragmentShaderBuffer = ofBufferFromFile( ofToDataPath( fragmentShaderFilename ) );
	ofBuffer geometryShaderBuffer = ofBufferFromFile( ofToDataPath( geometryShaderFilename ) );
	
	fileChangedTimes.clear();
	fileChangedTimes.push_back( getLastModified( vertexShaderFile ) );
	fileChangedTimes.push_back( getLastModified( fragmentShaderFile ) );
	fileChangedTimes.push_back( getLastModified( geometryShaderFile ) );
	
	if ( vertexShaderBuffer.size() > 0 ) {
		setupShaderFromSource(GL_VERTEX_SHADER, vertexShaderBuffer.getText() );
	}

	if ( fragmentShaderBuffer.size() > 0 ) {
		setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShaderBuffer.getText());
	}

	#ifndef TARGET_OPENGLES
	if ( geometryShaderBuffer.size() > 0 ) {
		setupShaderFromSource(GL_GEOMETRY_SHADER_EXT, geometryShaderBuffer.getText());
	}
	#endif

	bindDefaults();
	
	return linkProgram();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxSmartShader::_update(ofEventArgs &e) {
	if ( loadShaderNextFrame ) {
		reloadShaders();
		loadShaderNextFrame = false;
	}
	
	int currTime = ofGetElapsedTimeMillis();
	
	if (((currTime - lastTimeCheckMillis) > millisBetweenFileCheck) &&
	   	!loadShaderNextFrame ) {
		if ( filesChanged() ) {
			loadShaderNextFrame = true;
		}
		
		lastTimeCheckMillis = currTime;
	}
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxSmartShader::reloadShaders() {
	return load( vertexShaderFilename,  fragmentShaderFilename, geometryShaderFilename );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxSmartShader::enableWatchFiles() {
	if (!bWatchingFiles) {
		ofAddListener(ofEvents().update, this, &ofxSmartShader::_update );
		bWatchingFiles = true;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxSmartShader::disableWatchFiles() {
	if (bWatchingFiles) {
		ofRemoveListener(ofEvents().update, this, &ofxSmartShader::_update );
		bWatchingFiles = false;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxSmartShader::filesChanged() {
	bool fileChanged = false;
	
	if ( vertexShaderFile.exists() ) {
		std::time_t vertexShaderFileLastChangeTime = getLastModified( vertexShaderFile );
		if( vertexShaderFileLastChangeTime != fileChangedTimes.at(0) )
		{
			fileChangedTimes.at(0) = vertexShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	if ( fragmentShaderFile.exists() ) {
		std::time_t fragmentShaderFileLastChangeTime = getLastModified( fragmentShaderFile );
		if ( fragmentShaderFileLastChangeTime != fileChangedTimes.at(1) ) {
			fileChangedTimes.at(1) = fragmentShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	
	if ( geometryShaderFile.exists() ) {
		std::time_t geometryShaderFileLastChangeTime = getLastModified( geometryShaderFile );
		if ( geometryShaderFileLastChangeTime != fileChangedTimes.at(2) ) {
			fileChangedTimes.at(2) = geometryShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	return fileChanged;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
std::time_t ofxSmartShader::getLastModified( ofFile& _file ) {
	if ( _file.exists() ) {
        return std::filesystem::last_write_time(_file.path());
	}
	else {
		return 0;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxSmartShader::setMillisBetweenFileCheck( int _millis ) {
	millisBetweenFileCheck = _millis;
}

//--------------------------------------------------------------
void ofxSmartShader::setGeometryInputType( GLenum _type ) {
    ofShader::setGeometryInputType(_type);
    geometryInputType = _type;
}

//--------------------------------------------------------------
void ofxSmartShader::setGeometryOutputType( GLenum _type ) {
    ofShader::setGeometryOutputType(_type);
    geometryOutputType = _type;
}

//--------------------------------------------------------------
void ofxSmartShader::setGeometryOutputCount( int _count ) {
    ofShader::setGeometryOutputCount(_count);
    geometryOutputCount = _count;
}
