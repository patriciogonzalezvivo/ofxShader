#pragma once

#include "ofMain.h"

class ofxTextureCube {
public:
    ofxTextureCube();
    virtual ~ofxTextureCube();

    virtual bool    load(const string &_fileName, bool _vFlip = true);

    virtual const GLuint    getId() const { return m_id; };
    virtual int             getWidth() const { return m_width; };
    virtual int             getHeight() const { return m_height; };

    virtual void    bind();
    virtual void    unbind();

    glm::vec3       SH[9];

protected:
    int             m_width;
    int             m_height;

    GLuint          m_id;
};