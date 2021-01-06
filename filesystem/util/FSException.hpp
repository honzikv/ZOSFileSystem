
#ifndef FSEXCEPTION_HPP
#define FSEXCEPTION_HPP


#include <exception>
#include <string>

/**
 * Jednoducha trida pro vyjimku
 */
class FSException : public std::exception {

      const std::string text; // zprava chyby

    public:
      explicit FSException(std::string text);

      FSException();

    public:
      [[nodiscard]] const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
};


#endif //ZOSREWORK_FSEXCEPTION_HPP
