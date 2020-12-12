#include "FSException.hpp"

const char* FSException::what() const noexcept {
    return text.c_str();
}

FSException::FSException(std::string text) : text(std::move(text)) { }

FSException::FSException() {}
