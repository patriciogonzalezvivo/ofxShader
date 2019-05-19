#pragma once

#include "ofMain.h"

class ofxTextureCube {
public:
    ofxTextureCube();
    virtual ~ofxTextureCube();

    virtual bool    load(const string &_fileName, bool _vFlip = true);
    virtual bool    generate(float _elevation = 0.785398f, float _azimuth = 0.0f, float _turbidity = 4.0f, glm::vec3 _groundAlbedo = glm::vec3(0.25), int _width = 4096 );

    virtual const GLuint    getId() const { return m_id; };
    virtual int             getWidth() const { return m_width; };
    virtual int             getHeight() const { return m_height; };

    virtual void    bind();
    virtual void    unbind();

    virtual void    draw( ofCamera &_cam );

    glm::vec3       SH[9];

protected:
    int             m_width;
    int             m_height;

    ofShader        m_shader;
    ofVboMesh       m_mesh;

    GLuint          m_id;

    bool            m_debugInit;
};