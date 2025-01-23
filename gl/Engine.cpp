
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "gl.hpp"
double lastFrameTime = 0.0;



int LithosApplication::initWindow() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(getWidth(), getHeight(), "OpenGL Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);


    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable experimental features
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	for(int i=0; i < KEYBOARD_SIZE; ++i) {
		keyboard[i] = 0;
	}

	glfwSetKeyCallback(window, keyCallback);


    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;


    return 0;
}


void LithosApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   	auto app = reinterpret_cast<LithosApplication*>(glfwGetWindowUserPointer(window));
 	if(key > 0 && key < KEYBOARD_SIZE) {
 		app->keyboard[key] = action;
 	}
}


void LithosApplication::mainLoop() {
    while(alive) {
        // Main loop
        if (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Get current time
    double currentTime = glfwGetTime();
    // Calculate delta time (time since last frame)
    double deltaTime = currentTime - lastFrameTime;
    // Update last frame time to the current time
    lastFrameTime = currentTime;

            update(deltaTime);
            draw();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
}

RenderBuffer createRenderFrameBuffer(int width, int height) {
    RenderBuffer buffer;

    // Create framebuffer
    glGenFramebuffers(1, &buffer.frameBuffer);        
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    // Create frameTexture
    glGenTextures(1, &buffer.texture);
    glBindTexture(GL_TEXTURE_2D, buffer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Create depthbuffer
    glGenRenderbuffers(1, & buffer.depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // Configure framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depthBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.texture, 0);

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "createRenderFrameBuffer error!" << std::endl;
    }

    return buffer;
}


void LithosApplication::run() {
	if(!initWindow()) {
        // Backup original framebuffer
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
        
        renderBuffer = createRenderFrameBuffer(getWidth(), getHeight());

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);

        setup();
        mainLoop();
        clean();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void LithosApplication::close() {
    
	alive = false;
}



RenderBuffer LithosApplication::createDepthFrameBuffer(int width, int height) {
    RenderBuffer buffer;

    // Create framebuffer
    glGenFramebuffers(1, &buffer.frameBuffer);        
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    // Create frameTexture
    glGenTextures(1, &buffer.texture);
    glBindTexture(GL_TEXTURE_2D, buffer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Create depthbuffer
    //glGenRenderbuffers(1, & buffer.depthBuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, buffer.depthBuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // Configure framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.texture,0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depthBuffer);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.texture, 0);

    return buffer;
}


Image LithosApplication::loadTextureImage(const std::string& filename) {
    std::cout << "Loading " << filename << std::endl;
	Image image;

// Generate a texture object
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load the image
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }else {
        // Determine image format
        GLenum format;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        // Upload the texture to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Free image data and unbind texture
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        image = textureID;
    }
	return image;
}


int LithosApplication::getKeyboardStatus(int key) {
	return keyboard[key];
}

int LithosApplication::getWidth(){
	return WIDTH;
}

int LithosApplication::getHeight(){
	return HEIGHT;
}


std::string LithosApplication::readFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

GLuint LithosApplication::compileShader(const std::string& shaderCode, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* shaderSource = shaderCode.c_str();
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << shaderCode << std::endl;
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

GLuint LithosApplication::createShaderProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tcs, GLuint tes) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    if(tcs) {
        glAttachShader(program, tcs);
    }
    if(tes) {
        glAttachShader(program, tes);
    }
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    return program;
}