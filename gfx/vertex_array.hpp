#pragma once

class VertexArray : public NoCopy {
    GLuint vao, vbo, ibo;
    unsigned int item_count;
    unsigned int item_size;

public:
    VertexArray() {
        glGenVertexArrays(1, &this->vao);
        glGenBuffers(1, &this->vbo);
        glGenBuffers(1, &this->ibo);
    }
    ~VertexArray() {
        glDeleteBuffers(1, &this->ibo);
        glDeleteBuffers(1, &this->vbo);
        glDeleteVertexArrays(1, &this->vao);
    }

    template<class Iterator>
    void set_vertex_buffer(Iterator begin, Iterator end, GLuint usage) {
        this->item_size = sizeof(*begin);
        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER, this->vao);
        glBufferData(
                GL_ARRAY_BUFFER,
                (end-begin) * sizeof(*begin),
                &(*begin),
                usage);

    }

    template<class Iterator>
    void set_index_buffer(Iterator begin, Iterator end, GLuint usage) {
        size_t count = end - begin;
        this->item_count = count;
        glBindVertexArray(this->vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
        glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                count * sizeof(*begin),
                &(*begin),
                usage);
    }

    void set_pointer(GLuint location, GLint sz, int offset) {
        auto stride = this->item_size;
        assert(stride);
        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glEnableVertexAttribArray(location);
        auto offset_ = (char*)0 + offset;
        glVertexAttribPointer(location, sz, GL_FLOAT, 0, stride, offset_);
    }

    void draw() {
        assert(this->item_count > 0);
        glBindVertexArray(this->vao);
        glDrawElements(
                GL_TRIANGLES, GLint(this->item_count),
                GL_UNSIGNED_INT, nullptr);
    }
};
