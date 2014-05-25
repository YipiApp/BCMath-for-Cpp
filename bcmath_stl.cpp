/*
    This file is part of VK/KittenPHP-DB-Engine.

    VK/KittenPHP-DB-Engine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    VK/KittenPHP-DB-Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VK/KittenPHP-DB-Engine.  If not, see <http://www.gnu.org/licenses/>.

    This program is released under the GPL with the additional exemption
    that compiling, linking, and/or using OpenSSL is allowed.
    You are free to remove this exemption from derived works.

    Copyright 2012-2013 Vkontakte Ltd
              2012-2013 Arseny Smirnov
              2012-2013 Aliaksei Levin

    URL: https://github.com/vk-com/kphp-kdb/blob/master/KPHP/runtime/bcmath.cpp

    Adapted to STL C++:
        2014 Kijam Lopez B. <klopez@cuado.co>
*/


#define BC_ASSERT(cond) ((!(cond)) ? bc_assert(#cond,__FILE__,__LINE__) : bc_noassert())

#include "bcmath_stl.h"
#include <string.h>
#include <stdlib.h>

static int bc_scale = 6;

static const std::string ONE ("1");
static const std::string ZERO ("0");

static void bc_noassert() { }
static void bc_assert(const char *assertion, const char *file, int line) {
    std::cerr<<"Critical Error in: "<<assertion<<", File '"<<file<<"' in line "<<line<<"."<<std::endl;
    exit(-1);
}

//parse a number into parts, returns scale on success and -1 on error
static int bc_parse_number (const std::string &s, int &lsign, int &lint, int &ldot, int &lfrac, int &lscale) {
  int i = 0;
  lsign = 1;
  if (s[i] == '-' || s[i] == '+') {
    if (s[i] == '-') {
      lsign = -1;
    }
    i++;
  }
  int len = s.length();
  if (i >= len) {
    return -1;
  }
  lint = i;

  while (i < len && '0' <= s[i] && s[i] <= '9') {
    i++;
  }
  ldot = i;

  lscale = 0;
  if (i < len && s[i] == '.') {
    lscale = (int)s.length() - i - 1;
    i++;
  }
  lfrac = i;

  while (i < len && '0' <= s[i] && s[i] <= '9') {
    i++;
  }

  if (i < len) {
    return -1;
  }

  while (s[lint] == '0' && lint + 1 < ldot) {
    lint++;
  }
//  while (lscale > 0 && s[lfrac + lscale - 1] == '0') {
//    lscale--;
//  }
  if (lscale == 0 && lfrac > ldot) {
    lfrac--;
    BC_ASSERT (lfrac == ldot);
  }

  if (lsign < 0 && (lscale == 0 && s[lint] == '0')) {
    lsign = 1;
  }
  return lscale;
}

static std::string bc_zero (int scale) {
  if (scale == 0) {
    return ZERO;
  }
  std::string result (scale + 2, '0');
  result[1] = '.';
  return result;
}

static int bc_comp (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  if (llen != rlen) {
    return (llen < rlen ? -1 : 1);
  }

  for (int i = 0; i < llen; i++) {
    if (lhs[lint + i] != rhs[rint + i]) {
      return (lhs[lint + i] < rhs[rint + i] ? -1 : 1);
    }
  }

  int i;
  for (i = 0; (i < lscale || i < rscale) && i < scale; i++) {
    int lchar = (i < lscale ? lhs[lfrac + i] : '0');
    int rchar = (i < rscale ? rhs[rfrac + i] : '0');
    if (lchar != rchar) {
      return (lchar < rchar ? -1 : 1);
    }
  }

  return 0;
}

