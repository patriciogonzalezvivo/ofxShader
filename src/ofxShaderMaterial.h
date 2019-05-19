#pragma once

#include "ofxShader.h"

class ofxShaderMaterial : public ofxShader {
public:
    ofxShaderMaterial();
    virtual ~ofxShaderMaterial();

    void        setLight(ofLight *_light) { m_light = _light; }
    void        setSH(ofxTextureCube *_cubemap);
    void        setCubeMap(ofxTextureCube *_cubemap, int _textureIndex = 0 );
    void        setCamera(ofCamera *_camera) { m_camera = _camera; }
    void        setCameraExposure(float _aperture, float _shutterSpeed, float _sensitivity);

    const float getCameraEv100() const { return m_cameraEv100; }
    const float getCameraExposure() const { return m_cameraExposure; }
    const float getCameraAperture() const { return m_cameraAperture; }            //! returns this camera's aperture in f-stops
    const float getCameraShutterSpeed() const { return m_cameraShutterSpeed; }    //! returns this camera's shutter speed in seconds
    const float getCameraSensitivity() const { return m_cameraSensitivity; }      //! returns this camera's sensitivity in ISO

private:
    ofLight*        m_light;

    ofxTextureCube* m_cubemap;
    int             m_cubemapTextureIndex;

    ofCamera*       m_camera;
    float           m_cameraExposure; 
    float           m_cameraEv100;
    float           m_cameraAperture;
    float           m_cameraShutterSpeed;
    float           m_cameraSensitivity;
};