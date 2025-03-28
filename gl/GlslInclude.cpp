#include "gl.hpp"


GlslInclude::GlslInclude(std::string line, std::string code){
    this->line = line;
    this->code = code;
}

std::string GlslInclude::replace(std::string input,  std::string replace_word, std::string replace_by ) {
    size_t pos = input.find(replace_word);
    while (pos != std::string::npos) {
        input.replace(pos, replace_word.size(), replace_by);
        pos = input.find(replace_word, pos + replace_by.size());
    }
    return input;
}

std::string GlslInclude::replaceIncludes(std::vector<GlslInclude> includes, std::string code){
    for(GlslInclude include : includes) {
        code = replace(code, include.line, include.code);
    }
    return code;
}
