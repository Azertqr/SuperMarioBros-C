#include "MemoryAccess.hpp"

MemoryAccess::MemoryAccess()
{
    this->type = None;
    this->constantValue = 0;
    this->value = nullptr;
}

MemoryAccess::MemoryAccess(uint8_t constant)
{
    this->type = Constant;
    this->constantValue = constant;
    this->value = &this->constantValue;
}

MemoryAccess::MemoryAccess(uint8_t* pointer)
{
    this->type = pointer != nullptr ? Read : None;
    this->constantValue = 0;
    this->value = pointer;
}

MemoryAccess::MemoryAccess(std::function<uint8_t()> readCallback)
{
    this->type = Read;
    this->constantValue = 0;
    this->value = nullptr;
    this->readCallback = readCallback;
}

MemoryAccess::MemoryAccess(std::function<uint8_t()> readCallback, std::function<void(uint8_t)> writeCallback)
{
    this->type = Write;
    this->constantValue = 0;
    this->value = nullptr;
    this->readCallback = readCallback;
    this->writeCallback = writeCallback;
}

MemoryAccess::MemoryAccess(const MemoryAccess& other)
{
    this->type = other.type;
    this->constantValue = other.constantValue;
    if (this->type == Constant)
    {
        this->value = &this->constantValue;
    }
    else
    {
        this->value = other.value;
    }
    this->readCallback = other.readCallback;
    this->writeCallback = other.writeCallback;
}

MemoryAccess& MemoryAccess::operator=(const MemoryAccess& other)
{
    if (this != &other)
    {
        this->type = other.type;
        this->constantValue = other.constantValue;
        if (this->type == Constant)
        {
            this->value = &this->constantValue;
        }
        else
        {
            this->value = other.value;
        }
        this->readCallback = other.readCallback;
        this->writeCallback = other.writeCallback;
    }
    return *this;
}

MemoryAccess::operator uint8_t()
{
    return read();
}

MemoryAccess& MemoryAccess::operator=(uint8_t value)
{
    write(value);
    return *this;
}

MemoryAccess& MemoryAccess::operator++()
{
    write(read() + 1);
    return *this;
}

MemoryAccess MemoryAccess::operator++(int)
{
    MemoryAccess result = *this;
    write(read() + 1);
    return result;
}

MemoryAccess& MemoryAccess::operator--()
{
    write(read() - 1);
    return *this;
}

MemoryAccess MemoryAccess::operator--(int)
{
    MemoryAccess result = *this;
    write(read() - 1);
    return result;
}

uint8_t MemoryAccess::read()
{
    if (type == Constant || (type == Read && value != nullptr))
    {
        return *value;
    }
    else if (readCallback != nullptr)
    {
        return readCallback();
    }
    return 0;
}

void MemoryAccess::write(uint8_t value)
{
    if (this->value != nullptr)
    {
        *this->value = value;
    }
    else if (writeCallback != nullptr)
    {
        writeCallback(value);
    }
}
