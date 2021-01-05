
#ifndef CONVERSIONUTILS_HPP
#define CONVERSIONUTILS_HPP

#include <cstdint>

/**
 * Trida pro prevod jednotek - byty na MB, kB apod.
 */
class ConversionUtils {

      static constexpr uint64_t KILOBYTE_TO_BYTES = 1024;
      static constexpr uint64_t MEGABYTE_TO_BYTES = 1024 * KILOBYTE_TO_BYTES;
      static constexpr uint64_t GIGABYTE_TO_BYTES = 1024 * MEGABYTE_TO_BYTES;

    public:
      /**
       * GB na B
       */
      static const uint64_t gigabyteToBytes(uint8_t gigabyteSize) {
          return gigabyteSize * GIGABYTE_TO_BYTES;
      }

      /**
       * MB na B
       */
      static const uint64_t megabytesToBytes(uint16_t megabyteSize) {
          return megabyteSize * MEGABYTE_TO_BYTES;
      }

      /**
       * kB na B
       */
      static const uint64_t kilobytesToBytes(uint32_t kilobyteSize) {
          return kilobyteSize * KILOBYTE_TO_BYTES;
      }

      /**
       * B na MB
       */
      static const uint64_t bytesToMegabytes(uint64_t bytes) {
          return bytes / MEGABYTE_TO_BYTES;
      }

};


#endif //ZOSREWORK_CONVERSIONUTILS_HPP