static std::string bc_round (char *lhs, int lint, int ldot, int lfrac, int lscale, int scale, int sign, bool add_trailing_zeroes, bool round_last = false) {
  while (lhs[lint] == '0' && lint + 1 < ldot) {
    lint++;
  }

  BC_ASSERT (lint > 0 && lscale >= 0 && scale >= 0);

  if (sign < 0 && lhs[lint] == '0') {
    sign = 1;
    for (int i = 0; i < lscale; i++) {
      if (lhs[lfrac + i] != '0') {
        sign = -1;
        break;
      }
    }
  }

  if(round_last) {
      if (lscale > scale) {
        while (scale > 0 && lhs[lfrac + scale - 1] == '9' && lhs[lfrac + scale] >= '5') {
          scale--;
        }
        lscale = scale;
        if (lhs[lfrac + scale] >= '5') {
          if (scale > 0) {
            lhs[lfrac + scale - 1]++;
          } else {
            lfrac--;
            BC_ASSERT (lfrac == ldot);

            int i;
            lhs[lint - 1] = '0';
            for (i = 0; lhs[ldot - i - 1] == '9'; i++) {
              lhs[ldot - i - 1] = '0';
            }
            lhs[ldot - i - 1]++;
            if (ldot - i - 1 < lint) {
              lint = ldot - i - 1;
            }
          }
        }
      }

      while (lscale > 0 && lhs[lfrac + lscale - 1] == '0') {
        lscale--;
      }
  }else{
      if (lscale > scale) {
        lscale = scale;
      }
  }

  if (lscale == 0 && lfrac > ldot) {
    lfrac--;
    BC_ASSERT (lfrac == ldot);
  }

  if (sign < 0) {
    lhs[--lint] = '-';
  }

  if (lscale == scale || !add_trailing_zeroes) {
    return std::string (std::string(lhs + lint).substr(0, lfrac + lscale - lint));
  } else {
    std::string result (std::string(lhs + lint).substr(0, lfrac + lscale - lint));
    if (lscale == 0) {
      result+='.';
    }
    for(int kI = 0; kI < scale - lscale; ++kI)
        result+='0';
    return result;
  }
}

static std::string bc_add_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = std::max (llen, rlen) + 1;
  int result_scale = std::max (lscale, rscale);
  int result_size = result_len + result_scale + 3;
  std::string result(result_size, '0');

  int i, um = 0;
  int cur_pos = result_size;
  int was_frac = 0;
  for (i = result_scale - 1; i >= 0; i--) {
    if (i < lscale) {
      um += lhs[lfrac + i] - '0';
    }
    if (i < rscale) {
      um += rhs[rfrac + i] - '0';
    }

    if (um != 0 || was_frac) {
      result[--cur_pos] = (char)(um % 10 + '0');
      um /= 10;
      was_frac = 1;
    }
  }
  resscale = result_size - cur_pos;
  resfrac = cur_pos;
  if (was_frac) {
    result[--cur_pos] = '.';
  }
  resdot = cur_pos;

  for (int i = 0; i < result_len; i++) {
    if (i < llen) {
      um += lhs[ldot - i - 1] - '0';
    }
    if (i < rlen) {
      um += rhs[rdot - i - 1] - '0';
    }

    result[--cur_pos] = (char)(um % 10 + '0');
    um /= 10;
  }
  resint = cur_pos;
  BC_ASSERT (cur_pos > 0);

  return bc_round ((char*)result.data(), resint, resdot, resfrac, resscale, scale, sign, 1);
}

