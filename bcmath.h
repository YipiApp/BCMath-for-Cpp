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

    Adapted to QT5:
        2014 Kijam Lopez B. <klopez@cuado.co>

*/

#include<QString>
#include<QDebug>
#include<iostream>
#include<sstream>

class QBCMath {

public:
    QBCMath() { }
    QBCMath(const QBCMath &o) : value(o.value) { }
    QBCMath(const char *num) : value(num) {  }
    QBCMath(QString num) : value(num) {  }
    QBCMath(qint32 num) : value(QString::number(num)) {  }
    QBCMath(qint64 num) : value(QString::number(num)) {  }
    QBCMath(quint32 num) : value(QString::number(num)) {  }
    QBCMath(quint64 num) : value(QString::number(num)) {  }
    QBCMath(float num) : value(QString::number((double)num)) {  }
    QBCMath(double num) : value(QString::number(num)) {  }
    QBCMath(long double num) { std::stringstream ss; ss << num; value = QString::fromStdString(ss.str()); }

    QBCMath operator+(const QBCMath& o) {
        return QBCMath::bcadd(value, o.value);
    }

    QBCMath operator-(const QBCMath& o) {
        return QBCMath::bcsub(value, o.value);
    }

    QBCMath operator*(const QBCMath& o) {
        return QBCMath::bcmul(value, o.value);
    }

    QBCMath operator/(const QBCMath& o) {
        return QBCMath::bcdiv(value, o.value);
    }

    QBCMath operator%(const QBCMath& o) {
        return QBCMath::bcmod(value, o.value);
    }

    QBCMath operator^(const QBCMath& o) {
        return QBCMath::bcpow(value, o.value);
    }

    void operator+=(const QBCMath& o) {
        value = QBCMath::bcadd(value, o.value);
    }
    void operator-=(const QBCMath& o) {
        value = QBCMath::bcsub(value, o.value);
    }
    void operator*=(const QBCMath& o) {
        value = QBCMath::bcmul(value, o.value);
    }
    void operator/=(const QBCMath& o) {
        value = QBCMath::bcdiv(value, o.value);
    }
    void operator^=(const QBCMath& o) {
        value = QBCMath::bcpow(value, o.value);
    }

    bool operator > (const QBCMath& o) {
        return QBCMath::bccomp(value, o.value)>0;
    }
    bool operator >= (const QBCMath& o) {
        return QBCMath::bccomp(value, o.value)>=0;
    }
    bool operator == (const QBCMath& o) {
        return QBCMath::bccomp(value, o.value)==0;
    }
    bool operator < (const QBCMath& o) {
        return QBCMath::bccomp(value, o.value)<0;
    }
    bool operator <= (const QBCMath& o) {
        return QBCMath::bccomp(value, o.value)<=0;
    }

    qint32 toInt() {
        return value.toInt();
    }

    quint32 toUInt() {
        return value.toUInt();
    }

    qint64 toLongLong() {
        return value.toLongLong();
    }

    quint64 toULongLong() {
        return value.toULongLong();
    }

    long double toLongDouble() {
        return ::strtold((char*)value.toStdString().c_str(), NULL);
    }

    double toDouble() {
        return value.toDouble();
    }

    float toFloat() {
        return value.toDouble();
    }

    QString toString() {
        return value;
    }

    void round(int scale) {
        if(scale>=1)
            value = QBCMath::bcround(value, scale);
    }

    QString getIntPart() {
        int dot = value.indexOf('.');
        if(dot >= 0) {
            if(dot == 0)
                return QString("0");
            if(dot == 1 && value[0] == '-')
                return QString("-0");
            return value.mid(0, dot);
        }else{
            return value;
        }
    }

    QString getDecPart() {
        int dot = value.indexOf('.');
        if(dot >= 0)
            return value.length()>dot+1?value.mid(dot+1):QString("0");
        else
            return QString("0");
    }

private:
    QString value;

public:
    static void bcscale (int scale);

    static QString bcdiv (const QString &lhs, const QString &rhs, int scale = INT_MIN);

    static QString bcmod (const QString &lhs, const QString &rhs);

    static QString bcpow (const QString &lhs, const QString &rhs);

    static QString bcadd (const QString &lhs, const QString &rhs, int scale = INT_MIN);

    static QString bcsub (const QString &lhs, const QString &rhs, int scale = INT_MIN);

    static QString bcmul (const QString &lhs, const QString &rhs, int scale = INT_MIN);

    static QString bcround (const QString &lhs, int scale = INT_MIN);

    static int bccomp (const QString &lhs, const QString &rhs, int scale = INT_MIN);

};


#endif // BCMATH_H
