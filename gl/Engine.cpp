#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

#include "gl.hpp"
double lastFrameTime = 0.0;

void APIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                  GLsizei length, const GLchar* message, const void* userParam) {
    auto getSourceString = [](GLenum source) {
        switch (source) {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
            case GL_DEBUG_SOURCE_APPLICATION: return "Application";
            case GL_DEBUG_SOURCE_OTHER: return "Other";
            default: return "Unknown";
        }
    };
    auto getTypeString = [](GLenum type) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR: return "Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behavior";
            case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
            case GL_DEBUG_TYPE_MARKER: return "Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
            case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
            case GL_DEBUG_TYPE_OTHER: return "Other";
            default: return "Unknown";
        }
    };
    auto getSeverityString = [](GLenum severity) {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: return "High";
            case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
            case GL_DEBUG_SEVERITY_LOW: return "Low";
            case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
            default: return "Unknown";
        }
    };
    
    if(severity == GL_DEBUG_SEVERITY_HIGH) {
        std::cerr << "OpenGL Debug Message:\n"
              << "  Source: " << getSourceString(source) << "\n"
              << "  Type: " << getTypeString(type) << "\n"
              << "  ID: " << id << "\n"
              << "  Severity: " << getSeverityString(severity) << "\n"
              << "  Message: " << message << std::endl;
    }
}

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
    glfwSwapInterval(0);


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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglDebugCallback, nullptr);


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
    int frameCount = 0;
    float lastTime = 0.0f;
   while(alive) {
        // Main loop

        if (!glfwWindowShouldClose(window)) {
            // Get current time
            double currentTime = glfwGetTime(); // Get elapsed time in seconds
            ++frameCount;
            if (currentTime - lastTime > 1.0f)
            {
                lastTime = currentTime;
                framesPerSecond = (int)frameCount;
                frameCount = 0;
            }

            glfwPollEvents();

            // Calculate delta time (time since last frame)
            double deltaTime = currentTime - lastFrameTime;
            // Update last frame time to the current time
            lastFrameTime = currentTime;
            update(deltaTime);


            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw3d();
            draw2d(currentTime);
            glfwSwapBuffers(window);
        }else {
            alive = false;
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


RenderBuffer createRenderFrameBufferWithoutDepth(int width, int height) {
    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    
    glGenTextures(1, &buffer.colorTexture.index);
    glBindTexture(GL_TEXTURE_2D, buffer.colorTexture.index);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &buffer.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorTexture.index, 0);

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "createRenderFrameBufferWithoutDepth error!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return buffer;
}

RenderBuffer createRenderFrameBuffer(int width, int height, bool depth) {
    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.colorTexture.format = GL_RGBA8;
    glGenTextures(1, &buffer.colorTexture.index);
    glBindTexture(GL_TEXTURE_2D, buffer.colorTexture.index);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if(depth) {
        glGenTextures(1, &buffer.depthTexture.index);
        glBindTexture(GL_TEXTURE_2D, buffer.depthTexture.index);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Set the border color (default is black, but you can change it)
        GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // White border
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }
    glGenFramebuffers(1, &buffer.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorTexture.index, 0);

    if(depth) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthTexture.index, 0);
    }

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "createRenderFrameBuffer error!" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return buffer;
}


void LithosApplication::run() {
	if(!initWindow()) {
        // Backup original framebuffer
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
        

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
static long allocatedTextureArrayMemory = 0;

TextureArray createTextureArray(int width, int height, int layers, GLuint channel) {
    TextureArray texArray;
    texArray.format = channel;
    glGenTextures(1, &texArray.index);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray.index);

    // MAKE TEXTURE QUALITY LOOK WORSE, LIKE SQUARES
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    long currentMemory = 0;
    if(channel == GL_RGBA8) {
        currentMemory = 4;
    } else if(channel == GL_RGB8) {
        currentMemory = 3;
    } else if(channel == GL_RG8) {
        currentMemory = 2;
    } else if(channel == GL_R8) {
        currentMemory = 1;
    }

    currentMemory *= width * height * layers;
    allocatedTextureArrayMemory += currentMemory;
    std::cout <<  "allocatedTextureArrayMemory = " << std::to_string(allocatedTextureArrayMemory/ (1024* 1024)) << " MB" << std::endl;

    int mipLevels = 1 + floor(log2(glm::max(width, height)));
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, channel, width, height, layers);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "glTexStorage3D error: " << err << std::endl;
        throw std::runtime_error("OpenGL error in glTexStorage3D");
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

   // glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    return texArray;
}

