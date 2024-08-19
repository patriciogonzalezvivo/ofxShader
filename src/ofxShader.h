#pragma once

#include "ofMain.h"
#include <functional>
#include "ofxTextureCube.h"
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iostream>

struct UniformFunction {
    UniformFunction() {
    }

    UniformFunction(std::function<void(ofShader*)> _assign) {
        assign = _assign;
    }

    std::function<void(ofShader*)>  assign;
    bool                            present = true;
};

typedef map<string, UniformFunction> UniformFunctionsList;

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

    void            setUniformTextureCube(const string & _name, const ofxTextureCube& _cubemap, int _textureLocation) const;
    
    void            addIncludeFolder(const string &_folder);
    
    void            addDefineKeyword(const string &_define);
    void            delDefineKeyword(const string &_define);

    string          getFilename(GLenum _type);
    
    void            begin();

    ofEvent<bool>   onLoad;
    ofEvent<bool>   onChange;
    
protected:
    void                    _update(ofEventArgs &e);

    vector<string>          m_defines;
    vector<string>          m_includeFolders;
    
    UniformFunctionsList    m_uniformsFunctions;

    double                  m_lastFrame;

private:
    std::time_t     _getLastModified(ofFile& _file) const;
    bool            _filesChanged();
    
    vector<time_t>  m_fileChangedTimes;
    
    ofFile          m_vertexShaderFile;
    ofFile          m_fragmentShaderFile;
    ofFile          m_geometryShaderFile;
    
    string          m_vertexShaderFilename;
    string          m_fragmentShaderFilename;
    string          m_geometryShaderFilename;
    
    GLenum          m_geometryInputType;
    GLenum          m_geometryOutputType;
    int             m_geometryOutputCount;
    
    int             m_lastTimeCheckMillis;
    int             m_millisBetweenFileCheck;
    int             m_version;
    
    bool            m_bAutoVersionConversion;
    bool            m_bWatchingFiles;
    bool            m_loadShaderNextFrame;
};
