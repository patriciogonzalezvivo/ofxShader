#include "ofxTextureCube.h"
#include "face.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif 

#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390
#endif

#ifndef M_MIN
#define M_MIN(_a, _b) ((_a)<(_b)?(_a):(_b))
#endif

#define USE_BILINEAR_INTERPOLATION

extern "C" {
#include "skylight/ArHosekSkyModel.h"
}

// A few useful utilities from Filament
// https://github.com/google/filament/blob/master/tools/cmgen/src/CubemapSH.cpp
// -----------------------------------------------------------------------------------------------

ofxTextureCube::ofxTextureCube() 
: SH {  glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0),
        glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0),
        glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0) } {

    m_debugInit = false;
}

ofxTextureCube::~ofxTextureCube() {
    glDeleteTextures(1, &m_id);
}

template <typename T> 
void splitFacesVertical(T *_data, int _width, int _height, Face<T> **_faces ) {
    int faceWidth = _width / 3;
    int faceHeight = _height / 4;

    for (int i = 0; i < 6; i++) {
        _faces[i] = new Face<T>();
        _faces[i]->id = i;
        _faces[i]->data = new T[3 * faceWidth * faceHeight];
        _faces[i]->width = faceWidth;
        _faces[i]->height = faceHeight;
        _faces[i]->currentOffset = 0;
    }

    for (int l = 0; l < _height; l++) {
        int jFace = (l - (l % faceHeight)) / faceHeight;

        for (int iFace = 0; iFace < 3; iFace++) {
            Face<T> *face = NULL;
            int offset = 3 * (faceWidth * iFace + l * _width);

            //      0   1   2   i
            //  3      -Z       
            //  2      -X 
            //  1  -Y  +Z  +Y       
            //  0      +X
            //  j
            //
            if (iFace == 2 && jFace == 1) face = _faces[0]; // POS_Y
            if (iFace == 0 && jFace == 1) face = _faces[1]; // NEG_Y
            if (iFace == 1 && jFace == 0) face = _faces[2]; // POS_X
            if (iFace == 1 && jFace == 2) face = _faces[3]; // NEG_X
            if (iFace == 1 && jFace == 1) face = _faces[4]; // POS_Z
            if (iFace == 1 && jFace == 3) face = _faces[5]; // NEG_Z

            if (face) {
                // the number of components to copy
                int n = sizeof(T) * faceWidth * 3;

                std::memcpy(face->data + face->currentOffset, _data + offset, n);
                face->currentOffset += (3 * faceWidth);
            }
        }
    }
}

template <typename T> 
void splitFacesHorizontal(T *_data, int _width, int _height, Face<T> **_faces ) {
    int faceWidth = _width / 4;
    int faceHeight = _height / 3;

    for (int i = 0; i < 6; i++) {
        _faces[i] = new Face<T>();
        _faces[i]->id = i;
        _faces[i]->data = new T[3 * faceWidth * faceHeight];
        _faces[i]->width = faceWidth;
        _faces[i]->height = faceHeight;
        _faces[i]->currentOffset = 0;
    }

    for (int l = 0; l < _height; l++) {
        int jFace = (l - (l % faceHeight)) / faceHeight;

        for (int iFace = 0; iFace < 4; iFace++) {
            Face<T> *face = NULL;
            int offset = 3 * (faceWidth * iFace + l * _width);

            //      0   1   2   3 i      
            //  2      -X 
            //  1  -Y  +Z  +Y  -Z     
            //  0      +X
            //  j
            //
            if (iFace == 2 && jFace == 1) face = _faces[0]; // POS_Y
            if (iFace == 0 && jFace == 1) face = _faces[1]; // NEG_Y
            if (iFace == 1 && jFace == 0) face = _faces[2]; // POS_X
            if (iFace == 1 && jFace == 2) face = _faces[3]; // NEG_X
            if (iFace == 1 && jFace == 1) face = _faces[4]; // POS_Z
            if (iFace == 3 && jFace == 1) face = _faces[5]; // NEG_Z

            if (face) {
                // the number of components to copy
                int n = sizeof(T) * faceWidth * 3;

                std::memcpy(face->data + face->currentOffset, _data + offset, n);
                face->currentOffset += (3 * faceWidth);
            }
        }
    }
}

