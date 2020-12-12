
#ifndef CONVERSIONUTILS_HPP
#define CONVERSIONUTILS_HPP

#include <cstdint>


enum class UnitType {
      Byte,
      Kilobyte,
      Megabyte,
      Gigabyte
};

/**
 * Simple class for more elegant conversion of bytes
 */
class ConversionUtils {

      static constexpr uint64_t KILOBYTE_TO_BYTES = 1024;
      static constexpr uint64_t MEGABYTE_TO_BYTES = 1024 * KILOBYTE_TO_BYTES;
      static constexpr uint64_t GIGABYTE_TO_BYTES = 1024 * MEGABYTE_TO_BYTES;

    public:
      /**
       * Converts gigabyte number to byte number
       * @param gigabyteSize amount of gigabytes
       * @return number of bytes
       */
      static const uint64_t gigabyteToBytes(uint8_t gigabyteSize) {
          return gigabyteSize * GIGABYTE_TO_BYTES;
      }

      /**
       * Converts megabyte number to byte number
       * @param megabyteSize amount of megabytes
       * @return number of bytes
       */
      static const uint64_t megabytesToBytes(uint16_t megabyteSize) {
          return megabyteSize * MEGABYTE_TO_BYTES;
      }

      /**
       * Converts kilobyte number to byte number
       * @param kilobyteSize amount of kilobytes
       * @return number of bytes
       */
      static const uint64_t kilobytesToBytes(uint32_t kilobyteSize) {
          return kilobyteSize * KILOBYTE_TO_BYTES;
      }

      static const uint64_t bytesToKilobytes(uint64_t bytes) {
          return bytes / KILOBYTE_TO_BYTES;
      }

      static const uint64_t bytesToMegabytes(uint64_t bytes) {
          return bytes / MEGABYTE_TO_BYTES;
      }


      static uint64_t toBytes(UnitType type, uint64_t size) {
          if (type == UnitType::Byte) {
              return size;
          }
          if (type == UnitType::Kilobyte) {
              return kilobytesToBytes(size);
          }
          if (type == UnitType::Megabyte) {
              return megabytesToBytes(size);
          } else {
              return gigabyteToBytes(size);
          }
      }
};


#endif //ZOSREWORK_CONVERSIONUTILS_HPP
