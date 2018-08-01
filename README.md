# ofxShader

This class extends from `ofShader` and helps on the process to write and mantain shaders. Handles for you:

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

**Note**: they only get passed if they are present on the code.

# ofxShaderSandbox

This class set's a `ofFbo` and a `ofxShader` instance inside to Sandbox your shader project to do: simulations or postprocessing filters. It automatically generates multiple buffers following the same pattern propose by [glslCanvas](https://github.com/patriciogonzalezvivo/glslCanvas) and [glslViewer](https://github.com/patriciogonzalezvivo/glslViewer) so all shaders should work on WebGL through [glslCanvas](https://github.com/patriciogonzalezvivo/glslCanvas), the [on line editor](http://editor.thebookofshaders.com/) (source code [here](https://github.com/patriciogonzalezvivo/glslEditor)) or nativelly on on your RaspberryPi, Ubuntu Desktops or MacOS through [glslViewer](https://github.com/patriciogonzalezvivo/glslViewer). Check the example `multiple_buffers` example to see how that works.

