# ofxShader

This addon helps on the process to write and mantain shaders. Handles for you:

* Automatic reloading: it's based on [ofxAutoReloading](https://github.com/andreasmuller/ofxAutoReloadedShader) from [Andreas Muller](http://www.nanikawa.com/).

* Include preprocess: resolve all `#include` dependencies.

* Define keywords: easy to add/remove/toggle `#define` keywords.

* GLSL version conversion: depending if it's using ES, Rendering pipeline or the legacy OpenGL 2.2 version will add/remove the proper `#version` defines and/or presicion defines.

* automatically filled uniforms: following the conventions introduce on [The Book of Shaders](https://thebookofshaders.com/) and followed by [glslCanvas](https://github.com/patriciogonzalezvivo/glslCanvas), [glslEditor](https://github.com/patriciogonzalezvivo/glslEditor) and [glslViewer](https://github.com/patriciogonzalezvivo/glslViewer) the following uniforms are automatically handled for you:

  - `uniform float u_time;`: shader playback time (in seconds)

  - `uniform float u_delta;`: delta time between frames (in seconds)

  - `uniform vec4 u_date;`: year, month, day and seconds

  - `uniform vec2 u_resolution;`: viewport resolution (in pixels)

  - `uniform vec2 u_mouse;`: mouse pixel coords

