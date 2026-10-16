#ifndef MEMORYACCESS_HPP
#define MEMORYACCESS_HPP

#include <cstdint>
#include <functional>

/**
 * Encapsulates read/write access to a memory location or register.
 */
class MemoryAccess
{
public:
    enum Type
    {
        None,
        Read,
        Write,
        Constant
    };

    MemoryAccess();
    MemoryAccess(uint8_t constant);
    MemoryAccess(uint8_t* pointer);
    MemoryAccess(std::function<uint8_t()> readCallback);
    MemoryAccess(std::function<uint8_t()> readCallback, std::function<void(uint8_t)> writeCallback);
    MemoryAccess(const MemoryAccess& other);
    MemoryAccess& operator=(const MemoryAccess& other);

    operator uint8_t();
    MemoryAccess& operator=(uint8_t value);
    MemoryAccess& operator++();
    MemoryAccess operator++(int);
    MemoryAccess& operator--();
    MemoryAccess operator--(int);
    MemoryAccess& operator+=(uint8_t value);
    MemoryAccess& operator-=(uint8_t value);
    MemoryAccess& operator|=(uint8_t value);
    MemoryAccess& operator&=(uint8_t value);
    MemoryAccess& operator^=(uint8_t value);
    MemoryAccess& operator<<=(uint8_t value);
    MemoryAccess& operator>>=(uint8_t value);

    uint8_t read();
    void write(uint8_t value);

    uint8_t asl();
    uint8_t lsr();
    uint8_t rol(bool carry = false);
    uint8_t ror(bool carry = false);

private:
    Type type;
    uint8_t constantValue;
    uint8_t* value;
    std::function<uint8_t()> readCallback;
    std::function<void(uint8_t)> writeCallback;
};

#endif // MEMORYACCESS_HPP
