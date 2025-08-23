#ifndef SIZED_INT_ARRAY_H
#define SIZED_INT_ARRAY_H

#include <stdint.h>

template <uint8_t bit_width, uint16_t length, bool is_signed>
class SizedIntArrayBase {
    public:
    class IntProxy {
        protected:
        uint8_t* byte;
        uint8_t lsb_idx;
        uint8_t width;

        public:
        IntProxy(uint8_t* byte, uint8_t lsb_idx, uint8_t width) : byte(byte), lsb_idx(lsb_idx), width(width) {}
        operator uint64_t() {
            uint64_t value = *byte >> lsb_idx; // Push off bits before LSB
            // Add bytes that follow
            for (uint8_t i = 1; i < (width + lsb_idx + 7) / 8; i++) {
                value += static_cast<uint64_t>(*(byte + i)) << (i * 8 - lsb_idx); // TODO make bitshift not + for speed
            }
            value &= ((1UL << width) - 1); // Mask away bits after MSB
            return value;
        }
    };
};

template <uint8_t bit_width, uint16_t length>
class SizedIntArrayBase<bit_width, length, true> {
    public:
    class IntProxy {

        protected:
        uint8_t* byte;
        uint8_t lsb_idx;
        uint8_t width;

        public:
        IntProxy(uint8_t* byte, uint8_t lsb_idx, uint8_t width) : byte(byte), lsb_idx(lsb_idx), width(width) {}
        operator int64_t() {
            uint64_t value = *byte >> lsb_idx; // Push off bits before LSB
            // Add bytes that follow
            for (uint8_t i = 1; i < (width + lsb_idx + 7) / 8; i++) {
                value += static_cast<uint64_t>(*(byte + i)) << (i * 8 - lsb_idx); // TODO make bitshift not + for speed
            }
            if (value & 1 << (width - 1)) value |= ~((1UL << width) - 1); // Negative, set all bits after MSB
            else value &= ((1UL << width) - 1); // Positive, mask away bits after MSB
            return value;
        }
    };
};

/**
 * An array of integers, similar to int[], implemented for memory efficiency at
 * a given maximum bit width. Accessible and settable with subscript operator,
 * like a regular array. Does not support pointer operations like an array does.
 * Setting to an integer larger than the bit width supports is undefined
 * behavior.
 *
 * @tparam bit_width uint8_t number of bits to use to store each integer. Maximum of 63.
 * A given bit_width yields an integer maximum of \f$2^bit_width - 1\f$.
 * @tparam length uint16_t maximum number of elements this array can store
 * @tparam is_signed bool whether to treat integers as two's complement representations
 */
template <uint8_t bit_width, uint16_t length, bool is_signed>
class SizedIntArray : SizedIntArrayBase<bit_width, length, is_signed> {

    public:
    uint16_t count = length;
    static constexpr uint16_t size = (length * bit_width + 7) / 8 ; // in bytes
    uint8_t compressed[size] = { 0 };

    private:

    /*
     * The subscript operator does not usually support assignment. To allow
     * operations like `b[3] = 4`, this type is used by the subscript
     * operator to return an object with logic to mutate the element of the
     * enclosing SizedIntArray when assigned.
     */
    class IntProxy : public SizedIntArrayBase<bit_width, length, is_signed>::IntProxy {

        template <bool condition, typename T = void> struct enable_if {};
        template <typename T> struct enable_if<true, T> { typedef T type; };

        public:
        IntProxy(uint8_t* byte, uint8_t lsb_idx, uint8_t width) : SizedIntArrayBase<bit_width, length, is_signed>::IntProxy(byte, lsb_idx, width) {}
        IntProxy(IntProxy&);
        
        IntProxy& operator=(uint64_t val) {
            *(this->byte) = (*(this->byte) & ~(((1UL << this->width) - 1) << this->lsb_idx)) | (val << this->lsb_idx);
            for (uint8_t i = 1; i < (this->width + this->lsb_idx + 7) / 8; i++) {
                *(this->byte + i) = (*(this->byte + i) & ~((1UL << (this->width - i * 8 + this->lsb_idx)) - 1)) | (val >> (i * 8 - this->lsb_idx));
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
        IntProxy b(compressed + (idx * bit_width / 8), static_cast<uint8_t>(idx * bit_width % 8), bit_width);
        return b;
    }

};

#endif
