#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    //    ofSetupOpenGL(1024,768,OF_WINDOW);            // <-------- setup the GL context
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    
#ifdef TARGET_OPENGLES
    ofGLESWindowSettings settings;
    settings.setGLESVersion(2);
#else
    ofGLWindowSettings settings;
    settings.setGLVersion(3, 2);  // Programmable pipeline
#endif
    ofCreateWindow(settings);
    ofRunApp(new ofApp());

}
