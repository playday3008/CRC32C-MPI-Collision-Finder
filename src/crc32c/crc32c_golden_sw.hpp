#pragma once

#include <cstdint>

/*
 * \brief Compute the CRC-32C checksum for the Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 * 
 * \note This function have step of 1 byte.
 */
uint32_t crc32c_tabular_1_byte(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Compute the CRC-32C checksum for the Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 * 
 * \note This function have step of 2 bytes.
 */
uint32_t crc32c_tabular_2_bytes(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Compute the CRC-32C checksum for the Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 * 
 * \note This function have step of 4 bytes.
 */
uint32_t crc32c_tabular_4_bytes(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Compute the CRC-32C checksum for the Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 * 
 * \note This function have step of 8 bytes.
 */
uint32_t crc32c_tabular_8_bytes(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Compute the CRC-32C checksum for the Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 * 
 * \note This function have step of 16 bytes.
 */
uint32_t crc32c_tabular_16_bytes(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Decide which function to use to compute the CRC-32C checksum for the
 * Castagnoli polynomial.
 * 
 * \param M Pointer to the message.
 * \param bytes Number of bytes in the message.
 * \param crc Initial CRC value.
 * 
 * \return The CRC-32C checksum.
 */
uint32_t crc32c_tabular(const void *M, uint32_t bytes, uint32_t crc /*= 0*/);

/*
 * \brief Initialize the CRC-32C table for the Castagnoli polynomial.
 *
 * \return      None.
 */
void init_tabular_method_tables();

/*
 * \brief Deinitialize the CRC-32C table for the Castagnoli polynomial.
 *
 * \return      None.
 */
void deinit_tabular_method_tables();
