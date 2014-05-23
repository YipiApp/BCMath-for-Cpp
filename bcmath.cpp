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
*/


#include "bcmath.h"

static int bc_scale = 6;

static const QString ONE ("1");
static const QString ZERO ("0");

//parse a number into parts, returns scale on success and -1 on error
static int bc_parse_number (const QString &s, int &lsign, int &lint, int &ldot, int &lfrac, int &lscale) {
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
    Q_ASSERT (lfrac == ldot);
  }

  if (lsign < 0 && (lscale == 0 && s[lint] == '0')) {
    lsign = 1;
  }
  return lscale;
}

static QString bc_zero (int scale) {
  if (scale == 0) {
    return ZERO;
  }
  QString result (scale + 2, '0');
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

static QString bc_round (char *lhs, int lint, int ldot, int lfrac, int lscale, int scale, int sign, int add_trailing_zeroes) {
  while (lhs[lint] == '0' && lint + 1 < ldot) {
    lint++;
  }

  Q_ASSERT (lint > 0 && lscale >= 0 && scale >= 0);

  if (sign < 0 && lhs[lint] == '0') {
    sign = 1;
    for (int i = 0; i < lscale; i++) {
      if (lhs[lfrac + i] != '0') {
        sign = -1;
        break;
      }
    }
  }

  if (lscale > scale) {
    lscale = scale;
  }

/*
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
        Q_ASSERT (lfrac == ldot);

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
*/

  if (lscale == 0 && lfrac > ldot) {
    lfrac--;
    Q_ASSERT (lfrac == ldot);
  }

  if (sign < 0) {
    lhs[--lint] = '-';
  }

  if (lscale == scale || !add_trailing_zeroes) {
    return QString (QString(lhs + lint).mid(0, lfrac + lscale - lint));
  } else {
    QString result (QString(lhs + lint).mid(0, lfrac + lscale - lint));
    if (lscale == 0) {
      result.append('.');
    }
    for(int kI = 0; kI < scale - lscale; ++kI)
        result.append('0');
    return result;
  }
}

static QString bc_add_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = std::max (llen, rlen) + 1;
  int result_scale = std::max (lscale, rscale);
  int result_size = result_len + result_scale + 3;
  QString result(result_size, '0');

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
  Q_ASSERT (cur_pos > 0);

  return bc_round ((char*)result.toUtf8().data(), resint, resdot, resfrac, resscale, scale, sign, 1);
}

static QString bc_sub_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = llen;
  int result_scale = std::max (lscale, rscale);
  int result_size = result_len + result_scale + 3;
  QString result (result_size, '0');

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
  Q_ASSERT (cur_pos > 0);

  return bc_round ((char*)result.toUtf8().data(), resint, resdot, resfrac, resscale, scale, sign, 1);
}

static QString bc_mul_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot, resfrac, resscale;

  int result_len = llen + rlen;
  int result_scale = lscale + rscale;
  int result_size = result_len + result_scale + 3;
  QString result (result_size, '0');

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
  Q_ASSERT (cur_pos > 0);

  free(res);

  return bc_round ((char*)result.toUtf8().data(), resint, resdot, resfrac, resscale, scale, sign, 0);
}

static QString bc_div_positive (const char *lhs, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rint, int rdot, int rfrac, int rscale, int scale, int sign) {
  int llen = ldot - lint;
  int rlen = rdot - rint;

  int resint, resdot = -1, resfrac = -1, resscale;

  int result_len = std::max (llen + rscale - rlen + 1, 1);
  int result_scale = scale;
  int result_size = result_len + result_scale + 3;

  if (rscale == 0 && rhs[rint] == '0') {
    qWarning() << ("Division by zero in function bcdiv");
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
  Q_ASSERT (divider_len > 0);

  int cur_pow = llen - rlen + divider_skip;
  int cur_pos = 2;

  if (cur_pow < -scale) {
    divider -= divider_skip;
    divider_len += divider_skip;
    free(dividend);
    free(divider);
    return bc_zero (scale);
  }

  QString result (result_size, '0');
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

  return bc_round ((char*)result.toUtf8().data(), resint, resdot, resfrac, resscale, scale, sign, 0);
}


static QString bc_add (const char *lhs, int lsign, int lint, int ldot, int lfrac, int lscale, const char *rhs, int rsign, int rint, int rdot, int rfrac, int rscale, int scale) {
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

  Q_ASSERT (0);
}

void QBCMath::bcscale (int scale) {
  if (scale < 0) {
    bc_scale = 0;
  } else {
    bc_scale = scale;
  }
}

QString QBCMath::bcdiv (const QString &lhs, const QString &rhs, int scale) {
  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    qWarning() << ("Wrong parameter scale = %d in function bcdiv", scale);
    scale = 0;
  }
  if (lhs.isEmpty()) {
    return bc_zero (scale);
  }
  if (rhs.isEmpty()) {
    qWarning() << ("Division by empty QString in function bcdiv");
    return bc_zero (scale);
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    qWarning() << "First parameter \""<<  lhs.toStdString().c_str() <<"\" in function bcdiv is not a number";
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() <<"\" in function bcdiv is not a number";
    return ZERO;
  }

  return bc_div_positive (lhs.toStdString().c_str(), lint, ldot, lfrac, lscale, rhs.toStdString().c_str(), rint, rdot, rfrac, rscale, scale, lsign * rsign);
}

