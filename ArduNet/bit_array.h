#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

#include <stdint.h>

/**
 * An array of bits, similar to bool[], implemented for memory efficiency.
 * Eight elements can be stored per byte of space. Accessible and settable with
 * subscript operator, like a regular array. Does not support pointer operations
 * like an array does.
 *
 * @tparam Length maximum number of elements this array can store
 */
template <uint16_t Length>
class BitArray {

    public:
    static const uint16_t length = Length;

    private:
    static constexpr uint8_t size = (length + 7) / 8 ; // in bytes
    uint8_t arr[size] = { 0 };

    /*
     * The subscript operator does not usually support assignment. To allow
     * operations like `b[3] = true`, this type is used by the subscript
     * operator to return an object with logic to mutate the element of the
     * enclosing BitArray when assigned.
     */
    class BitProxy {

        uint8_t* byte;
        uint8_t bit_idx;

        public:
        BitProxy(uint8_t* byte, uint8_t bit_idx) : byte(byte), bit_idx(bit_idx) {}
        BitProxy(BitProxy&);

        // Implicit conversion to mimic bool
        operator bool() {
            return (*byte & (1 << bit_idx)) >> bit_idx;
        }

        BitProxy& operator=(bool val) {
            *byte = (*byte & ~(1 << bit_idx)) | (val << bit_idx);
            return *this;
        }

        // When assigned another BitProxy, only assign the value of the bit
        BitProxy& operator=(BitProxy& other) {
            *this = static_cast<int>(other);
            return *this;
        }

    };

    public:

    /**
     * Returns a proxy for the element at idx that remembers the element's
     * location in the array. The proxy is required in order to allow assignment
     * of array elements through the subscript operator, i.e. `b[3] = true`.
     *
     * @param idx the index of the element to get
     * @return a proxy for the element
     */
    BitProxy operator[](uint16_t idx) {
        BitProxy b = { arr + (idx / 8), static_cast<uint8_t>(idx % 8) };
        return b;
    }

};

#endif
