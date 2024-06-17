#include "smats/lib/gmp.h"

#include "smats/util/exception.h"

namespace std {

// https://en.cppreference.com/w/cpp/utility/hash/operator()
size_t hash<mpq_class>::operator()(const mpq_class &val) const {
  mp_limb_t result = 2166136261;
  size_t num_size = mpz_size(val.get_num_mpz_t());
  size_t den_size = mpz_size(val.get_den_mpz_t());
  const mp_limb_t *num_limbs = mpz_limbs_read(val.get_num_mpz_t());
  const mp_limb_t *den_limbs = mpz_limbs_read(val.get_den_mpz_t());
  for (size_t i = 0; i < num_size; i++) {
    result = (result * 16777619) ^ num_limbs[i];
  }
  for (size_t i = 0; i < den_size; i++) {
    result = (result * 16777619) ^ den_limbs[i];
  }
  return static_cast<size_t>(result);
}

}  // namespace std

namespace smats::gmp {

mpz_class floor(const mpq_class &val) {
  // This rounds towards zero
  mpz_class t{val};
  if (t == val || val > 0) {
    return t;
  } else {
    // val is negative and non-integer, so it was rounded upwards
    return t - 1;
  }
}

mpz_class ceil(const mpq_class &val) {
  // This rounds towards zero
  mpz_class t{val};
  return t == val || val < 0 ? t : t + 1;
  if (t == val || val < 0) {
    return t;
  } else {
    // val is positive and non-integer, so it was rounded downwards
    return t + 1;
  }
}

mpq_class string_to_mpq(std::string_view str) {
  // Remove leading + and - sign
  bool is_negative = str[0] == '-';
  if (is_negative || str[0] == '+') str.remove_prefix(1);
  if (str == "inf") return {1e100};
  if (str == "-inf") return {-1e100};

  // case 1: string is given in integer format
  size_t symbol_pos = str.find_first_of("/.Ee");
  if (symbol_pos == std::string::npos) {
    const size_t start_pos = str.find_first_not_of('0', str[0] == '+' ? 1 : 0);
    if (start_pos == std::string_view::npos) return {0};
    SMATS_ASSERT_FMT(std::all_of(str.cbegin() + start_pos, str.cend(), is_digit_or_sign), "Invalid number: {}", str);
    return is_negative ? -mpq_class{str.data() + start_pos} : mpq_class{str.data() + start_pos};
  }

  // case 2: string is given in nom/denom format
  if (str[symbol_pos] == '/') {
    mpq_class res{str.data()};
    res.canonicalize();
    return is_negative ? -res : res;
  }

  const size_t e_pos = str[symbol_pos] == 'e' || str[symbol_pos] == 'E' ? symbol_pos : str.find_first_of("Ee");
  mpz_class mult{is_negative ? -1 : 1};
  bool is_exp_positive = true;

  // case 3a: string is given as base-10 decimal number (e)
  if (e_pos != std::string::npos) {
    const long exponent = std::stol(str.data() + e_pos + 1);  // NOLINT(runtime/int)
    is_exp_positive = exponent >= 0;
    mult = 10;
    mpz_pow_ui(mult.get_mpz_t(), mult.get_mpz_t(), std::abs(exponent));
    if (is_negative) mult = -mult;
    // Remove the exponent
    str = str.substr(0, e_pos);

    if (str.empty()) return is_exp_positive ? mpq_class{mult} : is_negative ? mpq_class{-1, -mult} : mpq_class{1, mult};
  }

  const size_t &len = str.length();

  // case 3b: string does not contain a . , only an exponent E
  if (str[symbol_pos] == 'e' || str[symbol_pos] == 'E') {
    int plus_pos = str[0] == '+' ? 1 : 0;
    SMATS_ASSERT_FMT(std::all_of(str.cbegin() + plus_pos, str.cend(), is_digit_or_sign), "Invalid number: {}", str);

    char *const str_number = new char[len - plus_pos + 1];
    memcpy(str_number, str.data() + plus_pos, len - plus_pos);
    str_number[len - plus_pos] = '\0';
    mpq_class res{str_number, 10};
    delete[] str_number;
    return res * mult;
  }

  const size_t &dot_pos = symbol_pos;

  // case 3c: string contains a .
  size_t start_pos = str.find_first_not_of('0');
  size_t digits;

  // case 4a: string starts with a . or the numbers before the . are all 0
  if (start_pos == dot_pos) {
    start_pos = str.find_first_not_of('0', dot_pos + 1);
    // case 5: string contains only a .
    if (start_pos == std::string_view::npos) {
      return {0};
    } else {
      digits = len - start_pos;
    }
  } else {  // case 4b: string does not start with a . and the numbers before the . are not all 0
    digits = len - start_pos - 1;
  }

  const size_t n_decimals = len - dot_pos - 1;
  SMATS_ASSERT_FMT(std::all_of(str.begin() + start_pos, str.begin() + dot_pos, is_digit_or_sign), "Invalid number: {}",
                   str);
  SMATS_ASSERT_FMT(std::all_of(str.begin() + dot_pos + 1, str.cend(), is_digit_or_sign), "Invalid number: {}", str);
  char *const str_number = new char[digits + n_decimals + 3];

  if (digits > n_decimals) {
    memcpy(str_number, str.data() + start_pos, digits - n_decimals);
    memcpy(str_number + dot_pos, str.data() + dot_pos + 1, n_decimals);
  } else {
    memcpy(str_number, str.data() + start_pos, n_decimals);
  }

  str_number[digits] = '/';
  str_number[digits + 1] = '1';
  memset(str_number + digits + 2, '0', n_decimals);
  str_number[digits + 2 + n_decimals] = '\0';

  mpq_class res{str_number, 10};
  delete[] str_number;
  res.canonicalize();
  return is_exp_positive ? mpq_class{res * mult} : res / mult;
}

}  // namespace smats::gmp
