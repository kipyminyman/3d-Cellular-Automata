#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * Returns the contents of a file as a string.
 * @param  fname Name of file to load
 * @return       string of contents of the file
 */
std::string loadFile(const char *fname);

/**
 * Creates, compiles, and returns shader of type `shader_type` with source from
 * file `fname`.
 *
 * @param  shader_type Specifies the type of shader to be created.
 * @param  fname       Name of file to load. Loaded bytes will become the shader
 *                     source.
 * @return             OpenGL reference to shader object.
 */
GLuint createShader(GLenum shader_type, const char *fname);

/**
 * Prints the status of the passed shader. If an error occured during
 * compilation, the error will be printed to stderr.
 *
 * @param shader Shader to check the status of.
 */
void checkShaderStatus(GLuint shader);

/**
 * Creates, links, sets as active, and returns OpenGL reference to a shader
 * program.
 *
 * @param  shaders A list of shaders to attach to the program.
 * @param  count   The number of shaders in the list
 * @return         OpenGL reference to shader program
 */
GLuint createShaderProgram(GLuint *shaders, int count);

/**
 * Sets the uniform matrix at uni_location to a glm perspective matrix with the
 * parameters following uni_location being the same as a call to
 * glm::perspective(...).
 * @param uni_location Location of the uniform obtained by a call like
 *                     glGetUniformLocation()
 * @param fov          [description]
 * @param ratio        [description]
 * @param near         [description]
 * @param far          [description]
 */
void setPerspective(GLint uni_location, float fov, float ratio, float near, float far);

/**
 * Given that uni_location is the attribute associated with the view matrix in
 * the vertex shader, then both functions will set that matrix to be one that
 * looks from the from_ parameters and to the at_ parameters or in the dir with
 * pitch, in the other function.
 *
 * @param uni_location [description]
 * @param from_x       [description]
 * @param from_y       [description]
 * @param from_z       [description]
 * @param at_x         [description]
 * @param at_y         [description]
 * @param at_z         [description]
 */
void setView(GLint uni_location, float from_x, float from_y, float from_z, float at_x, float at_y, float at_z);
void setView(GLint uni_location, float from_x, float from_y, float from_z, float dir, float pitch);

// TODO: add a struct for attributes instead of passing all these things
//       It feels like a good idea, but idk if it's a good idea. In the end, it
//       will only impact syntax, but still...

/**
 * Binds the buffer, gets (and ultimately returns) the attribute `name` in
 * `shader_program`. Then, the attribute is set up according to the provided
 * specs. See man pages on glVertexAttribPointer for more details.
 *
 * NOTE: if `type` is GL_FLOAT or `normalized` is GL_TRUE, then
 * glVertexAttribPointer will be used. If `type` is GL_DOUBLE, then
 * glVertexAttribLPointer is used. Otherwise, glVertexAttribIPointer is used.
 *
 * NOTE: It is assumed that the passed buffer is to be bound to GL_ARRAY_BUFFER,
 * as this function is specifically used to set up vertex attributes.
 *
 * @param  buffer     Specifies the name of a buffer object.
 * @param  program    Specifies the program object to be queried.
 * @param  name       Points to a null terminated string containing the name of
 *                    the attribute variable whose location is to be queried.
 * @param  size       Specifies the number of components per generic vertex
 *                    attribute. Must be 1, 2, 3, 4. Additionally, the symbolic
 *                    constant GL_BGRA is accepted by glVertexAttribPointer. The
 *                    initial value is 4.
 * @param  type       Specifies the data type of each component in the array.
 *                    See opengl docs on glVertexAttribPointer for more details.
 * @param  stride     Specifies the byte offset between consecutive generic
 *                    vertex attributes. If stride is 0, the generic vertex
 *                    attributes are understood to be tightly packed in the array.
 * @param  pointer    Specifies a offset of the first component of the first
 *                    generic vertex attribute in the array in the data store of
 *                    the buffer currently bound to the GL_ARRAY_BUFFER target.
 *                    To put it simply, this is the initial offset of the buffer.
 * @param  normalized For glVertexAttribPointer, specifies whether fixed-point
 *                    data values should be normalized (GL_TRUE) or converted
 *                    directly as fixed-point values (GL_FALSE) when they are
 *                    accessed. Note: this is only used when you want to use
 *                    glVertexAttribPointer.
 * @return            Same as the return of `glGetAttribLocation(program, name)`
 *                    Returns the location of an attribute variable.
 */
GLint setupVertexAttribute(GLuint buffer, GLuint program, const GLchar *name,
                           GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, GLboolean normalized = GL_FALSE);

// other stuff //

/**
 * Creates, binds, allocates, and returns a buffer (e.g. a VBO or an element
 * buffer object) of size `size` (in bytes) to target `target` with data `*data`
 * for usage `usage`. After this call, the newly created buffer is now bound.
 *
 * @param  target Specifies the target to which the buffer object is bound. See
 *                OpenGL documentation on `glBindBuffer`'s `target` for more
 *                info.
 * @param  size   Specifies the size in bytes of the buffer object's new data
 *                store.
 * @param  data   Specifies a pointer to data that will be copied into the data
 *                store for initialization, or NULL if no data is to be copied.
 * @param  usage  Specifies the expected usage pattern of the data store. The
 *                symbolic constant must be GL_STREAM_DRAW, GL_STREAM_READ,
 *                GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY,
 *                GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
 * @return        Generated buffer object's name.
 */
GLuint allocateBuffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage);



#endif
