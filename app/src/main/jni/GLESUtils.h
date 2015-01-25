#ifndef __PictoPop__GLESUtils__
#define __PictoPop__GLESUtils__

#include <GLES2/gl2.h>

const GLuint createProgram(const char* VertexSource, const char* FragmentSource);
const GLuint loadShader(GLenum shaderType, const char* pSource);
const GLuint createSimpleTexture2D(GLuint _textureid, GLubyte* pixels, GLuint textureGLtype, int width, int height, int channels);
void printGLString(const char *name, GLenum s);
void checkGlError(const char* op);

#endif