QString QBCMath::bcmod (const QString &lhs, const QString &rhs) {
  if (lhs.isEmpty()) {
    return ZERO;
  }
  if (rhs.isEmpty()) {
    qWarning() << ("Modulo by empty QString in function bcmod");
    return ZERO;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) != 0) {
    qWarning() << "First parameter \""<< lhs.toStdString().c_str() <<"\" in function bcmod is not an integer";
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) != 0) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() <<"\" in function bcmod is not an integer";
    return ZERO;
  }

  long long mod = 0;
  for (int i = rint; i < rdot; i++) {
    mod = mod * 10 + rhs[i].cell() - '0';
  }

  if (rdot - rint > 18 || mod == 0) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() <<"\" in function bcmod is not a non zero integer less than 1e18 by absolute value";
    return ZERO;
  }

  long long res = 0;
  for (int i = lint; i < ldot; i++) {
    res = res * 2;
    if (res >= mod) {
      res -= mod;
    }
    res = res * 5 + lhs[i].cell() - '0';
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

  return QString(QString(buffer + cur_pos).mid(0, 20 - cur_pos));
}

QString QBCMath::bcpow (const QString &lhs, const QString &rhs) {
  if (lhs.isEmpty()) {
    return ZERO;
  }
  if (rhs.isEmpty()) {
    return ONE;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) != 0) {
    qWarning() << "First parameter \""<<lhs.toStdString().c_str()<<"\" in function bcpow is not an integer";
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) != 0) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() <<"\" in function bcpow is not an integer";
    return ZERO;
  }

  long long deg = 0;
  for (int i = rint; i < rdot; i++) {
    deg = deg * 10 + rhs[i].cell() - '0';
  }

  if (rdot - rint > 18 || (rsign < 0 && deg != 0)) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() <<"\" in function bcpow is not a non negative integer less than 1e18";
    return ZERO;
  }

  if (deg == 0) {
    return ONE;
  }

  QString result = ONE;
  QString mul = lhs;
  while (deg > 0) {
    if (deg & 1) {
      result = bcmul (result, mul, 0);
    }
    mul = bcmul (mul, mul, 0);
    deg >>= 1;
  }

  return result;
}

QString QBCMath::bcadd (const QString &lhs, const QString &rhs, int scale) {
  if (lhs.isEmpty()) {
    return bcadd (ZERO, rhs, scale);
  }
  if (rhs.isEmpty()) {
    return bcadd (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    qWarning() << "Wrong parameter scale = "<< scale <<" in function bcadd";
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    qWarning() << "First parameter \""<< lhs.toStdString().c_str() << "\" in function bcadd is not a number";
    return bc_zero (scale);
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    qWarning() << "Second parameter \""<< rhs.toStdString().c_str() << "\" in function bcadd is not a number";
    return bc_zero (scale);
  }

  return bc_add (lhs.toStdString().c_str(), lsign, lint, ldot, lfrac, lscale, rhs.toStdString().c_str(), rsign, rint, rdot, rfrac, rscale, scale);
}

QString QBCMath::bcsub (const QString &lhs, const QString &rhs, int scale) {
  if (lhs.isEmpty()) {
    return bcsub (ZERO, rhs, scale);
  }
  if (rhs.isEmpty()) {
    return bcsub (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    qWarning() << "Wrong parameter scale = "<< scale <<" in function bcsub";
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    qWarning() << "First parameter \""<<lhs.toStdString().c_str()<<"\" in function bcsub is not a number";
    return bc_zero (scale);
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    qWarning() << "Second parameter \""<<rhs.toStdString().c_str()<<"\" in function bcsub is not a number";
    return bc_zero (scale);
  }

  rsign *= -1;

  return bc_add (lhs.toStdString().c_str(), lsign, lint, ldot, lfrac, lscale, rhs.toStdString().c_str(), rsign, rint, rdot, rfrac, rscale, scale);
}

QString QBCMath::bcmul (const QString &lhs, const QString &rhs, int scale) {
  if (lhs.isEmpty()) {
    return bcmul (ZERO, rhs, scale);
  }
  if (rhs.isEmpty()) {
    return bcmul (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    qWarning() << "Wrong parameter scale = "<<scale<<" in function bcmul";
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    qWarning() << "First parameter \""<<lhs.toStdString().c_str()<<"\" in function bcmul is not a number";
    return ZERO;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    qWarning() << "Second parameter \""<<rhs.toStdString().c_str()<<"\" in function bcmul is not a number";
    return ZERO;
  }

  return bc_mul_positive (lhs.toStdString().c_str(), lint, ldot, lfrac, lscale, rhs.toStdString().c_str(), rint, rdot, rfrac, rscale, scale, lsign * rsign);
}

int QBCMath::bccomp (const QString &lhs, const QString &rhs, int scale) {
  if (lhs.isEmpty()) {
    return QBCMath::bccomp (ZERO, rhs, scale);
  }
  if (rhs.isEmpty()) {
    return QBCMath::bccomp (lhs, ZERO, scale);
  }

  if (scale == INT_MIN) {
    scale = bc_scale;
  }
  if (scale < 0) {
    qWarning() << "Wrong parameter scale = "<<scale<<" in function bccomp";
    scale = 0;
  }

  int lsign, lint, ldot, lfrac, lscale;
  if (bc_parse_number (lhs, lsign, lint, ldot, lfrac, lscale) < 0) {
    qWarning() << "First parameter \""<<lhs.toStdString().c_str()<<"\" in function bccomp is not a number";
    return 0;
  }

  int rsign, rint, rdot, rfrac, rscale;
  if (bc_parse_number (rhs, rsign, rint, rdot, rfrac, rscale) < 0) {
    qWarning() << "Second parameter \""<<rhs.toStdString().c_str()<<"\" in function bccomp is not a number";
    return 0;
  }

  if (lsign != rsign) {
    return (lsign - rsign) / 2;
  }

  return (1 - 2 * (lsign < 0)) * bc_comp (lhs.toStdString().c_str(), lint, ldot, lfrac, lscale, rhs.toStdString().c_str(), rint, rdot, rfrac, rscale, scale);
}
