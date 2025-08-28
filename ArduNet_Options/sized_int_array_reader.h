#ifndef SIZED_INT_ARRAY_READER_H
#define SIZED_INT_ARRAY_READER_H

#include <stdint.h>

/**
 * Reads from an existing array of unsigned integers of a given bit width,
 * packed to straddle byte boundaries with no padding bits. Accessible with
 * subscript operator. Cannot perform assignments. Can accept an offset to
 * apply to every access.
 *
 * @tparam bit_width uint8_t number of bits per integer. Maximum of 63.
 * A given bit_width yields an integer maximum of \f$2^BitWidth - 1\f$.
 * @param array uint8_t[] the read-only array of integers. Needs to have type
 * uint8_t[], regardless of the actual bit width of the integers inside.
 * @param length uint16_t number of elements in the array
 * @param offset int64_t Optional signed integer to subtract from every access.
 * For example, an offset of -70 with a bit_width of 9 translates the range of the
 * array from [0, 511] to [-70, 441].
 */
template <uint8_t bit_width>
class SizedIntArrayReader {

    const uint8_t* array;
    uint16_t length;
    int64_t offset = 0;
    bool progmem;

    public:
    SizedIntArrayReader(const uint8_t* array, uint16_t length, int64_t offset=0, bool progmem=false) :
        array(array), length(length), offset(offset), progmem(progmem) {}

    /**
     * Returns a proxy for the element at idx that remembers its bit width and
     * location in the array. The proxy is required in order to allow assignment
     * of array elements through the subscript operator, i.e. `b[3] = 4`.
     *
     * @param idx the index of the element to get
     * @return a proxy for the element
     */
    int64_t operator[](uint16_t idx) {
        const uint8_t* low_byte = array + ((idx + 1) * static_cast<uint32_t>(bit_width) - 1) / 8;
        uint8_t msb_idx = 7 - ((idx * bit_width) % 8);
        uint8_t lsb_idx = 7 - (((idx + 1) * bit_width - 1) % 8);
        uint64_t value = progmem ? pgm_read_byte(low_byte) : *low_byte;
        value >>= lsb_idx; // Push off bits before LSB
        // Add bytes that follow
        for (uint8_t i = 1; i < (bit_width + msb_idx + 7) / 8; i++) {
            uint64_t addend = progmem ? pgm_read_byte(low_byte - i) : *(low_byte - i);
            value += addend << (i * 8 - lsb_idx); // TODO make bitshift not + for speed
        }
        value &= (1UL << bit_width) - 1; // Mask away bits after MSB
        return value + offset;
    }

};

#endif
