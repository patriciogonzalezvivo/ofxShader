
#include "ofxSmartShader.h"

// Research https://github.com/cinder/Cinder/blob/master/src/cinder/gl/ShaderPreprocessor.cpp

ofxSmartShader::ofxSmartShader() {
	m_bWatchingFiles = false;
    m_bAutoVersionConversion = true;
}

ofxSmartShader::~ofxSmartShader() {
	disableWatchFiles();
}

void ofxSmartShader::addIncludeFolder(std::string &_folder) {
    m_includeFolders.push_back(_folder);
}

void ofxSmartShader::addDefineKeyword(std::string &_define) {
    m_defines.push_back(_define);
}

void ofxSmartShader::delDefineKeyword(std::string &_define) {
    for (int i = m_defines.size() - 1; i >= 0 ; i++) {
        if ( m_defines[i] == _define ) {
            m_defines.erase(m_defines.begin() + i);
        }
    }
}

std::string getAbsPath(const std::string& _str) {
    std::string abs_path = realpath(_str.c_str(), NULL);
    std::size_t found = abs_path.find_last_of("\\/");
    if (found){
        return abs_path.substr(0, found);
    }
    else {
        return "";
    }
}

std::string urlResolve(const std::string& _path, const std::string& _pwd, const std::vector<std::string> _includeFolders) {
    std::string url = _pwd + '/' + _path;
    if (ofFile(url).exists()) {
        return url;
    }
    else {
        for (uint i = 0; i < _includeFolders.size(); i++) {
            std::string new_path = _includeFolders[i] + "/" + _path;
            if (ofFile(new_path).exists()) {
                return new_path;
            }
        }
        return _path;
    }
}

bool loadFromPath(const std::string& _path, std::string* _into, const std::vector<std::string> _includeFolders) {
    std::ifstream file;
    std::string buffer;
    
    file.open(_path.c_str());
    if(!file.is_open()) return false;
    std::string original_path = getAbsPath(_path);
    
    while(!file.eof()) {
        getline(file, buffer);
        if (buffer.find("#include ") == 0 || buffer.find("#pragma include ") == 0){
            unsigned begin = buffer.find_first_of("\"");
            unsigned end = buffer.find_last_of("\"");
            if (begin != end) {
                std::string file_name = buffer.substr(begin+1,end-begin-1);
                file_name = urlResolve(file_name, original_path, _includeFolders);
                std::string newBuffer;
                if(loadFromPath(file_name, &newBuffer, _includeFolders)){
                    (*_into) += "\n" + newBuffer + "\n";
                }
                else {
                    std::cout << file_name << " not found at " << original_path << std::endl;
                }
            }
        } else {
            (*_into) += buffer + "\n";
        }
    }
    
    file.close();
    return true;
}

bool haveExt(const std::string& file, const std::string& ext){
    return file.find("."+ext) != std::string::npos;
}

bool ofxSmartShader::load(string _shaderName ) {
	return load( _shaderName + ".vert", _shaderName + ".frag", _shaderName + ".geom" );
}

