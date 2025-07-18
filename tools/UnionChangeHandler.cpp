#include "tools.hpp"

UnionChangeHandler::UnionChangeHandler(std::initializer_list<OctreeChangeHandler*> handlersArgs) {
    for (auto handler : handlersArgs) {
        this->handlers.push_back(handler);
    }

};

void UnionChangeHandler::create(OctreeNode* node) {
    
};

void UnionChangeHandler::update(OctreeNode* node) {
    for (auto handler : handlers) {
        handler->update(node);
    }
};

void UnionChangeHandler::erase(OctreeNode* node) {
    for(auto handler : handlers) {
        handler->erase(node);
    }
};