MultiLayerRenderBuffer createMultiLayerRenderFrameBuffer(int width, int height, int layers, int attachments, bool depth, GLuint color) {
   
    MultiLayerRenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.colorTexture = createTextureArray(width, height, layers, color);

    glGenFramebuffers(1, &buffer.frameBuffer);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "glGenFramebuffers failed with error: " << err << std::endl;
    }

    if(depth) {
        glGenTextures(1, &buffer.depthTexture.index);
        glBindTexture(GL_TEXTURE_2D, buffer.depthTexture.index);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    if(depth) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthTexture.index, 0);
    }




    std::vector<GLenum> buffers;

    int attachmentCount = std::min(attachments, layers); // 'attachments' ou 'layers', o que for menor


    // Se houver múltiplos anexos, associamos as camadas de maneira mais flexível
    for (int l = 0; l < attachmentCount; ++l) {
        GLenum attachment = GL_COLOR_ATTACHMENT0 + l%attachmentCount;
        buffers.push_back(attachment);

        // Para cada anexo, associamos a camada correspondente
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, buffer.colorTexture.index, 0,l);
    }

    // Verifique se o número de buffers configurados é válido
    if (!buffers.empty()) {
        glDrawBuffers(buffers.size(), buffers.data());
    } else {
        std::cerr << "No color attachments configured!" << std::endl;
    }
    



    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return buffer;
}


RenderBuffer createDepthFrameBuffer(int width, int height) {
    RenderBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.colorTexture.index = 0;
    // Create framebuffer
    glGenFramebuffers(1, &buffer.frameBuffer);        
    glGenTextures(1, &buffer.depthTexture.index);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);

    // Create frameTexture
    glBindTexture(GL_TEXTURE_2D, buffer.depthTexture.index);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set filtering (no mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Configure framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.depthTexture.index,0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void loadTexture(TextureLayers * layers, std::initializer_list<std::string> fns, int index, bool mipMapping) {
    std::vector<std::string> textures;

    for(std::string t : fns) {
        textures.push_back(t);
    }

    for(size_t i = 0; i < textures.size() ; ++i) {
        std::string filename = textures[i];

        int width, height, channel;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channel, 0);
        std::cout << "Loading " << filename << " [index="<< std::to_string(i) <<", channels=" << std::to_string(channel) <<  "]"<< std::endl;
       
        if (!data) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            return;
        }
        glBindTexture(GL_TEXTURE_2D_ARRAY, layers->textures[i].index);
        
        // Upload image data to the specific layer
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index,  width, height, 1, channelsToFormat(channel), GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);    

        if(mipMapping) {
            // Upload the texture to OpenGL
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }else {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }


    // Free image data and unbind texture

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);    
}


TextureImage LithosApplication::loadTextureImage(const std::string& filename, bool mipmapEnabled) {
    std::cout << "Loading " << filename << std::endl;
	TextureImage image;
    image.mipmapEnabled = mipmapEnabled;
// Generate a texture object
    glGenTextures(1, &image.index);
    glBindTexture(GL_TEXTURE_2D, image.index);

    // Load the image
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }else {
        // Determine image format
        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
        image.format = format;
        image.width = width;
        image.height = height;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // Upload the texture to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        if(mipmapEnabled) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Free image data and unbind texture
        stbi_image_free(data);

    }
    glBindTexture(GL_TEXTURE_2D, 0);
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

// Function to prepend a number to each line
std::string prependNumberToEachLine(const std::string& input, int number) {
    std::stringstream ss(input);
    std::string line;
    std::string result;

    // Loop through each line
    while (std::getline(ss, line)) {
        std::ostringstream oss;
        oss << std::setw(5) << std::setfill(' ') << number;  

        result +=  oss.str() + " " + line + "\n";
        number++;  // Increment the number for each line
    }

    return result;
}

std::string trim(const std::string& str) {
    // Remove leading and trailing whitespaces
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");

    if (first == std::string::npos) {
        return ""; // Return empty string if only whitespace
    }
    return str.substr(first, (last - first + 1));
}
std::string removeExtraSpaces(const std::string& str) {
    std::istringstream iss(str);
    std::string result;
    std::string word;

    // Remove extra spaces between words
    while (iss >> word) {
        if (!result.empty()) {
            result += " ";
        }
        result += word;
    }
    return result;
}

std::string LithosApplication::readFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::cerr << "Loading " << filePath << std::endl;

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
        std::cerr << prependNumberToEachLine(shaderCode, 0) << std::endl;
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

GLuint LithosApplication::createShaderProgram(std::initializer_list<GLuint> shaders) {
    GLuint program = glCreateProgram();
    
    for (GLuint v : shaders) {
        glAttachShader(program, v);
    }
    
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


