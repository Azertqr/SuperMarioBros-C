#include "MemoryAccess.hpp"

MemoryAccess::MemoryAccess() : type(None), pointer(nullptr), constValue(0) {}

MemoryAccess::MemoryAccess(const MemoryAccess& other)
    : type(other.type), constValue(other.constValue) {
    if (type == Constant) {
        pointer = &constValue;
    } else {
        pointer = other.pointer;
    }
}

MemoryAccess& MemoryAccess::operator=(const MemoryAccess& other) {
    if (this != &other) {
        type = other.type;
        constValue = other.constValue;
        if (type == Constant) {
            pointer = &constValue;
        } else {
            pointer = other.pointer;
        }
    }
    return *this;
}

MemoryAccess MemoryAccess::createConstant(uint8_t constant) {
    MemoryAccess ma;
    ma.type = Constant;
    ma.constValue = constant;
    ma.pointer = &ma.constValue;
    return ma;
}

MemoryAccess MemoryAccess::createRead(uint8_t* ptr) {
    MemoryAccess ma;
    ma.type = Read;
    ma.pointer = ptr;
    return ma;
}

MemoryAccess MemoryAccess::createWrite(uint8_t* ptr) {
    MemoryAccess ma;
    ma.type = Write;
    ma.pointer = ptr;
    return ma;
}

uint8_t MemoryAccess::read() const {
    if (pointer) {
        return *pointer;
    }
    return 0;
}

void MemoryAccess::write(uint8_t val) {
    if (type == Write && pointer) {
        *pointer = val;
    }
}
