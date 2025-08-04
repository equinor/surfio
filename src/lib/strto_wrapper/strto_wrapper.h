#pragma once

#if !STD_FROM_CHARS_FLOAT_SUPPORT
#include <clocale>
#include <cstdlib>
#include <stdexcept>
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#if defined(__FreeBSD__) || (defined(__APPLE__) && defined(__MACH__))
#include <xlocale.h>
#endif // End of defined(__FreeBSD__) || (defined(__APPLE__) && defined(__MACH__))
#define strtod_l_wrapper strtod_l
#define strtof_l_wrapper strtof_l
struct wrapped_c_locale {
  locale_t cLocale;
  wrapped_c_locale() {
    cLocale = newlocale(LC_NUMERIC_MASK, "C", nullptr);
    if (!cLocale)
      throw std::runtime_error("Did not manage to create a C locale");
  }
  ~wrapped_c_locale() { freelocale(cLocale); }
};

#elif defined(_WIN32) ||                                                                           \
    defined(_WIN64) // End of defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define strtod_l_wrapper _strtod_l
#define strtof_l_wrapper _strtof_l
struct wrapped_c_locale {
  _locale_t cLocale;
  wrapped_c_locale() {
    cLocale = _create_locale(LC_NUMERIC, "C");
    if (!cLocale)
      throw std::runtime_error("Did not manage to create a C locale");
  }
  ~wrapped_c_locale() { _free_locale(cLocale); }
};

#endif // End of defined(__WIN32) || defined(__WIN64)

#endif // End of !STD_FROM_CHARS_FLOAT_SUPPORT
