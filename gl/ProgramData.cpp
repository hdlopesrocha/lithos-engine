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

ProgramData::ProgramData() {
    ubo= 0;
    glGenBuffers(1, &ubo);  // Generate the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);  // Bind it as a UBO
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), NULL, GL_DYNAMIC_DRAW);  // Allocate memory
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);  // Bind it to binding point 0
    glBindBuffer(GL_UNIFORM_BUFFER, 0);  // Unbind
}


void ProgramData::uniform(UniformBlock * block){
    //UniformBlock::print(block);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(UniformBlock), 
                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr) {
        memcpy(ptr, block, sizeof(UniformBlock));
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
}