static std::string bc_sub_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = llen;
  int result_scale = std::max (lscale, rscale);
  int result_size = result_len + result_scale + 3;
  std::string result (result_size, '0');

  int i, um = 0, next_um = 0;
  int cur_pos = result_size;
  int was_frac = 0;
  for (i = result_scale - 1; i >= 0; i--) {
    um = next_um;
    if (i < lscale) {
      um += lhs[lfrac + i] - '0';
    }
    if (i < rscale) {
      um -= rhs[rfrac + i] - '0';
    }
    if (um < 0) {
      next_um = -1;
      um += 10;
    } else {
      next_um = 0;
    }

    if (um != 0 || was_frac) {
      result[--cur_pos] = (char)(um + '0');
      was_frac = 1;
    }
  }
  resscale = result_size - cur_pos;
  resfrac = cur_pos;
  if (was_frac) {
    result[--cur_pos] = '.';
  }
  resdot = cur_pos;

  for (int i = 0; i < result_len; i++) {
    um = next_um;
    um += lhs[ldot - i - 1] - '0';
    if (i < rlen) {
      um -= rhs[rdot - i - 1] - '0';
    }
    if (um < 0) {
      next_um = -1;
      um += 10;
    } else {
      next_um = 0;
    }

    result[--cur_pos] = (char)(um + '0');
  }
  resint = cur_pos;
  BC_ASSERT (cur_pos > 0);

  return bc_round ((char*)result.data(), resint, resdot, resfrac, resscale, scale, sign, 1);
}

static std::string bc_mul_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = llen + rlen;
  int result_scale = lscale + rscale;
  int result_size = result_len + result_scale + 3;
  std::string result (result_size, '0');

  int *res = (int *)malloc(sizeof (int) * result_size);
  memset(res, 0, sizeof (int) * result_size);
  for (int i = -lscale; i < llen; i++) {
    int x = (i < 0 ? lhs[lfrac - i - 1] : lhs[ldot - i - 1]) - '0';
    for (int j = -rscale; j < rlen; j++) {
      int y = (j < 0 ? rhs[rfrac - j - 1] : rhs[rdot - j - 1]) - '0';
      res[i + j + result_scale] += x * y;
    }
  }
  for (int i = 0; i + 1 < result_size; i++) {
    res[i + 1] += res[i] / 10;
    res[i] %= 10;
  }

  int cur_pos = result_size;
  for (int i = 0; i < result_scale; i++) {
    result[--cur_pos] = (char)(res[i] + '0');
  }
  resscale = result_size - cur_pos;
  resfrac = cur_pos;
  if (result_scale > 0) {
    result[--cur_pos] = '.';
  }
  resdot = cur_pos;

  for (int i = result_scale; i < result_len + result_scale; i++) {
    result[--cur_pos] = (char)(res[i] + '0');
  }
  resint = cur_pos;
  BC_ASSERT (cur_pos > 0);

  free(res);

  char *data = (char*)malloc((result.length()+1)*sizeof(char));
  sprintf(data, result.c_str());
  std::string ret = bc_round (data, resint, resdot, resfrac, resscale, scale, sign, 0);
  free(data);

  return ret;
}

