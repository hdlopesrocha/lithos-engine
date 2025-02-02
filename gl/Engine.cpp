
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

GLFWwindow * LithosApplication::getWindow() {
    return window;
}

void LithosApplication::mainLoop() {
    while(alive) {
        // Main loop
        if (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Get current time
            double currentTime = glfwGetTime();
            // Calculate delta time (time since last frame)
            double deltaTime = currentTime - lastFrameTime;
            // Update last frame time to the current time
            lastFrameTime = currentTime;
            update(deltaTime);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw3d();
            draw2d();
            glfwSwapBuffers(window);
        }else {
            alive = false;
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

RenderBuffer createRenderFrameBuffer(int width, int height) {
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    


    glGenTextures(1, &buffer.colorTexture);
    glBindTexture(GL_TEXTURE_2D, buffer.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenTextures(1, &buffer.depthTexture);
    glBindTexture(GL_TEXTURE_2D, buffer.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

// Set the border color (default is black, but you can change it)
GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // White border
glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers(1, &buffer.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthTexture, 0);




    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "createRenderFrameBuffer error!" << std::endl;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
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

GLuint createTextureArray(int width, int height, int layers) {
    GLuint texArray;
    glGenTextures(1, &texArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);


    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);


    int mipLevels = 1 + floor(log2(glm::max(width, height)));
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, width, height, 3);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return texArray;
}

RenderBuffer createMultiLayerRenderFrameBuffer(int width, int height, int layers) {
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
   
    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.colorTexture = createTextureArray(width, height, layers);

    glGenFramebuffers(1, &buffer.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    // Attach the entire texture array (for layered rendering)
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.colorTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
   
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
    return buffer;
}


RenderBuffer createDepthFrameBuffer(int width, int height) {
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
    
    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    
    // Create framebuffer
    glGenFramebuffers(1, &buffer.frameBuffer);        
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    // Create frameTexture
    glGenTextures(1, &buffer.depthTexture);
    glBindTexture(GL_TEXTURE_2D, buffer.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Configure framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.depthTexture,0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);

    return buffer;
}

GLenum channelsToFormat(int channels) {
    if (channels == 1)
        return GL_RED;
    else if (channels == 3)
        return GL_RGB;
    else if (channels == 4)
        return GL_RGBA;
    return GL_DEPTH_COMPONENT;
}

TextureArray LithosApplication::loadTextureArray(const std::string& color, const std::string& normal, const std::string& bump) {

    // Generate a texture object
    TextureArray textureArray;
    glGenTextures(1, &textureArray);
    glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

    // Load the image
    int width, height;


    std::string filenames[3] = { color, normal, bump} ;
    unsigned char* datas[3] = {NULL, NULL, NULL};
    int channels[3];

    for(int i = 0; i < 3 ; ++i) {
        std::string filename = filenames[i];

        if(filename.size()){
            std::cout << "Loading " << filename << std::endl;
            datas[i] = stbi_load(filename.c_str(), &width, &height, &channels[i], 0);
            if (!datas[i]) {
                std::cerr << "Failed to load texture: " << color << std::endl;
                return textureArray;
            }
        }
    }
    int mipLevels = 1 + floor(log2(glm::max(width, height)));
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, width, height, 3);

    for(int i = 0; i < 3 ; ++i) {
        if (datas[i]) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, channelsToFormat(channels[i]), GL_UNSIGNED_BYTE, datas[i]);
        }
    }

    // Upload the texture to OpenGL
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Free image data and unbind texture

    for(int i = 0; i < 3 ; ++i) {
        if (datas[i]) {
            stbi_image_free(datas[i]);
        }
    }


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);    
	return textureArray;
}


TextureImage LithosApplication::loadTextureImage(const std::string& filename) {
    std::cout << "Loading " << filename << std::endl;
	TextureImage image;

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