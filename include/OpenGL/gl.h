#import <OpenGLES/ES1/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

//void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
#define glRectd glRectf
#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer);
#define GL_T2F_V3F 1

#define uint GLuint

#ifdef __cplusplus
}
#endif
