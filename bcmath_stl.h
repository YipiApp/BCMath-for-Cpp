#ifndef BCMATH_H
#define BCMATH_H

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

    URL: https://github.com/vk-com/kphp-kdb/blob/master/KPHP/runtime/bcmath.h

    Adapted to STL C++:
        2014 Kijam Lopez B. <klopez@cuado.co>

*/

#include<string>
#include<iostream>
#include<sstream>

class BCMath {

public:
    BCMath() { }
    BCMath(const BCMath &o) : value(o.value) { }
    BCMath(const char *num) : value(num) {  }
    BCMath(std::string num) : value(num) {  }
    BCMath(int num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(long long num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(unsigned int num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(unsigned long long num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(float num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(double num) : value() { std::stringstream ss; ss << num; value = ss.str(); }
    BCMath(long double num) { std::stringstream ss; ss << num; value = ss.str(); }

    BCMath operator+(const BCMath& o) {
        return BCMath::bcadd(value, o.value);
    }

    BCMath operator-(const BCMath& o) {
        return BCMath::bcsub(value, o.value);
    }

    BCMath operator*(const BCMath& o) {
        return BCMath::bcmul(value, o.value);
    }

    BCMath operator/(const BCMath& o) {
        return BCMath::bcdiv(value, o.value);
    }

    BCMath operator%(const BCMath& o) {
        return BCMath::bcmod(value, o.value);
    }

    BCMath operator^(const BCMath& o) {
        return BCMath::bcpow(value, o.value);
    }

    void operator+=(const BCMath& o) {
        value = BCMath::bcadd(value, o.value);
    }
    void operator-=(const BCMath& o) {
        value = BCMath::bcsub(value, o.value);
    }
    void operator*=(const BCMath& o) {
        value = BCMath::bcmul(value, o.value);
    }
    void operator/=(const BCMath& o) {
        value = BCMath::bcdiv(value, o.value);
    }
    void operator^=(const BCMath& o) {
        value = BCMath::bcpow(value, o.value);
    }

    bool operator > (const BCMath& o) {
        return BCMath::bccomp(value, o.value)>0;
    }
    bool operator >= (const BCMath& o) {
        return BCMath::bccomp(value, o.value)>=0;
    }
    bool operator == (const BCMath& o) {
        return BCMath::bccomp(value, o.value)==0;
    }
    bool operator < (const BCMath& o) {
        return BCMath::bccomp(value, o.value)<0;
    }
    bool operator <= (const BCMath& o) {
        return BCMath::bccomp(value, o.value)<=0;
    }

    int toInt() {
        std::istringstream buffer(value);
        int ret;
        buffer >> ret;
        return ret;
    }

    unsigned int toUInt() {
        std::istringstream buffer(value);
        unsigned int ret;
        buffer >> ret;
        return ret;
    }

    long long toLongLong() {
        std::istringstream buffer(value);
        long long ret;
        buffer >> ret;
        return ret;
    }

    unsigned long long toULongLong() {
        std::istringstream buffer(value);
        unsigned long long ret;
        buffer >> ret;
        return ret;
    }

    long double toLongDouble() {
        std::istringstream buffer(value);
        long double ret;
        buffer >> ret;
        return ret;
    }

    double toDouble() {
        std::istringstream buffer(value);
        double ret;
        buffer >> ret;
        return ret;
    }

    float toFloat() {
        std::istringstream buffer(value);
        float ret;
        buffer >> ret;
        return ret;
    }

    std::string toString() {
        return value;
    }

    void round(int scale) {
        if(scale>=1)
            value = BCMath::bcround(value, scale);
    }

    std::string getIntPart() {
        std::size_t dot = value.find('.');
        if(dot != std::string::npos) {
            if(dot == 0)
                return std::string("0");
            if(dot == 1 && value[0] == '-')
                return std::string("-0");
            return value.substr(0, dot);
        }else{
            return value;
        }
    }

    std::string getDecPart() {
        std::size_t dot = value.find('.');
        if(dot != std::string::npos)
            return value.length()>dot+1?value.substr(dot+1):std::string("0");
        else
            return std::string("0");
    }

private:
    std::string value;

public:
    static void bcscale (int scale);

    static std::string bcdiv (const std::string &lhs, const std::string &rhs, int scale = INT_MIN);

    static std::string bcmod (const std::string &lhs, const std::string &rhs);

    static std::string bcpow (const std::string &lhs, const std::string &rhs);

    static std::string bcadd (const std::string &lhs, const std::string &rhs, int scale = INT_MIN);

    static std::string bcsub (const std::string &lhs, const std::string &rhs, int scale = INT_MIN);

    static std::string bcmul (const std::string &lhs, const std::string &rhs, int scale = INT_MIN);

    static std::string bcround (const std::string &lhs, int scale = INT_MIN);

    static int bccomp (const std::string &lhs, const std::string &rhs, int scale = INT_MIN);

};


#endif // BCMATH_H