// From
// https://github.com/dariomanesku/cmft/blob/master/src/cmft/image.cpp#L3124
template <typename T> 
void splitFacesFromEquilateral(T *_data, unsigned int _width, unsigned int _height, Face<T> **_faces ) {
    // Alloc data.
    const uint32_t faceWidth = (_height + 1)/2;
    const uint32_t faceHeight = faceWidth;

    // Get source parameters.
    const float srcWidthMinusOne  = float(int(_width-1));
    const float srcHeightMinusOne = float(int(_height-1));
    const float invfaceWidthf = 1.0f/float(faceWidth);

    for (int i = 0; i < 6; i++) {
        _faces[i] = new Face<T>();
        _faces[i]->id = i;
        _faces[i]->data = new T[3 * faceWidth * faceHeight];
        _faces[i]->width = faceWidth;
        _faces[i]->height = faceHeight;
        _faces[i]->currentOffset = 0;

        for (uint32_t yy = 0; yy < faceHeight; ++yy) {
            T* dstRowData = &_faces[i]->data[yy * faceWidth * 3];

            for (uint32_t xx = 0; xx < faceWidth; ++xx) {
                T* dstColumnData = &dstRowData[xx * 3];

                // Cubemap (u,v) on current face.
                const float uu = 2.0f*xx*invfaceWidthf-1.0f;
                const float vv = 2.0f*yy*invfaceWidthf-1.0f;

                // Get cubemap vector (x,y,z) from (u,v,faceIdx).
                float vec[3];
                texelCoordToVec(vec, uu, vv, i);

                // Convert cubemap vector (x,y,z) to latlong (u,v).
                float xSrcf;
                float ySrcf;
                latLongFromVec(xSrcf, ySrcf, vec);

                // Convert from [0..1] to [0..(size-1)] range.
                xSrcf *= srcWidthMinusOne;
                ySrcf *= srcHeightMinusOne;

                // Sample from latlong (u,v).
                #ifdef USE_BILINEAR_INTERPOLATION
                    const uint32_t x0 = ftou(xSrcf);
                    const uint32_t y0 = ftou(ySrcf);
                    const uint32_t x1 = M_MIN(x0+1, _width-1);
                    const uint32_t y1 = M_MIN(y0+1, _height-1);

                    const T *src0 = &_data[y0 * _width * 3 + x0 * 3];
                    const T *src1 = &_data[y0 * _width * 3 + x1 * 3];
                    const T *src2 = &_data[y1 * _width * 3 + x0 * 3];
                    const T *src3 = &_data[y1 * _width * 3 + x1 * 3];

                    const float tx = xSrcf - float(int(x0));
                    const float ty = ySrcf - float(int(y0));
                    const float invTx = 1.0f - tx;
                    const float invTy = 1.0f - ty;

                    T p0[3];
                    T p1[3];
                    T p2[3];
                    T p3[3];
                    vec3Mul(p0, src0, invTx*invTy);
                    vec3Mul(p1, src1,    tx*invTy);
                    vec3Mul(p2, src2, invTx*   ty);
                    vec3Mul(p3, src3,    tx*   ty);

                    const T rr = p0[0] + p1[0] + p2[0] + p3[0];
                    const T gg = p0[1] + p1[1] + p2[1] + p3[1];
                    const T bb = p0[2] + p1[2] + p2[2] + p3[2];

                    dstColumnData[0] = rr;
                    dstColumnData[1] = gg;
                    dstColumnData[2] = bb;
                #else
                    const uint32_t xSrc = ftou(xSrcf);
                    const uint32_t ySrc = ftou(ySrcf);

                    dstColumnData[0] = _data[ySrc * _width * 3 + xSrc * 3 + 0];
                    dstColumnData[1] = _data[ySrc * _width * 3 + xSrc * 3 + 1];
                    dstColumnData[2] = _data[ySrc * _width * 3 + xSrc * 3 + 2];
                #endif
            }
        }
    }
}

bool haveExt(const std::string& _file, const std::string& _ext){
    return _file.find( "." + _ext) != std::string::npos;
}

