#ifndef STRINGPARSING_HPP
#define STRINGPARSING_HPP

#include "ConversionUtils.hpp"
#include "FSException.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

/**
 * Utilitni trida pro parsing retezcu
 */
class StringParsing {
    public:

      // Regex pro kontrolu pri formatovani disku
      static inline const std::regex DRIVE_FORMAT_REGEX = std::regex("\\d+(b|kb|mb|gb|k|m|g)");

      /**
       * Extrakce cisla z daneho vstup
       * @param input vstup, ktery obsahuje cislo
       * @return
       */
      static std::pair<uint64_t, int> extractNumber(std::string& input);

      /**
       * Vrati, zda-li pro dany retezec odpovida regex
       * @param string retezec, ktery se bude testovat
       * @return true, pokud retezec odpovida regexu
       */
      static bool matchesFormatRegex(std::string string);

      /**
       * Zjisti z retezce velikost pro formatovani
       * @param input vstupni retezec
       * @return velikost v bytech
       */
      static uint64_t parseDriveFormatSize(std::string input);

      /**
       * Prevede dana data na byty
       * @param unitSize velikost jednotky
       * @param unit jednotka
       * @return pocet bytu
       */
      static uint64_t convertToBytes(uint64_t unitSize, std::string unit);

      /**
       * Zpracuje retezec s cestou a vrati jednotlive elementy ve vektoru
       * @param path retezec s cestou
       * @return vektor s elementy cesty
       */
      static std::vector<std::string> parsePath(const std::string& path);
};

#endif //STRINGPARSING_HPP
