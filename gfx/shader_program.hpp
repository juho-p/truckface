#pragma once

class ShaderProgram : public NoCopy {
    std::vector<GLuint> shaders;
    GLuint program;

public:
    ShaderProgram() {
        this->program = glCreateProgram();
    }
    ~ShaderProgram() {
        for (auto shader : this->shaders) {
            glDeleteShader(shader);
        }
        glDeleteProgram(this->program);
    }

    bool add_shader_from_source(GLuint type, const char * src) {
        const GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint status = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == 0) { // compile fail
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> buffer(len + 1, 0);
            char* const cstr = &buffer.front();
            glGetShaderInfoLog(shader, len, nullptr, cstr);
            cerr << "Shader compilation failed!\n";
            cerr << "Compilation log:\n";
            cerr << cstr << endl;
            cerr << "========================================" << endl;
            glDeleteShader(shader);
            return false;
        }
        this->shaders.push_back(shader);
        glAttachShader(this->program, shader);
        return true;
    }

    bool link() {
        glLinkProgram(this->program);

        GLint status = 0;
        glGetProgramiv(this->program, GL_LINK_STATUS, &status);

        if (status == 0) { // link failure
            GLint len;
            glGetShaderiv(this->program, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> buffer(len + 1, 0);
            char* const cstr = &buffer.front();
            glGetProgramInfoLog(this->program, len, nullptr, cstr);
            cerr << "Shader linking failed!\n";
            cerr << "Linker log:\n";
            cerr << cstr << endl;
            cerr << "========================================" << endl;
            return false;
        }
        return true;
    }

    void activate() {
        glUseProgram(this->program);
    }

    void bind_attribute_location(const char* name, GLuint location) {
        glBindAttribLocation(this->program, location, name);
    }

    GLint get_uniform_location(const char* name) {
        return glGetUniformLocation(this->program, name);
    }
};