bool ofxTextureCube::load(const std::string &_path, bool _vFlip) {

    // Init
    glGenTextures(1, &m_id);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef TARGET_OPENGLES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    int sh_samples = 0;

    // unsigned char* data = loadPixels(_path, &m_width, &m_height, RGB, false);
    ofPixels pixels;
    ofLoadImage(pixels, _path);
    m_width = pixels.getWidth();
    m_height = pixels.getHeight();
    unsigned char* data = pixels.getData();
    // cout << pixels.getBitsPerPixel() << endl;

    // LOAD FACES
    Face<unsigned char> **faces = new Face<unsigned char>*[6];

    if (m_height > m_width) {
        splitFacesVertical<unsigned char>(data, m_width, m_height, faces);

        // adjust NEG_Z face
        if (_vFlip) {
            faces[5]->flipHorizontal();
            faces[5]->flipVertical();
        }
    }
    else {
        if (m_width/2 == m_height)  {
            splitFacesFromEquilateral<unsigned char>(data, m_width, m_height, faces);
        }
        else {
            splitFacesHorizontal<unsigned char>(data, m_width, m_height, faces);
        }
    }
    
    for (int i = 0; i < 6; i++) {
        faces[i]->upload();
        sh_samples += faces[i]->calculateSH(SH);
    }

    // delete[] data;
    for(int i = 0; i < 6; ++i) {
        delete[] faces[i]->data;
        delete faces[i];
    }
    delete[] faces;

    for (int i = 0; i < 9; i++) {
        SH[i] = SH[i] * (32.0f / (float)sh_samples);
        // cout << SH[i].x << "," << SH[i].y << "," << SH[i].z << endlxw;
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return true;
}

static float angleBetween(float thetav, float phiv, float theta, float phi) {
    float cosGamma = sinf(thetav) * sinf(theta) * cosf(phi - phiv) + cosf(thetav) * cosf(theta);
    return acosf(cosGamma);
}

float clamp ( float value , float min , float max ) {
    if (value < min)
        return min;
    if (value > max)
        return max;

    return value;
}

bool ofxTextureCube::generate(float _elevation, float _azimuth, float _turbidity, glm::vec3 _groundAlbedo, int _width ) {
    // Init
    glGenTextures(1, &m_id);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef TARGET_OPENGLES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    int sh_samples = 0;

    m_width = _width;
    m_height = int(max(_width/2, 1));

    // unsigned char* data = loadPixels(_path, &m_width, &m_height, RGB, false);
    ofFloatPixels f_pixels;
    f_pixels.allocate(m_width, m_height, 3);

    // FILAMENT SKYGEN 
    // https://github.com/google/filament/blob/master/tools/skygen/src/main.cpp
    //
    float solarElevation = clamp(_elevation, 0.0f, float(M_PI_2));
    float sunTheta = float(M_PI_2 - solarElevation);
    glm::vec3 integral = glm::vec3(0.0f);
    float sunPhi = 0.0f;
    bool normalize = true;

    ArHosekSkyModelState* skyState[9] = {
        arhosek_xyz_skymodelstate_alloc_init(_turbidity, _groundAlbedo.r, solarElevation),
        arhosek_xyz_skymodelstate_alloc_init(_turbidity, _groundAlbedo.g, solarElevation),
        arhosek_xyz_skymodelstate_alloc_init(_turbidity, _groundAlbedo.b, solarElevation)
    };

    glm::mat3 XYZ_sRGB = glm::mat3(
            3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    );

    const unsigned int w = m_width;
    const unsigned int h = m_height;
    float maxSample = 0.00001f;

    for (unsigned int y = 0; y < m_height; y++) {
        float v = (y + 0.5f) / m_height;
        float theta = float(M_PI * v);

        if (theta > M_PI_2)
            continue;
            
        for (unsigned int x = 0; x < m_width; x++) {
            float u = (x + 0.5f) / m_width;
            float phi = float(-2.0 * M_PI * u + M_PI + _azimuth);

            float gamma = angleBetween(theta, phi, sunTheta, sunPhi);

            glm::vec3 sample = glm::vec3(
                arhosek_tristim_skymodel_radiance(skyState[0], theta, gamma, 0),
                arhosek_tristim_skymodel_radiance(skyState[1], theta, gamma, 1),
                arhosek_tristim_skymodel_radiance(skyState[2], theta, gamma, 2)
            );

            if (normalize) {
                sample *= float(4.0 * M_PI / 683.0);
            }

            maxSample = std::max(maxSample, sample.y);
            sample = XYZ_sRGB * sample;

            f_pixels.setColor(x, y, ofFloatColor(sample.r, sample.g, sample.b));
        }
    }

    // cleanup sky data
    arhosekskymodelstate_free(skyState[0]);
    arhosekskymodelstate_free(skyState[1]);
    arhosekskymodelstate_free(skyState[2]);

    float hdrScale = 1.0f / (normalize ? maxSample : maxSample / 16.0f);
    // if (normalize)
    //     maxSample /= float ( 4.0 * M_PI / 683.0 );

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            ofFloatColor color = f_pixels.getColor(x, y);

            if (y >= h / 2) {
                color.r = _groundAlbedo.r;
                color.g = _groundAlbedo.g;
                color.b = _groundAlbedo.b;
            }

            color *= hdrScale;

            f_pixels.setColor(x, y, ofFloatColor(color.r, color.g, color.b));
        }
    }

    // LOAD data as equilateral into cube
    float* data = f_pixels.getData();

    // LOAD FACES
    Face<float> **faces = new Face<float>*[6];
    splitFacesFromEquilateral<float>(data, m_width, m_height, faces);
    
    for (int i = 0; i < 6; i++) {
        faces[i]->upload();
        sh_samples += faces[i]->calculateSH(SH);
    }

    // delete[] data;
    for(int i = 0; i < 6; ++i) {
        delete[] faces[i]->data;
        delete faces[i];
    }
    delete[] faces;

    for (int i = 0; i < 9; i++) {
        SH[i] = SH[i] * (32.0f / (float)sh_samples);
        // cout << SH[i].x << "," << SH[i].y << "," << SH[i].z << endlxw;
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return true;
}