static std::string bc_div_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot = -1, resfrac = -1, resscale;

  int result_len = std::max (llen + rscale - rlen + 1, 1);
  int result_scale = scale;
  int result_size = result_len + result_scale + 3;

  if (rscale == 0 && rhs[rint] == '0') {
    std::cerr << ("Division by zero in function bcdiv")<< std::endl<< std::endl;
    return ZERO;
  }

  int dividend_len = llen + lscale;
  int divider_len = rlen + rscale;
  int *dividend = (int *)malloc(sizeof (int) * (result_size + dividend_len + divider_len));
  int *divider = (int *)malloc(sizeof (int) * divider_len);
  memset(dividend, 0, sizeof (int) * (result_size + dividend_len + divider_len));
  memset(divider, 0, sizeof (int) * divider_len);

  for (int i = -lscale; i < llen; i++) {
    int x = (i < 0 ? lhs[lfrac - i - 1] : lhs[ldot - i - 1]) - '0';
    dividend[llen - i - 1] = x;
  }

  for (int i = -rscale; i < rlen; i++) {
    int x = (i < 0 ? rhs[rfrac - i - 1] : rhs[rdot - i - 1]) - '0';
    divider[rlen - i - 1] = x;
  }

  int divider_skip = 0;
  while (divider_len > 0 && divider[0] == 0) {
    divider++;
    divider_skip++;
    divider_len--;
  }
  BC_ASSERT (divider_len > 0);

  int cur_pow = llen - rlen + divider_skip;
  int cur_pos = 2;

  if (cur_pow < -scale) {
    divider -= divider_skip;
    divider_len += divider_skip;
    free(dividend);
    free(divider);
    return bc_zero (scale);
  }

  std::string result (result_size, '0');
  resint = cur_pos;
  if (cur_pow < 0) {
    result[cur_pos++] = '0';
    resdot = cur_pos;
    result[cur_pos++] = '.';
    resfrac = cur_pos;
    for (int i = -1; i > cur_pow; i--) {
      result[cur_pos++] = '0';
    }
  }

  int beg = 0, real_beg = 0;
  while (cur_pow >= -scale) {
    char dig = '0';
    while (true) {
      if (real_beg < beg && dividend[real_beg] == 0) {
        real_beg++;
      }

      bool less = false;
      if (real_beg == beg) {
        for (int i = 0; i < divider_len; i++) {
          if (dividend[beg + i] != divider[i]) {
            less = (dividend[beg + i] < divider[i]);
            break;
          }
        }
      }
      if (less) {
        break;
      }

      for (int i = divider_len - 1; i >= 0; i--) {
        dividend[beg + i] -= divider[i];
        if (dividend[beg + i] < 0) {
          dividend[beg + i] += 10;
          dividend[beg + i - 1]--;
        }
      }
      dig++;
    }

    result[cur_pos++] = dig;

    if (cur_pow == 0) {
      resdot = cur_pos;
      if (scale > 0) {
        result[cur_pos++] = '.';
      }
      resfrac = cur_pos;
    }
    cur_pow--;
    beg++;
  }
  resscale = cur_pos - resfrac;

  divider -= divider_skip;
  divider_len += divider_skip;
  free(dividend);
  free(divider);

  char *data = (char*)malloc((result.length()+1)*sizeof(char));
  sprintf(data, result.c_str());
  std::string ret = bc_round (data, resint, resdot, resfrac, resscale, scale, sign, 0);
  free(data);

  return ret;
}


static std::string bc_add (const char *lhs, int lsign, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rsign, int rint, int rdot, int rfrac, int rscale, int scale) {
  if (lsign > 0 && rsign > 0) {
    return bc_add_positive (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, scale, 1);
  }

  if (lsign > 0 && rsign < 0) {
    if (bc_comp (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, 1000000000) >= 0) {
      return bc_sub_positive (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, scale, 1);
    } else {
      return bc_sub_positive (rhs, rint, rdot, rfrac, rscale, lhs, lint, ldot, lfrac, lscale, scale, -1);
    }
  }

  if (lsign < 0 && rsign > 0) {
    if (bc_comp (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, 1000000000) <= 0) {
      return bc_sub_positive (rhs, rint, rdot, rfrac, rscale, lhs, lint, ldot, lfrac, lscale, scale, 1);
    } else {
      return bc_sub_positive (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, scale, -1);
    }
  }

  if (lsign < 0 && rsign < 0) {
    return bc_add_positive (lhs, lint, ldot, lfrac, lscale, rhs, rint, rdot, rfrac, rscale, scale, -1);
  }

  BC_ASSERT (0);
  return ZERO; //Is dummy...
}

void BCMath::bcscale (int scale) {
  if (scale < 0) {
    bc_scale = 0;
  } else {
    bc_scale = scale;
  }
}

