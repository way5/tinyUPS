/*
#####################################################################################
# File: flog.h                                                                      #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 7:24:53 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Thursday, 7th September 2023 5:47:49 pm                            #
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

const char CHAR_SPRT[] = ": ";
const char CHAR_NL[] = "\n";

class fLogClass {
    public:
        fLogClass(const char * name, size_t size_max = 8192): _path(name), _smax(size_max) {};
        ~fLogClass() {};
        size_t touch();
        // - no timestamp
        size_t put(const char * str, ...);
        // - with timestamp
        size_t putts(const char * str, ...);
        /**
         * @brief Writes a line with [(timestamp-offset);digit[\n]] pair or just a [val;[\n]]
         *
         * @tparam T
         * @param val   data to store in log
         * @param timestamp if true (default) adding the timestamp
         * @param nl    adding a new line
        */
        template <typename T>
        void putdts(T & val, bool timestamp = true, bool nl = true) {
            char * b;
            char * c;
            _CHB(b, 128);
            _CHB(c, 64);
            String mask;
            if(timestamp) {
                ntp.timestampToString(b);
                if(std::is_same<T, float>::value || std::is_same<T, double>::value)
                    mask = String(";%.2f");
                else
                    mask = String(";%d");
            } else {
                if(std::is_same<T, float>::value || std::is_same<T, double>::value)
                    mask = String("%.2f");
                else
                    mask = String("%d");
            }
            if(nl) {
                mask += String("\n");
            } else {
                mask += String(";");
            }
            sprintf(c, mask.c_str(), val);
            strcat(b, c);
            _CHBD(c);
            write(b);
            _CHBD(b);
        };
        // Returns log name
        const char * getName() {
            return _path;
        };

    protected:
        File _f;
        const char * _path;
        size_t _smax = 0;
        size_t open();
        void close();
        bool logRotate(size_t & fs);
        size_t write(const char * str);
};

extern fLogClass logsys;

#endif                          // FLOGCLASS_H
