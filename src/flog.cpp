/*
#####################################################################################
# File: flog.cpp                                                                    #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 7:24:45 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 26th July 2023 5:05:06 pm                               #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "flog.h"

/**
 * @brief
 *
 * @param name
 * @param size_max
*/
// void fLogClass::init(const __FlashStringHelper * name, uint16_t size_max) {
void fLogClass::init(const char * name, size_t size_max) {
    // PGM_P nm = reinterpret_cast<PGM_P>(name);
    // pgm_str(nm, _name);
    _name = name;
    // _f = LittleFS.open(_name, FILE_APPEND);
    _f = FFat.open(_name, FILE_APPEND);
    _smax = size_max;
#if DEBUG == 1
    __DF("%s ready\n", _name);
#endif
}

/**
 * @brief Do truncate this log file if needed
 *
 * @param fs
 * @return true
 * @return false
*/
bool fLogClass::logRotate(size_t fs) {
    if(_smax == 0) return false;
    if(fs >= _smax) {
        char * bkp_name;
        _CHB(bkp_name, 16);
        strcpy(bkp_name, _name);
        strcat(bkp_name, ".0");
        close();
        // check if log file backup already exists
        if(FFat.exists(bkp_name)) {
            FFat.remove(bkp_name);
        }
        FFat.rename(_name, bkp_name);
        _CHBD(bkp_name);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param str
 * @param ...
 * @return size_t
*/
size_t fLogClass::put(PGM_P str, ...) {
    if(_smax == 0) return 0;
    char * b;
    size_t fs;
    _CHB(b, 256);
    va_list args;
    va_start(args, str);
    vsnprintf_P(b, 256, str, args);
    va_end(args);
    strcat_P(b, CHAR_NL);
    fs = write(b);
    _CHBD(b);

    return fs;
}

/**
 * @brief Writes PSTR into log
 *
 * @param str
 * @param ...
 * @return size_t
*/
size_t fLogClass::putts(PGM_P str, ...) {
    if(_smax == 0) return 0;
    char * b;
    size_t fs;
    _CHB(b, 256);
    // writing timestamp
    ntp.getDatetime(b);
    strcat_P(b, CHAR_SPRT);
    fs = write(b);
    _CHBC(b);
    // writing data
    va_list args;
    va_start(args, str);
    vsnprintf_P(b, 256, str, args);
    va_end(args);
    strcat_P(b, CHAR_NL);
    fs += write(b);
    _CHBD(b);

    return fs;
}

/**
 * @brief
 *
 * @param str
 * @return size_t
*/
size_t fLogClass::put(const __FlashStringHelper * str) {
    if(_smax == 0) return 0;
    size_t fs;
    char * b;
    _CHB(b, 256);
    PGM_P pgm = reinterpret_cast<PGM_P>(str);
    memcpy_P(b, pgm, strlen_P(pgm));
    strcat_P(b, CHAR_NL);
    fs = write(b);
    _CHBD(b);

    return fs;
}

/**
 * @brief Writes F() string into log
 *
 * @param str
 * @return size_t
*/
size_t fLogClass::putts(const __FlashStringHelper * str) {
    if(_smax == 0) return 0;
    char * b;
    _CHB(b, 48);
    // writing timestamp
    ntp.getDatetime(b);
    strcat_P(b, CHAR_SPRT);
    write(b);
    _CHBD(b);

    return put(str);
}

/**
 * @brief
 *
 * @param str
*/
size_t fLogClass::write(const char * str) {
    if(_smax == 0) return 0;
    // open log
    size_t fs = open();
    if(logRotate(fs)) {
        fs = open();
        sysLog.put(PSTR("(i) logrotate: %s"), this->_name);
    }
    // write data
    fs = _f.print(str);
#if DEBUG == 1
    __DF(PSTR("puts to %s: %d chars\n"), _name, fs);
#endif
    // close log
    close();
    delay(100);
    return fs;
}

/**
 * @brief Opening log
 *
 * @return size_t
*/
size_t fLogClass::open() {
    if(_smax == 0) return 0;
    _f = FFat.open(_name, "a");
    return _f.size();
}

/**
 * @brief Closing log
 *
*/
void fLogClass::close() {
    if(_smax == 0) return;
    // _f.flush();
    _f.close();
}