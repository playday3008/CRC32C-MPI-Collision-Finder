#pragma once

#include <cstdint>

/*
 * \brief Compute the CRC32C (Castagnoli) polynomial.
 *
 * \param M     Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param prev  Previous CRC32C value.
 * 
 * \return     The CRC32C value.
 * 
 * \note       This function uses the hardware CRC32 instruction to compute the
 *            CRC32C polynomial.
 */
uint32_t crc32c_golden_amd(const void *M, uint32_t bytes, uint32_t prev /* = 0*/);

/*
 * \brief Initialize the golden LUT.
 *
 * \return      None.
 */
void init_golden_amd();

/*
 * \brief Deinitialize the golden LUT.
 *
 * \return      None.
 */
void deinit_golden_amd();
