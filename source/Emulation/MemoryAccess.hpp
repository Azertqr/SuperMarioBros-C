#ifndef MEMORYACCESS_HPP
#define MEMORYACCESS_HPP

#include <cstdint>

class MemoryAccess {
public:
    enum Type {
        None,
        Read,
        Write,
        Constant
    };

    MemoryAccess();
    MemoryAccess(const MemoryAccess& other);
    MemoryAccess& operator=(const MemoryAccess& other);

    static MemoryAccess createConstant(uint8_t constant);
    static MemoryAccess createRead(uint8_t* ptr);
    static MemoryAccess createWrite(uint8_t* ptr);

    uint8_t read() const;
    void write(uint8_t val);
    Type getType() const { return type; }

private:
    Type type;
    uint8_t* pointer;
    uint8_t constValue;
};

#endif // MEMORYACCESS_HPP