void ofxTextureCube::bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void ofxTextureCube::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ofxTextureCube::draw( ofCamera &_cam ) {

    if (!m_debugInit) {
        float size = 100.0;

        float vertices[] = {
            -size,  size,  size,
            -size, -size,  size,
            size, -size,  size,
            size,  size,  size,
            -size,  size, -size,
            -size, -size, -size,
            size, -size, -size,
            size,  size, -size,
        };

        unsigned int  indices[] = {
            0, 1, 2,
            0, 2, 3,
            3, 2, 6,
            3, 6, 7,
            0, 4, 7,
            0, 7, 3,
            4, 6, 7,
            4, 6, 5,
            0, 5, 4,
            0, 5, 1,
            1, 6, 5,
            1, 6, 2,
        };

        m_mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        m_mesh.addVertices(reinterpret_cast<glm::vec3*>(vertices), 8);
        m_mesh.addIndices(indices, 36);

        std::string vert;
        std::string frag;

    #ifdef TARGET_OPENGLES
//        GLSL 100
//        ----------------------------------------------
        string version100 = "#version 100\n";
        version100 += "#define texture(A,B) texture2D(A,B)\n";
        
        vert = version100;
        frag = version100;
        
        if ( !find_id(vertexSrc, "precision ") ) {
            vert += "#ifdef GL_ES\n\
precision highp float;\n\
#endif\n";
        }
        
        if ( !find_id(fragmentSrc, "precision ") ) {
            frag += "#ifdef GL_ES\n\
precision highp float;\n\
#endif\n";
        }
#else
//        GLSL 120
//        ----------------------------------------------
        if ( !ofIsGLProgrammableRenderer() ) {
            vert = "#version 120\n\
            #define texture(A,B) texture2D(A,B)\n";
            frag = "#version 120\n";
        }
        else {
//        GLSL 150
//        ----------------------------------------------
            vert = "#version 150\n\
#define attribute in\n\
#define varying out\n\
#define texture2D(A,B) texture(A,B)\n\
#define textureCube(TEX, DIRECTION, LOD) texture(TEX, DIRECTION)\n";
            frag = "#version 150\n\
#define varying in\n\
#define gl_FragColor fragColor\n\
#define texture2D(A,B) texture(A,B)\n\
#define textureCube(TEX, DIRECTION, LOD) texture(TEX, DIRECTION)\n\
out vec4 fragColor;\n";
        }
#endif

        vert += "\n\
uniform mat4    modelViewProjectionMatrix;\n\
attribute vec4  position;\n\
varying vec4    v_position;\n\
\n\
void main(void) {\n\
    v_position = position;\n\
    gl_Position = modelViewProjectionMatrix * vec4(v_position.xyz, 1.0);\n\
}";

        frag += "\n\
uniform samplerCube u_cubeMap;\n\
\n\
varying vec4    v_position;\n\
\n\
void main(void) {\n\
    vec4 reflection = texture(u_cubeMap, normalize(v_position.xyz));\n\
    gl_FragColor = reflection;\n\
}";

        m_shader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
        m_shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
        m_shader.linkProgram();

        m_debugInit = true; 
    }

    ofPushMatrix();
    // ofRotateX(90);
    m_shader.begin();
    // m_shader.setUniformMatrix4f("u_modelViewProjectionMatrix", _cam.getProjectionMatrix() * glm::toMat4(-cam.getOrientationQuat()));
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, getId());
    m_shader.setUniform1i("u_cubeMap", 1);
    glActiveTexture(GL_TEXTURE0);

    m_mesh.draw();
    m_shader.end();
    ofPopMatrix();
}