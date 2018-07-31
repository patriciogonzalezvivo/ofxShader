#pragma once

#include "ofMain.h"

class ofxShader : public ofShader {
public:
	ofxShader();
	virtual ~ofxShader();
	
	// override the initialisation functions
	bool            load(const string &_shaderName);
	bool            load(const string &_vertName, const string &_fragName, const string &_geomName);
	
	bool            reloadShaders();
	
	void            enableWatchFiles();
	void            disableWatchFiles();
    
    void            enableAutoVersionConversion();
    void            disableAutoVersionConversion();
		
	void            setMillisBetweenFileCheck(int _millis);
    
    void            setGeometryInputType(GLenum _type);
    void            setGeometryOutputType(GLenum _type);
    void            setGeometryOutputCount(int _count);
    void            setConversionVersion(int _version);
    
    void            addIncludeFolder(const string &_folder);
    
    void            addDefineKeyword(const string &_define);
    void            delDefineKeyword(const string &_define);

    string          getFilename(GLenum _type) const;
    
    void            begin();

    ofEvent<bool>   onLoad;
    ofEvent<bool>   onChange;
    
protected:
    void            _update(ofEventArgs &e);
    void            _checkActiveUniforms(string &_source);
	
private:
    std::time_t     getLastModified(ofFile& _file);
    bool            filesChanged();
    
    vector<string>  m_defines;
    vector<string>  m_includeFolders;
    vector<time_t>  m_fileChangedTimes;
    
    ofFile          m_vertexShaderFile;
    ofFile          m_fragmentShaderFile;
    ofFile          m_geometryShaderFile;
    
    string          m_vertexShaderFilename;
    string          m_fragmentShaderFilename;
    string          m_geometryShaderFilename;

    double          m_lastFrame;
    
    GLenum          m_geometryInputType;
    GLenum          m_geometryOutputType;
    int             m_geometryOutputCount;
    
    int             m_lastTimeCheckMillis;
    int             m_millisBetweenFileCheck;
    int             m_version;
    
    bool            m_bAutoVersionConversion;
    bool            m_bWatchingFiles;
    bool            m_loadShaderNextFrame;
    
    bool            m_time;
    bool            m_date;
    bool            m_delta;
    bool            m_mouse;
    bool            m_resolution;
};


