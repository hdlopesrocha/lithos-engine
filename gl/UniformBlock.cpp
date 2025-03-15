#include "gl.hpp"

void UniformBlock::set(uint index, uint flag, bool value) {
    if(value){
        uintData[index] |= flag;
    }else {
        uintData[index] &= ~flag;
    }
}

std::string UniformBlock::toString(UniformBlock * block) {
	uint * data = (uint *) block;
	size_t size = sizeof(UniformBlock)/sizeof(uint);
    std::stringstream ss;

	for(int i=0; i < size ; ++i) {
        if(i%4 == 0) {
            ss << std::endl;
        }
		ss << "0x"<< std::setw(8) << std::setfill('0')  << std::hex << data[i] << " ";
	}
	ss << std::endl;
    return ss.str();
}

void UniformBlock::uniform(GLuint bindingIndex, void * block, size_t size, ProgramData * data){

    // Bind the UBO to the correct binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, data->ubo);  // Binding UBO to binding point 0

    // Map the buffer for writing (invalidate previous contents)
    void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(UniformBlock), 
                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr) {
        // Copy the data into the buffer
        memcpy(ptr, block, size);

        // Unmap the buffer after writing
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }

    // Unbind the UBO (if necessary)
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    

    
}
