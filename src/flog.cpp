/*
#####################################################################################
# File: flog.cpp                                                                    #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 7:24:45 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 1:35:44 pm                              #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "flog.h"

/**
 * @brief Should run (optionally) just after the FS has been initialized
 *        in order to assure the file exists at all times.
 *
 * @return size_t
*/
size_t fLogClass::touch() {
    // beating FR_NO_FILE exception. an easy way.
    if(_smax == 0) return 0;
    size_t _fs = this->open();
    delay(100);
    this->close();
    return _fs;
}

/**
 * @brief
 *
 * @param str
 * @param ...
 * @return size_t
*/
size_t fLogClass::put(const char * str, ...) {
    if(_smax == 0) return 0;
    char * b;
    size_t fs;
    _CHB(b, 256);
    va_list args;
    va_start(args, str);
    vsnprintf(b, 256, str, args);
    va_end(args);
    strcat(b, CHAR_NL);
    fs = this->write(b);
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
size_t fLogClass::putts(const char * str, ...) {
    if(_smax == 0) return 0;
    char * b;
    size_t fs;
    _CHB(b, 256);
    // writing timestamp
    ntp.getDatetime(b);
    strcat(b, CHAR_SPRT);
    fs = this->write(b);
    _CHBC(b);
    // writing data
    va_list args;
    va_start(args, str);
    vsnprintf(b, 256, str, args);
    va_end(args);
    strcat(b, CHAR_NL);
    fs += this->write(b);
    _CHBD(b);

    return fs;
}

/**
 * @brief Do truncate this log file if needed
 *
 * @param fs
 * @return true
 * @return false
*/
bool fLogClass::logRotate(size_t & fs) {
    fs = this->open();
    if(fs >= _smax) {
        char * bkp_path;
        _CHB(bkp_path, 16);
        strcpy(bkp_path, _path);
        strcat(bkp_path, ".0");
        this->close();
        // check if log file backup already exists
        if(FFat.exists(bkp_path)) {
            FFat.remove(bkp_path);
        }
        FFat.rename(_path, bkp_path);
        _CHBD(bkp_path);
        fs = this->open();
        return true;
    }
    return false;
}

/**
 * @brief Writes string into file(_path) and returns the number of characters written
 *
 * @param str
*/
size_t fLogClass::write(const char * str) {
    size_t fs;
    if(logRotate(fs)) {
        logsys.put("(i) logrotate: %s", this->_path);
    }
    fs = _f.print(str);
#if DEBUG == 1
    __DF("puts to %s: %d chars\n", _path, fs);
#endif
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
    _f = FFat.open(_path, FILE_APPEND);
    return _f.size();
}

/**
 * @brief Closing log
 *
*/
void fLogClass::close() {
    _f.close();
}