bool ofxSmartShader::load(string _vertName, string _fragName, string _geomName) {
	unload();
	
    ofShader::setGeometryOutputCount( m_geometryOutputCount );
    ofShader::setGeometryInputType( m_geometryInputType );
    ofShader::setGeometryOutputType( m_geometryOutputType );

	// hackety hack, clear errors or shader will fail to compile
	GLuint err = glGetError();
	
	m_lastTimeCheckMillis = ofGetElapsedTimeMillis();
	setMillisBetweenFileCheck( 2 * 1000 );
	enableWatchFiles();
	
	m_loadShaderNextFrame = false;
	
    // Update filenames
	m_vertexShaderFilename = _vertName;
	m_fragmentShaderFilename = _fragName;
	m_geometryShaderFilename = _geomName;
	
    // Update last change time
	m_vertexShaderFile.clear();
	m_fragmentShaderFile.clear();
	m_geometryShaderFile.clear();
	
	m_vertexShaderFile   = ofFile( ofToDataPath( m_vertexShaderFilename ) );
	m_fragmentShaderFile = ofFile( ofToDataPath( m_fragmentShaderFilename ) );
	m_geometryShaderFile = ofFile( ofToDataPath( m_geometryShaderFilename ) );
    
    m_fileChangedTimes.clear();
    m_fileChangedTimes.push_back( getLastModified( m_vertexShaderFile ) );
    m_fileChangedTimes.push_back( getLastModified( m_fragmentShaderFile ) );
    m_fileChangedTimes.push_back( getLastModified( m_geometryShaderFile ) );
    
    // Update Sources
    std::string vertexSrc = "";
    std::string fragmentSrc = "";
    std::string geometrySrc = "";
    
    // 1. Load shaders resolving #include to nested sources
    loadFromPath( ofToDataPath( m_vertexShaderFilename ), &vertexSrc, m_includeFolders );
    loadFromPath( ofToDataPath( m_fragmentShaderFilename ), &fragmentSrc, m_includeFolders );
    #ifndef TARGET_OPENGLES
    loadFromPath( ofToDataPath( m_geometryShaderFilename ), &geometrySrc, m_includeFolders );
    #endif
    
    // 2. Add defines
    std::string defines_header = "";
    for (unsigned int i = 0; i < m_defines.size(); i++) {
        defines_header += "#define " + m_defines[i] + "\n";
    }
    defines_header += "#line 0 \n";
    
    // 2. Check active default uniforms
    m_time = false;
    m_date = false;
    m_delta = false;
    m_mouse = false;
    m_resolution = false;
    _checkActiveUniforms(vertexSrc);
    _checkActiveUniforms(fragmentSrc);
    _checkActiveUniforms(geometrySrc);
    
    // 3. Add defines
    string version_vert_header = "";
    string version_frag_header = "";
    string version_geom_header = "";
    
    if (m_bAutoVersionConversion) {
#ifdef TARGET_OPENGLES
        string version100 = "#ifdef GL_ES\n\
precision highp float;\n\
#endif\n";
        version_vert_header = version100;
        version_frag_header = version100;
#else
        if (ofIsGLProgrammableRenderer()) {
            version_vert_header = "#version 150\n\
#define attribute in\n\
#define varying out\n";
            version_frag_header = "#version 150\n\
#define varying in\n\
#define gl_FragColor fragColor\n\
out vec4 fragColor;\n";
            version_geom_header = "#version 150\n";
        }
        else {
            version_vert_header = "#version 120\n";
            version_frag_header = "#version 120\n";
            version_geom_header = "#version 120\n";
        }
#endif
    }
	
	if ( vertexSrc.size() > 0 ) {
		setupShaderFromSource( GL_VERTEX_SHADER, version_vert_header + defines_header + vertexSrc );
	}

	if ( fragmentSrc.size() > 0 ) {
		setupShaderFromSource( GL_FRAGMENT_SHADER, version_frag_header + defines_header + fragmentSrc );
	}

	#ifndef TARGET_OPENGLES
	if ( geometrySrc.size() > 0 ) {
		setupShaderFromSource( GL_GEOMETRY_SHADER_EXT, version_geom_header + defines_header + geometrySrc );
	}
	#endif

	bindDefaults();
	
	return linkProgram();
}

bool find_id(const std::string& program, const char* id) {
    return std::strstr(program.c_str(), id) != 0;
}

void ofxSmartShader::_checkActiveUniforms(std::string &_source) {
    if (!m_time)
        m_time = find_id(_source, "u_time");
    if (!m_date)
        m_date = find_id(_source, "u_date");
    if (!m_delta)
        m_delta = find_id(_source, "u_delta");
    if (!m_mouse)
        m_mouse = find_id(_source, "u_mouse");
    if (!m_resolution)
        m_resolution = find_id(_source, "u_resolution");
}

