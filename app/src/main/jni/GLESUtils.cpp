#include "GLESUtils.h"
#include <android/log.h>
#include <stdlib.h>

#define  LOG_TAG    "GLESUtils"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

void printGLString(const char *name, GLenum s) {
    LOGD("GL %s = %s\n", name, (const char *)glGetString(s));
}

void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGD("after %s() glError (0x%x)\n", op, error);
    }
}

const GLuint createSimpleTexture2D(GLuint _textureid, GLubyte* pixels, GLuint textureGLtype, int width, int height, int channels) {
    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    // Bind the texture object
    glBindTexture(textureGLtype, _textureid);
    checkGlError("glBindTexture");

    GLenum format;
    switch (channels)
    {
        case 1:
            format = GL_LUMINANCE;
            break;
        case 4:
            format = GL_RGBA;
            break;
        case 3:
        default:
            format = GL_RGB;
            break;
    }

    glTexImage2D(textureGLtype, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, pixels);

    checkGlError("glTexImage2D");
    // Set the filtering mode
    glTexParameteri(textureGLtype, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri(textureGLtype, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    return _textureid;
}

const GLuint loadShader(GLenum shaderType, const char* Source)
{
    GLuint shader = glCreateShader(shaderType);
    if(shader)
    {
        glShaderSource(shader, 1, &Source, 0);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if(!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen)
            {
                char * buf = (char*) malloc (infoLen);
                if(buf)
                {
                    glGetShaderInfoLog(shader, infoLen, 0, buf);
                    LOGD("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

const GLuint createProgram(const char * pVertexSource, const char *pFragmentSource)
{
    GLuint program = 0;
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if(!vertexShader)
    {
        LOGD("vertexHader loader");
        return 0;
    }

    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if(!fragShader)
    {
        LOGD("fragShader loader failed");
        return 0;
    }

    program = glCreateProgram();
    if(program)
    {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader Vertex");
        glAttachShader(program, fragShader);
        checkGlError("glAttachShader Fragment");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if( linkStatus != GL_TRUE )
        {
            LOGD("linkStatus isn't GL_TRUE");
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if(bufLength)
            {
                char* buf = (char*) malloc(bufLength);
                if(buf)
                {
                    glGetProgramInfoLog(program, bufLength, 0, buf);
                    LOGD("Could not link program:\n%s\n", buf);
                    free(buf);
                }
                else
                    LOGD("could not allocate for bufLength");
            }
            glDeleteProgram(program);
            program = 0;
        }
        else
            LOGD("ProgramCreated %d", program);
    }
    else
    {
        LOGD("could not do glCreateProgram");
    }

    return program;
}