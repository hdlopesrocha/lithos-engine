#include "gl.hpp"
#include <format>
#include <iostream>

void UniformBlock::print(UniformBlock * block) {
	uint * data = (uint *) block;
	size_t size = sizeof(UniformBlock)/sizeof(uint);
	
	std::cout << "UniformBlock: ";
	for(int i=0; i < size ; ++i) {
        if(i%4 == 0) {
            std::cout << std::endl << "\t";
        }
		std::cout << "0x"<< std::setw(8) << std::setfill('0')  << std::hex << data[i] << " ";
	}
	std::cout << std::endl;
}

ProgramData::ProgramData(GLuint program) {
    this->program = program;

    ubo = 0;
    // 1️⃣ Get the uniform block index
    GLuint blockIndex = glGetUniformBlockIndex(program, "UniformBlock");

    // 2️⃣ Bind uniform block index to a binding point (e.g., 0)
    GLuint bindingPoint = 0;
    glUniformBlockBinding(program, blockIndex, bindingPoint);

    // 3️⃣ Generate and bind UBO
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    // 4️⃣ Allocate memory for the UBO (NULL for now, will update later)
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), NULL, GL_DYNAMIC_DRAW);

    // 5️⃣ Bind UBO to the same binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
}


void ProgramData::uniform(UniformBlock * block){
    //UniformBlock::print(block);
    
    // Bind UBO
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    
    // Orphan the buffer (old data is discarded)
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), NULL, GL_DYNAMIC_DRAW);

    // Upload new data
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBlock), &block);

    // Unbind UBO (optional)
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
}
