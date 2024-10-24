# Bithack

This is the last exam I did, because I had no idea at first and have no time it in the end.

I deliberately failed this one because I don't want to submit something without any old school RTFM, STFW and my own effort, but sharing some code idea and notes are always good of all the times.

## Implementation Notes

Obviously we need to improve this "rotate by one" way of implementation. For bits, bit operations (hacks) are always the most efficient.

So... We need to first extract the bits, rotate it, and put it back. Maybe we need to do it multiple times. But how?

There's Gemini 1.5 Flash's idea, with my commenting:

```c
/**
 * Powered by Gemini 1.5 Flash @ Devv.ai
 * Helper function to get a bit from the bit vector
 */
static inline uint8_t get_bit(const bit_vector_t *bv, size_t pos) {
    return (bv->buf[pos / 8] >> (pos % 8)) & 1;
    // pos / 8      : 8 bits per char byte
    // >> (pos % 8) : push the bit to LSB
    // & 1          : if this bit 1 return 1 else 0
}

/**
 * Powered by Gemini 1.5 Flash @ Devv.ai
 * Helper function to set a bit in the bit vector
 */
static inline void set_bit(bit_vector_t *bv, size_t pos, uint8_t val) {
    // 1 << (pos % 8) : push the bit to position in one byte (as a mask)
    // |=             : if val == 1 then only the bit in pos will turn 1
    //                  (other bits in the mask is 0)
    // &= ~ ...       : else the mask will be 11101111
    //                  the bit in pos will be masked to 0
    if (val) {
        bv->buf[pos / 8] |= (1 << (pos % 8));
    } else {
        bv->buf[pos / 8] &= ~(1 << (pos % 8));
    }
}

/**
 * Powered by Gemini 1.5 Flash @ Devv.ai
 */
static void rotate_the_bit_vector_left(bit_vector_t *const bit_vector,
                                       const size_t bit_offset,
                                       const size_t bit_length,
                                       const size_t bit_left_amount) {
    // Extract the relevant bits
    // Wait won't this be truncated????
    // better be `bits[bit_length / 64 (+ 1)]`
    uint64_t bits = 0;
    for (size_t i = 0; i < bit_length; ++i) {
        // the bit will be pushed left by one
        // and a new bit will be getted
        bits = (bits << 1) | get_bit(bit_vector, bit_offset + i);
    }

    // Rotate the bits left

    // bits << bit_left_amount
    //   : normal left shift (without wrapping)
    // bits >> (bit_length - bit_left_amount)
    //   : wrap the overflow to the LSB of bit array
    // | : C always do logical shifts on UNSIGNED,
    //     so the two bit arrays will be filled
    //     with zeros at the opposite side of the
    //     shift, we bit wise ORing them, we're done.
    bits = (bits << bit_left_amount) | (bits >> (bit_length - bit_left_amount));

    // Write the rotated bits back to the bit vector
    for (size_t i = 0; i < bit_length; ++i) {
        set_bit(bit_vector, bit_offset + i, (bits >> (bit_length - 1 - i)) & 1);
    }
}
```

This is too ELEGENT. I didn't expect the code to be so clean and simple. Why haven't I come up with such a clever and simple solution??

> Results on my PC, they did a great job:
> 
> ```txt
> check result: PASSED
> performance of -s: 29
> performance of -m: 34
> performance of -l: 39
> ------score--------
> -s : 75.00 /100
> -m : 81.82 /100
> -l : 84.00 /100
> total score: 81.55 /100
> ```

I did find something, the `uint64_t bits = 0` might have the problem of data being truncated, so a `char[bit_length]` might be a better option.

# The End

I feel pleasured to finish the Xflops 2024 enrollment exam. Really enjoyed the HPL part when I finally compiled the binary and pushed the clusters to their limits, that is the greatest of all the times. Whether or not being enrolled, I did learn a thing or two, which is what matters most.
