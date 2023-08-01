/*
#####################################################################################
# File: flog.h                                                                      #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 7:24:53 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 12th July 2023 5:38:29 pm                               #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/* NOTE
    ******** DEVELOPERs' MEMO
    Open file modes:
        r      Open text file for reading.  The stream is positioned at the
                beginning of the file.
        r+     Open for reading and writing.  The stream is positioned at the
                beginning of the file.
        w      Truncate file to zero length or create text file for writing.
                The stream is positioned at the beginning of the file.
        w+     Open for reading and writing.  The file is created if it does
                not exist, otherwise it is truncated.  The stream is
                positioned at the beginning of the file.
        a      Open for appending (writing at end of file).  The file is
                created if it does not exist.  The stream is positioned at the
                end of the file.
        a+     Open for reading and appending (writing at end of file).  The
                file is created if it does not exist.  The initial file
                position for reading is at the beginning of the file, but
                output is always appended to the end of the file.

*/

#ifndef FLOGCLASS_H
#define FLOGCLASS_H

#include <type_traits>
#include "helpers.h"
#include "ntpc.h"
#include "eemem.h"
#include "FS.h"
#include "FFat.h"

const char CHAR_SPRT[] PROGMEM = ": ";
const char CHAR_NL[] PROGMEM = "\n";
//
// CLASS
//
class fLogClass {
    public:
        // void init(const __FlashStringHelper * name, uint16_t size_max = 8192);
        void init(const char * name, size_t size_max = 8192);
        // Applend a line
        // - no timestamp
        size_t put(PGM_P str, ...);
        size_t put(const __FlashStringHelper * str);
        // size_t put(const char * str);
        // - with timestamp
        size_t putts(PGM_P str, ...);
        size_t putts(const __FlashStringHelper * str);
        // size_t putts(const char * str);
        /**
         * @brief Writes a line with [(timestamp-offset);digit[\n]] pair or just a [val;[\n]]
         *
         * @todo offset 2023/01/01 11:59:59 PM +0.00 = (1672574399)
         * @tparam T
         * @param val   data to store in log
         * @param timestamp if true (default) adding the timestamp
         * @param nl    adding a new line
        */
        template <typename T>
        void putdts(T & val, bool timestamp = true, bool nl = true) {
            char * b;
            _CHB(b, 64);
            String mask;
            if(timestamp) {
                ntp.timestampToString(b);
                write(b);
                _CHBC(b);
                if(std::is_same<T, float>::value || std::is_same<T, double>::value)
                    mask = String(PSTR(";%.2f"));
                else
                    mask = String(PSTR(";%d"));
            } else {
                if(std::is_same<T, float>::value || std::is_same<T, double>::value)
                    mask = String(PSTR("%.2f"));
                else
                    mask = String(PSTR("%d"));
            }
            if(nl) {
                mask += String("\n");
            } else {
                mask += String(";");
            }
            sprintf_P(b, mask.c_str(), val);
            write(b);
            _CHBD(b);
        };
        // Returns log name
        const char * getName() {
            return _name;
        };

    protected:
        File _f;
        const char * _name;
        size_t _smax = 0;
        size_t open();
        void close();
        bool logRotate(size_t fs);
        size_t write(const char * str);
};

extern fLogClass sysLog;

#endif                          // FLOGCLASS_H