std::string BCMath::bcdiv (const std::string &lhs, const std::string &rhs, int scale) {
  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    std::cerr << "Wrong parameter scale = "<<scale<<" in function bcdiv"<< std::endl;
    scale = 0;
  }
  if (lhs.empty()) {
    return bc_zero (scale);
  }
  if (rhs.empty()) {
    std::cerr << "Division by empty "<<  rhs.c_str() <<" in function bcdiv"<< std::endl;
    return bc_zero (scale);
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    std::cerr << "First parameter \""<<  lhs.c_str() <<"\" in function bcdiv is not a number"<< std::endl;
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    std::cerr << "Second parameter \""<< rhs.c_str() <<"\" in function bcdiv is not a number"<< std::endl;
    return ZERO;
  }

  return bc_div_positive (lhs.c_str(), lint, ldot, lfrac, lscale, rhs.c_str(), rint, rdot, rfrac, rscale, scale, lsign * rsign);
}

std::string BCMath::bcmod (const std::string &lhs, const std::string &rhs) {
  if (lhs.empty()) {
    return ZERO;
  }
  if (rhs.empty()) {
    std::cerr << "Modulo by empty "<<  rhs.c_str() <<" in function bcmod"<< std::endl;
    return ZERO;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) != 0) {
    std::cerr << "First parameter \""<< lhs.c_str() <<"\" in function bcmod is not an integer"<< std::endl;
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) != 0) {
    std::cerr << "Second parameter \""<< rhs.c_str() <<"\" in function bcmod is not an integer"<< std::endl;
    return ZERO;
  }

  long long mod = 0;
  for (int i = rint; i < rdot; i++) {
    mod = mod * 10 + rhs[i] - '0';
  }

  if (rdot - rint > 18 || mod == 0) {
    std::cerr << "Second parameter \""<< rhs.c_str() <<"\" in function bcmod is not a non zero integer less than 1e18 by absolute value"<< std::endl;
    return ZERO;
  }

  long long res = 0;
  for (int i = lint; i < ldot; i++) {
    res = res * 2;
    if (res >= mod) {
      res -= mod;
    }
    res = res * 5 + lhs[i] - '0';
    while (res >= mod) {
      res -= mod;
    }
  }

  char buffer[20];
  int cur_pos = 20;
  do {
    buffer[--cur_pos] = (char)(res % 10 + '0');
    res /= 10;
  } while (res > 0);

  if (lsign < 0) {
    buffer[--cur_pos] = '-';
  }

  return std::string(std::string(buffer + cur_pos).substr(0, 20 - cur_pos));
}

std::string BCMath::bcpow (const std::string &lhs, const std::string &rhs) {
  if (lhs.empty()) {
    return ZERO;
  }
  if (rhs.empty()) {
    return ONE;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) != 0) {
    std::cerr << "First parameter \""<<lhs.c_str()<<"\" in function bcpow is not an integer"<< std::endl;
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) != 0) {
    std::cerr << "Second parameter \""<< rhs.c_str() <<"\" in function bcpow is not an integer"<< std::endl;
    return ZERO;
  }

  long long deg = 0;
  for (int i = rint; i < rdot; i++) {
    deg = deg * 10 + rhs[i] - '0';
  }

  if (rdot - rint > 18 || (rsign < 0 && deg != 0)) {
    std::cerr << "Second parameter \""<< rhs.c_str() <<"\" in function bcpow is not a non negative integer less than 1e18"<< std::endl;
    return ZERO;
  }

  if (deg == 0) {
    return ONE;
  }

  std::string result = ONE;
  std::string mul = lhs;
  while (deg > 0) {
    if (deg & 1) {
      result = bcmul (result, mul, 0);
    }
    mul = bcmul (mul, mul, 0);
    deg >>= 1;
  }

  return result;
}

