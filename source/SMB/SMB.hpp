#ifndef SMB_HPP
#define SMB_HPP

// Modern toolchains no longer ship boost/utility/binary.hpp, but the generated
// SMB constants still use BOOST_BINARY(...). Provide a minimal compatibility shim
// so the project keeps compiling with C++14+ and binary literals.
#ifndef BOOST_BINARY
#define BOOST_BINARY(x) (0b##x)
#endif

#include <cstdint>

#include "SMBConstants.hpp"
#include "SMBEngine.hpp"

//---------------------------------------------------------------------
// Macros:
//---------------------------------------------------------------------

/**
 * Read and access a byte from emulated memory.
 */
#define M(addr) getMemory(addr)

/**
 * Read a word from emulated memory (in little-endian format).
 */
#define W(addr) getMemoryWord(addr)

/**
 * Call a subroutine stored in a goto label.
 */
#define JSR(subroutine, index) pushReturnIndex(index); goto subroutine; Return_ ## index:

/**
 * High/upper byte of a 16-bit integer.
 */
#define HIBYTE(v) (static_cast<uint8_t>((v >> 8) & 0xff))

/**
 * Low byte of a 16-bit integer.
 */
#define LOBYTE(v) (static_cast<uint8_t>(v & 0xff))

#endif // SMB_HPP