void ofxSmartShader::begin() {
    ofShader::begin();
    
    if (m_time)
        setUniform1f("u_time", ofGetElapsedTimef());
    
    if (m_date)
        setUniform4f("u_date", ofGetYear(), ofGetMonth(), ofGetDay(), ofGetSeconds());
    
    if (m_delta) {
        double now = ofGetElapsedTimef();
        setUniform1f("u_delta", now - m_lastFrame);
        m_lastFrame = now;
    }
    
    if (m_mouse)
        setUniform2f("u_mouse", ofGetMouseX(), ofGetMouseY());
    
    if (m_resolution)
        setUniform2f("u_resolution", ofGetWidth(), ofGetHeight());
}

void ofxSmartShader::_update (ofEventArgs &e) {
	if ( m_loadShaderNextFrame ) {
		reloadShaders();
		m_loadShaderNextFrame = false;
	}
	
	int currTime = ofGetElapsedTimeMillis();
	
	if (((currTime - m_lastTimeCheckMillis) > m_millisBetweenFileCheck) &&
	   	!m_loadShaderNextFrame ) {
		if ( filesChanged() ) {
			m_loadShaderNextFrame = true;
		}
		
		m_lastTimeCheckMillis = currTime;
	}
}

bool ofxSmartShader::reloadShaders() {
	return load( m_vertexShaderFilename,  m_fragmentShaderFilename, m_geometryShaderFilename );
}

void ofxSmartShader::enableAutoVersionConversion() {
    if (!m_bAutoVersionConversion) {
        m_loadShaderNextFrame = true;
    }
    m_bAutoVersionConversion = true;
}

void ofxSmartShader::disableAutoVersionConversion() {
    if (m_bAutoVersionConversion) {
        m_loadShaderNextFrame = true;
    }
    m_bAutoVersionConversion = false;
}

void ofxSmartShader::enableWatchFiles() {
	if (!m_bWatchingFiles) {
		ofAddListener( ofEvents().update, this, &ofxSmartShader::_update );
		m_bWatchingFiles = true;
	}
}

void ofxSmartShader::disableWatchFiles() {
	if (m_bWatchingFiles) {
		ofRemoveListener( ofEvents().update, this, &ofxSmartShader::_update );
		m_bWatchingFiles = false;
	}
}

bool ofxSmartShader::filesChanged() {
	bool fileChanged = false;
	
	if ( m_vertexShaderFile.exists() ) {
		std::time_t vertexShaderFileLastChangeTime = getLastModified( m_vertexShaderFile );
		if ( vertexShaderFileLastChangeTime != m_fileChangedTimes.at(0) ) {
			m_fileChangedTimes.at(0) = vertexShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	if ( m_fragmentShaderFile.exists() ) {
		std::time_t fragmentShaderFileLastChangeTime = getLastModified( m_fragmentShaderFile );
		if ( fragmentShaderFileLastChangeTime != m_fileChangedTimes.at(1) ) {
			m_fileChangedTimes.at(1) = fragmentShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	
	if ( m_geometryShaderFile.exists() ) {
		std::time_t geometryShaderFileLastChangeTime = getLastModified( m_geometryShaderFile );
		if ( geometryShaderFileLastChangeTime != m_fileChangedTimes.at(2) ) {
			m_fileChangedTimes.at(2) = geometryShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	return fileChanged;
}

std::time_t ofxSmartShader::getLastModified( ofFile& _file ) {
	if ( _file.exists() ) {
        return std::filesystem::last_write_time(_file.path());
	}
	else {
		return 0;
	}
}

void ofxSmartShader::setMillisBetweenFileCheck( int _millis ) {
	m_millisBetweenFileCheck = _millis;
}

void ofxSmartShader::setGeometryInputType( GLenum _type ) {
    ofShader::setGeometryInputType(_type);
    m_geometryInputType = _type;
}

void ofxSmartShader::setGeometryOutputType( GLenum _type ) {
    ofShader::setGeometryOutputType(_type);
    m_geometryOutputType = _type;
}

void ofxSmartShader::setGeometryOutputCount( int _count ) {
    ofShader::setGeometryOutputCount(_count);
    m_geometryOutputCount = _count;
}