std::string BCMath::bcadd (const std::string &lhs, const std::string &rhs, int scale) {
  if (lhs.empty()) {
    return bcadd (ZERO, rhs, scale);
  }
  if (rhs.empty()) {
    return bcadd (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    std::cerr << "Wrong parameter scale = "<< scale <<" in function bcadd"<< std::endl;
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    std::cerr << "First parameter \""<< lhs.c_str() << "\" in function bcadd is not a number"<< std::endl;
    return bc_zero (scale);
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    std::cerr << "Second parameter \""<< rhs.c_str() << "\" in function bcadd is not a number"<< std::endl;
    return bc_zero (scale);
  }

  return bc_add (lhs.c_str(), lsign, lint, ldot, lfrac, lscale, rhs.c_str(), rsign, rint, rdot, rfrac, rscale, scale);
}

std::string BCMath::bcsub (const std::string &lhs, const std::string &rhs, int scale) {
  if (lhs.empty()) {
    return bcsub (ZERO, rhs, scale);
  }
  if (rhs.empty()) {
    return bcsub (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    std::cerr << "Wrong parameter scale = "<< scale <<" in function bcsub"<< std::endl;
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    std::cerr << "First parameter \""<<lhs.c_str()<<"\" in function bcsub is not a number"<< std::endl;
    return bc_zero (scale);
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    std::cerr << "Second parameter \""<<rhs.c_str()<<"\" in function bcsub is not a number"<< std::endl;
    return bc_zero (scale);
  }

  rsign *= -1;

  return bc_add (lhs.c_str(), lsign, lint, ldot, lfrac, lscale, rhs.c_str(), rsign, rint, rdot, rfrac, rscale, scale);
}

std::string BCMath::bcmul (const std::string &lhs, const std::string &rhs, int scale) {
  if (lhs.empty()) {
    return bcmul (ZERO, rhs, scale);
  }
  if (rhs.empty()) {
    return bcmul (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    std::cerr << "Wrong parameter scale = "<<scale<<" in function bcmul"<< std::endl;
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    std::cerr << "First parameter \""<<lhs.c_str()<<"\" in function bcmul is not a number"<< std::endl;
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    std::cerr << "Second parameter \""<<rhs.c_str()<<"\" in function bcmul is not a number"<< std::endl;
    return ZERO;
  }

  return bc_mul_positive (lhs.c_str(), lint, ldot, lfrac, lscale, rhs.c_str(), rint, rdot, rfrac, rscale, scale, lsign * rsign);
}

int BCMath::bccomp (const std::string &lhs, const std::string &rhs, int scale) {
  if (lhs.empty()) {
    return BCMath::bccomp (ZERO, rhs, scale);
  }
  if (rhs.empty()) {
    return BCMath::bccomp (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    std::cerr << "Wrong parameter scale = "<<scale<<" in function bccomp"<< std::endl;
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    std::cerr << "First parameter \""<<lhs.c_str()<<"\" in function bccomp is not a number"<< std::endl;
    return 0;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    std::cerr << "Second parameter \""<<rhs.c_str()<<"\" in function bccomp is not a number"<< std::endl;
    return 0;
  }

  if (lsign != rsign) {
    return (lsign - rsign) / 2;
  }

  return (1 - 2 * (lsign < 0)) * bc_comp (lhs.c_str(), lint, ldot, lfrac, lscale, rhs.c_str(), rint, rdot, rfrac, rscale, scale);
}

std::string BCMath::bcround (const std::string &lhs, int scale) {
    if (lhs.empty()) {
      return BCMath::bcround (ZERO, scale);
    }

    if (scale == INT_MIN) {
      scale = bc_scale;
    }

    if (scale < 0) {
      std::cerr << "Wrong parameter scale = "<<scale<<" in function bccomp"<< std::endl;
      scale = 0;
    }

    int lsign, lint, ldot, lfrac, lscale;
    if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
      std::cerr << "First parameter \""<<lhs.c_str()<<"\" in function bcround is not a number"<< std::endl;
      return 0;
    }

    int len = lhs.size();
    std::string result(len + 1, '0');
    for(int i = len-1;i>=lint;--i) {
        result[i+1] = lhs[i];
    }

    char *data = (char*)malloc((result.length()+1)*sizeof(char));
    sprintf(data, result.c_str());
    std::string ret = bc_round (data, lint+1, ldot+1, lfrac+1, lscale, scale, lsign, 1, 1);
    free(data);

    return ret;
}
