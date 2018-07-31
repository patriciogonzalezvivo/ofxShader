#pragma once

#include "ofxShader.h"

struct Uniform {
    float   value[4];
    int     size;
    bool    bInt = false;
};

typedef map<string, Uniform> UniformList;
typedef map<string, ofTexture*> TextureList;

class ofxShaderSandbox : public ofBaseDraws, public ofBaseHasTexture {
public:
	ofxShaderSandbox();
	virtual ~ofxShaderSandbox();

    virtual void	allocate(int _width, int _height);
	
	virtual bool	load(const string &_frag);

	// set a single uniform value
	virtual void    setUniform1i(const string &_name, int v1);
	virtual void    setUniform2i(const string &_name, int v1, int v2);
	virtual void    setUniform3i(const string &_name, int v1, int v2, int v3);
	virtual void    setUniform4i(const string &_name, int v1, int v2, int v3, int v4);

	virtual void    setUniform1f(const string &_name, float v1);
	virtual void    setUniform2f(const string &_name, float v1, float v2);
	virtual void    setUniform3f(const string &_name, float v1, float v2, float v3);
	virtual void    setUniform4f(const string &_name, float v1, float v2, float v3, float v4);

	virtual void    setUniform2f(const string &_name, const glm::vec2 & v);
	virtual void    setUniform3f(const string &_name, const glm::vec3 & v);
	virtual void    setUniform4f(const string &_name, const glm::vec4 & v);
	virtual void    setUniform4f(const string &_name, const ofFloatColor & v);

	// set a texture reference
	virtual void    setUniformTexture(const string &_name, const ofBaseHasTexture& img, int textureLocation);
	virtual void    setUniformTexture(const string &_name, const ofTexture& img, int textureLocation);

	virtual void	render();

	virtual int 	getTotalBuffers();

	// ofBaseDraws
	virtual float	getHeight() const { return m_height; };
	virtual float 	getWidth() const { return m_width; };

	virtual void 	draw(float _x, float _y) const { draw(_x, _y, getWidth(), getHeight()); }
	virtual void 	draw(float _x, float _y, float _w, float _h) const;

	// ofBaseHasTexture
	virtual ofTexture& getTexture();
	virtual const ofTexture& getTexture() const;

	virtual void 	setUseTexture(bool bUseTex) {};
	virtual bool 	isUsingTexture() const {return true;}


	virtual void 	_reload(bool &_args);
protected:
	virtual void 	_updateBuffers();

private:
    ofFbo           m_fbo;
    ofxShader		m_shader;

    vector<string>	m_defines;
    UniformList   	m_uniforms;
    TextureList		m_textures;

    vector<ofFbo>		m_buffers;
    vector<ofxShader>	m_buffers_shaders;

    int				m_width;
    int				m_height;
};