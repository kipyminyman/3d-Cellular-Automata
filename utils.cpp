#include "utils.h"
#include <iostream>
#include <fstream>

std::string loadFile(const char *fname)
{
    std::string ret = "";
    std::string line = "";
    std::ifstream file(fname);

    while (getline(file, line)) { ret += line + "\n"; }

    file.close();
    return ret;
}

GLuint createShader(GLenum shader_type, const char *fname)
{
    GLuint shader_id = glCreateShader(shader_type);
    std::string str = loadFile(fname);
    const char *src = str.c_str();
    glShaderSource(shader_id, 1, &src, NULL);
    glCompileShader(shader_id);
    return shader_id;
}

void checkShaderStatus(GLuint shader)
{
    GLint shader_type;
    GLint compile_status;
    GLint source_size;

    glGetShaderiv(shader, GL_SHADER_TYPE, &shader_type);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_size);

    std::cout << "============================================" << std::endl;

    std::cout << "Shader type:\t" << shader_type << " (";
    if (shader_type == GL_VERTEX_SHADER) { std::cout << "vertex shader" << ")" << std::endl; }
    else if (shader_type == GL_FRAGMENT_SHADER) { std::cout << "fragment shader" << ")" << std::endl; }
    else { std::cout << "other" << ")" << std::endl; }

    std::cout << "Shader size:\t" << source_size << std::endl;

    std::cout << "Compiled:\t";
    if (compile_status == GL_TRUE) { std::cout << "Success" << std::endl; }
    else {
        std::cout << "Failed" << std::endl << std::flush;

        char buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		std::cerr << buffer << std::endl << std::flush;
    }

    std::cout << "============================================" << std::endl;
}

GLuint createShaderProgram(GLuint *shaders, int count)
{
    GLuint shader_program = glCreateProgram();

    for (size_t i = 0; i < count; i++) {
        glAttachShader(shader_program, shaders[i]);
    }

    glLinkProgram(shader_program);
    glUseProgram(shader_program);
    return shader_program;
}

void setPerspective(GLint uni_location, float fov, float ratio, float near, float far)
{
    glm::mat4 proj = glm::perspective(fov, ratio, near, far);
    glUniformMatrix4fv(uni_location, 1, GL_FALSE, glm::value_ptr(proj));
}

void setView(GLint uni_location, float from_x, float from_y, float from_z, float at_x, float at_y, float at_z)
{
    glm::mat4 view = glm::lookAt(
	    glm::vec3(from_x, from_y, from_z), // from where...
	    glm::vec3(at_x, at_y, at_z), // look at
	    glm::vec3(0.0f, 1.0f, 0.0f) // y is up
	);
    glUniformMatrix4fv(uni_location, 1, GL_FALSE, glm::value_ptr(view));
}

void setView(GLint uni_location, float from_x, float from_y, float from_z, float dir, float pitch)
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(dir), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, glm::vec3(-from_x, -from_y, -from_z));
    glUniformMatrix4fv(uni_location, 1, GL_FALSE, glm::value_ptr(view));
}

GLint setupVertexAttribute(GLuint buffer, GLuint program, const GLchar *name, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, GLboolean normalized)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLint ret_attr = glGetAttribLocation(program, name);

    if (type == GL_FLOAT || normalized == GL_TRUE) {
        glVertexAttribPointer(ret_attr, size, type, normalized, stride, pointer);
    } else if (type == GL_DOUBLE) {
        glVertexAttribLPointer(ret_attr, size, type, stride, pointer);
    } else {
        glVertexAttribIPointer(ret_attr, size, type, stride, pointer);
    }

    glEnableVertexAttribArray(ret_attr);
    return ret_attr;
}


GLuint allocateBuffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    GLuint ret;
    glGenBuffers(1, &ret);
    glBindBuffer(target, ret);
    glBufferData(target, size, data, usage);
    return ret;
}
