#include <crc32c/crc32c.hpp>

#include <cpuid.h>
#include <cstdint>
#include <immintrin.h>

#include <stdexcept>

#include "crc32c_golden_amd.hpp"
#include "crc32c_golden_intel.hpp"
#include "crc32c_golden_sw.hpp"

/*
 * \brief Detect the CPU vendor
 *
 * \return CRC32C::CpuVendor
 */
CRC32C::CpuVendor CRC32C::detect_cpu_vendor() {
  // Get the CPU vendor string
  uint32_t eax, ebx, ecx, edx;
  if (!__get_cpuid(0, &eax, &ebx, &ecx, &edx))
    throw std::runtime_error("CPUID not supported");

  // Check the CPU vendor string
  if (ebx == signature_INTEL_ebx && edx == signature_INTEL_edx &&
      ecx == signature_INTEL_ecx) {
    return CpuVendor::Intel;
  } else if (ebx == signature_AMD_ebx && edx == signature_AMD_edx &&
             ecx == signature_AMD_ecx) {
    return CpuVendor::AMD;
  } else {
    return CpuVendor::Unsupported;
  }
}

/*
 * \brief Detect CPU Features
 *
 * \return uint64_t (edx << 32 | ecx)
 */
uint64_t CRC32C::detect_cpu_features() {
  uint32_t eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    throw std::runtime_error("CPUID not supported");

  return (uint64_t)edx << 32 | ecx;
}

/*
 * \brief Detect the CRC32C supported algorithm
 * 
 * \return CRC32C::CRC32CAlgorithm
 */
CRC32C::CRC32CAlgorithm CRC32C::detect_crc32c_algorithm() {
  // Detect CPU Features
  if (!cpu_features) {
    cpu_features = detect_cpu_features();
  }
  uint32_t ecx = cpu_features & 0xFFFFFFFF;
  uint32_t edx = cpu_features >> 32;

  // Detect the CPU vendor
  if (cpu_vendor == CpuVendor::Unset) {
    cpu_vendor = detect_cpu_vendor();
  }

  // Check the CPU Features
  if (ecx & bit_PCLMUL && // Common to both Intel and AMD
      edx & bit_SSE2 &&   // Common to both Intel and AMD
      ecx & bit_SSE3 &&   // Intel only, but it almost? impossible to find a CPU with SSE4.2 but not with SSE3
      ecx & bit_SSE4_1 && // Intel only, but it almost? impossible to find a CPU with SSE4.2 but not with SSE4.1
      ecx & bit_SSE4_2)   // Common to both Intel and AMD
  {
    if (cpu_vendor == CpuVendor::Intel) {
      return CRC32CAlgorithm::Intel;
    } else if (cpu_vendor == CpuVendor::AMD) {
      return CRC32CAlgorithm::AMD;
    } else {
      // Try AMD anyway
      return CRC32CAlgorithm::AMD;
    }
  } else {
    return CRC32CAlgorithm::Software;
  }
}

/*
 * \brief Constructor
 */
CRC32C::CRC32C() {
  // Detect the CRC32C supported algorithm
  if (algorithm == CRC32CAlgorithm::Unset) {
    algorithm = detect_crc32c_algorithm();
  }

  // Set the function pointer
  switch (algorithm) {
  case CRC32CAlgorithm::Intel:
    init_golden_intel();
    calc = crc32c_golden_intel;
    break;
  case CRC32CAlgorithm::AMD:
    init_golden_amd();
    calc = crc32c_golden_amd;
    break;
  case CRC32CAlgorithm::Software:
  default:
    init_tabular_method_tables();
    calc = crc32c_tabular;
    break;
  }
}

/*
 * \brief Destructor
 */
CRC32C::~CRC32C() {
  // Deinit the function pointer
  switch (algorithm) {
  case CRC32CAlgorithm::Intel:
    deinit_golden_intel();
    break;
  case CRC32CAlgorithm::AMD:
    deinit_golden_amd();
    break;
  case CRC32CAlgorithm::Software:
  default:
    deinit_tabular_method_tables();
    break;
  }
}
