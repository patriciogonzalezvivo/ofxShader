#include "ofxShaderMaterial.h"

ofxShaderMaterial::ofxShaderMaterial() : 
ofxShader(), 
m_light(nullptr),
m_cubemap(nullptr), m_cubemapTextureIndex(0),
m_camera(nullptr), m_cameraExposure(2.60417e-05), m_cameraEv100(14.9658), m_cameraAperture(16), m_cameraShutterSpeed(1.0f/125.0f), m_cameraSensitivity(100.0f) {

    m_uniformsFunctions["u_camera"] = UniformFunction( [this](ofShader* _shader) {
        if (m_camera != nullptr)
            _shader->setUniform3f("u_camera", m_camera->getPosition());
    });

    m_uniformsFunctions["u_cameraDistance"] = UniformFunction([this](ofShader* _shader) {
        if (m_camera != nullptr)
            _shader->setUniform1f("u_cameraDistance",glm::length(m_camera->getPosition()));  
    });

    m_uniformsFunctions["u_cameraNearClip"] = UniformFunction( [this](ofShader* _shader) {
        if (m_camera != nullptr)
            _shader->setUniform1f("u_cameraNearClip", m_camera->getNearClip() );
    });

    m_uniformsFunctions["u_cameraFarClip"] = UniformFunction( [this](ofShader* _shader) {
        if (m_camera != nullptr)
            _shader->setUniform1f("u_cameraFarClip", m_camera->getFarClip() );
    });

    m_uniformsFunctions["u_cameraEv100"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_cameraEv100", getCameraEv100());
    });

    m_uniformsFunctions["u_cameraExposure"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_cameraExposure", getCameraExposure());
    });

    m_uniformsFunctions["u_cameraAperture"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_cameraAperture", getCameraAperture());
    });

    m_uniformsFunctions["u_cameraShutterSpeed"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_cameraShutterSpeed", getCameraShutterSpeed());
    });

    m_uniformsFunctions["u_cameraSensitivity"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_cameraSensitivity", getCameraSensitivity());
    });

    // LIGHT UNIFORMS
    //
    m_uniformsFunctions["u_light"] = UniformFunction( [this](ofShader* _shader) {
        if (m_light != nullptr)
            _shader->setUniform3f("u_light", m_light->getPosition() );
    });

    m_uniformsFunctions["u_lightColor"] = UniformFunction( [this](ofShader* _shader) {
        if (m_light != nullptr)
            _shader->setUniform3f("u_lightColor", m_light->getDiffuseColor().r, m_light->getDiffuseColor().g, m_light->getDiffuseColor().b );
    });

    // IBL UNIFORM
    m_uniformsFunctions["u_cubeMap"] = UniformFunction( [this](ofShader* _shader) {
        if (m_cubemap != nullptr)
            setUniformTextureCube( "u_cubeMap", *m_cubemap, m_cubemapTextureIndex);
    });

    m_uniformsFunctions["u_SH"] = UniformFunction( [this](ofShader* _shader) {
        if (m_cubemap != nullptr)
            _shader->setUniform3fv("u_SH", glm::value_ptr(m_cubemap->SH[0]), 9);
    });

    m_uniformsFunctions["u_iblLuminance"] = UniformFunction( [this](ofShader* _shader) {
        _shader->setUniform1f("u_iblLuminance", 30000.0f * getCameraExposure());
    });
}

ofxShaderMaterial::~ofxShaderMaterial() {

}

void ofxShaderMaterial::setCubeMap( ofxTextureCube *_cubemap, int _textureIndex) { 
    m_cubemap = _cubemap;
    m_cubemapTextureIndex = _textureIndex;
    addDefineKeyword("CUBE_MAP u_cubeMap");
    addDefineKeyword("SH_ARRAY u_SH");
}

/** Sets this camera's exposure (default is 16, 1/125s, 100 ISO)
 * from https://github.com/google/filament/blob/master/filament/src/Exposure.cpp
 *
 * The exposure ultimately controls the scene's brightness, just like with a real camera.
 * The default values provide adequate exposure for a camera placed outdoors on a sunny day
 * with the sun at the zenith.
 *
 * @param aperture      Aperture in f-stops, clamped between 0.5 and 64.
 *                      A lower \p aperture value *increases* the exposure, leading to
 *                      a brighter scene. Realistic values are between 0.95 and 32.
 *
 * @param shutterSpeed  Shutter speed in seconds, clamped between 1/25,000 and 60.
 *                      A lower shutter speed increases the exposure. Realistic values are
 *                      between 1/8000 and 30.
 *
 * @param sensitivity   Sensitivity in ISO, clamped between 10 and 204,800.
 *                      A higher \p sensitivity increases the exposure. Realistice values are
 *                      between 50 and 25600.
 *
 * @note
 * With the default parameters, the scene must contain at least one Light of intensity
 * similar to the sun (e.g.: a 100,000 lux directional light).
 *
 * @see Light, Exposure
 */
void ofxShaderMaterial::setCameraExposure(float _aperture, float _shutterSpeed, float _sensitivity) {
    m_cameraAperture = _aperture;
    m_cameraShutterSpeed = _shutterSpeed;
    m_cameraSensitivity = _sensitivity;
    
    // With N = aperture, t = shutter speed and S = sensitivity,
    // we can compute EV100 knowing that:
    //
    // EVs = log2(N^2 / t)
    // and
    // EVs = EV100 + log2(S / 100)
    //
    // We can therefore find:
    //
    // EV100 = EVs - log2(S / 100)
    // EV100 = log2(N^2 / t) - log2(S / 100)
    // EV100 = log2((N^2 / t) * (100 / S))
    //
    // Reference: https://en.wikipedia.org/wiki/Exposure_value
    m_cameraEv100 = std::log2((_aperture * _aperture) / _shutterSpeed * 100.0f / _sensitivity);

    // This is equivalent to calling exposure(ev100(N, t, S))
    // By merging the two calls we can remove extra pow()/log2() calls
    const float e = (_aperture * _aperture) / _shutterSpeed * 100.0f / _sensitivity;
    m_cameraExposure = 1.0f / (1.2f * e);
};
