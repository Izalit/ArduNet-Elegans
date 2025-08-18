#ifndef SIZED_INT_ARRAY_H
#define SIZED_INT_ARRAY_H

#include <stdint.h>

/**
 * An array of integers, similar to int[], implemented for memory efficiency at
 * a given maximum bit width. Accessible and settable with subscript operator,
 * like a regular array. Does not support pointer operations like an array does.
 * Setting to an integer larger than the bit width supports is undefined
 * behavior.
 *
 * TODO signed
 *
 * @tparam bit_width number of bits to use to store each integer. Maximum of 63.
 * A given bit_width yields an integer maximum of \f$2^bit_width - 1\f$.
 * @tparam length maximum number of elements this array can store
 */
template <uint8_t bit_width, uint16_t length>
class SizedIntArray {

    private:
    static constexpr uint16_t size = (length * bit_width + 7) / 8 ; // in bytes
    uint8_t arr[size] = { 0 };

    /*
     * The subscript operator does not usually support assignment. To allow
     * operations like `b[3] = 4`, this type is used by the subscript
     * operator to return an object with logic to mutate the element of the
     * enclosing SizedIntArray when assigned.
     */
    class IntProxy {

        uint8_t* byte;
        uint8_t lsb_idx;
        uint8_t width;

        public:
        IntProxy(uint8_t* byte, uint8_t lsb_idx, uint8_t width) : byte(byte), lsb_idx(lsb_idx), width(width) {}
        IntProxy(IntProxy&);

        // Implicit conversion to mimic integer types
        // TODO: type? based on bit_width?
        operator uint64_t() {
            uint64_t value = *byte >> lsb_idx; // Push off bits before LSB
            // Add bytes that follow
            for (uint8_t i = 1; i < (width + lsb_idx + 7) / 8; i++) {
                value += static_cast<uint64_t>(*(byte + i)) << (i * 8 - lsb_idx); // TODO make bitshift not + for speed
            }
            value &= ((1UL << width) - 1); // Mask away bits after MSB
            return value;
        }
        
        IntProxy& operator=(uint64_t val) {
            *byte = (*byte & ~(((1UL << width) - 1) << lsb_idx)) | (val << lsb_idx);
            for (uint8_t i = 1; i < (width + lsb_idx + 7) / 8; i++) {
                *(byte + i) = (*(byte + i) & ~((1UL << (width - i * 8 + lsb_idx)) - 1)) | (val >> (i * 8 - lsb_idx));
            }
            return *this;
        }

        // When assigned another IntProxy, only assign its integer value
        IntProxy& operator=(IntProxy const& other) {
            *this = static_cast<uint64_t>(other);
            return *this;
        }

    };

    public:

    /**
     * Returns a proxy for the element at idx that remembers its bit width and
     * location in the array. The proxy is required in order to allow assignment
     * of array elements through the subscript operator, i.e. `b[3] = 4`.
     *
     * @param idx the index of the element to get
     * @return a proxy for the element
     */ 
    IntProxy operator[](uint16_t idx) {
        IntProxy b(arr + (idx * bit_width / 8), static_cast<uint8_t>(idx * bit_width % 8), bit_width);
        return b;
    }

};

#endif
