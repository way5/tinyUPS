/*
#####################################################################################
# File: helpers.cpp                                                                 #
# Project: tinyUPS                                                                  #
# File Created: Sunday, 29th May 2022 2:02:52 am                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 4th July 2023 10:01:39 pm                                 #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "helpers.h"

/**
 * @brief Converts floating point value to string
 *        and APPENDS it to the buffer
 *
 * @param val
 * @param buffer
 * @param prec
 */
void val2str(float val, char *buffer, uint8_t prec)
{
    uint16_t int1 = 0;
    uint16_t fract1 = 0;
    uint8_t blen = strlen(buffer);
    float exp = 0;
    char *b0;
    _CHB(b0, 24);
    char point = '.';
    uint8_t cntr1 = 0;
    uint8_t negative = 0;
    // check if negative
    if (val < 0)
    {
        negative = 1;
        // *buffer = '-';
        val *= -1;
        // buffer++;
    }
    // fraction length
    exp = pow(10, prec);
    // separate the number
    int1 = (uint16_t)val;
    fract1 = (uint16_t)((val - (float)int1) * exp);
    // fraction part
    while (prec)
    {
        if (fract1 != 0)
        {
            b0[cntr1] = (fract1 % 10) + '0';
            fract1 /= 10;
        }
        else
        {
            b0[cntr1] = '0';
        }
        prec--;
        cntr1++;
    }
    // point
    b0[cntr1] = point;
    cntr1++;
    // integer part
    if (int1 != 0)
    {
        while (int1)
        {
            b0[cntr1] = (int1 % 10) + '0';
            int1 /= 10;
            cntr1++;
        }
    }
    else
    {
        b0[cntr1] = '0';
        cntr1++;
    }
    if (negative)
    {
        b0[cntr1] = '-';
        cntr1++;
    }
    while (cntr1)
    {
        *(buffer + blen) = b0[(cntr1 - 1)];
        blen++;
        cntr1--;
    }
    _CHBD(b0);
}

/**
 * @brief Converts val to string and
 *        APPENDS it to buffer
 *
 * @param str
 * @param buffer
 */
void val2str(long val, char *buffer)
{
    uint8_t cntr1 = 0;
    uint8_t negative = 0;
    uint8_t blen = strlen(buffer);
    char *b0;
    _CHB(b0, 24);
    if (val != 0)
    {
        if (val < 0)
        {
            negative = 1;
            val *= -1;
        }
        // b0 contains string characters in reversed order
        while (val)
        {
            b0[cntr1] = (val % 10) + '0';
            val /= 10;
            cntr1++;
        }
        if (negative)
        {
            b0[cntr1] = '-';
            cntr1++;
        }
        while (cntr1)
        {
            *(buffer + blen) = b0[(cntr1 - 1)];
            blen++;
            cntr1--;
        }
    }
    else
    {
        *buffer = '0';
    }
    _CHBD(b0);
}

/**
 * @brief
 *
 * @param val
 * @param buffer
 */
void val2str(unsigned int val, char *buffer)
{
    val2str(static_cast<long>(val), buffer);
}

/**
 * @brief
 *
 * @param val
 * @param buffer
 */
void val2str(int val, char *buffer)
{
    val2str(static_cast<long>(val), buffer);
}

/**
 * @brief
 *
 * @param val
 * @param buffer
 */
void val2str(unsigned long val, char *buffer)
{
    val2str(static_cast<long>(val), buffer);
}

/**
 * @brief Converts char array into digit (NOT FOR FLOATs)
 *
 * @param buffer
 * @param val
 */
void str2val(char *buffer, long *val)
{
    int16_t slen = (int16_t)strlen(buffer);
    uint32_t mult = 1;
    *val = 0;
    // process buffer
    while (slen >= 0)
    {
        // if negative
        if (*(buffer + slen) == '-')
        {
            *val *= -1;
            break;
        }
        if (*(buffer + slen) != '\0')
        {
            *val += (*(buffer + slen) - '0') * mult;
            mult *= 10;
        }
        slen--;
    }
}

void str2val(char *buffer, unsigned long *val)
{
    str2val(buffer, reinterpret_cast<long *>(val));
}

void str2val(char *buffer, int *val)
{
    str2val(buffer, reinterpret_cast<long *>(val));
}

void str2val(char *buffer, unsigned int *val)
{
    str2val(buffer, reinterpret_cast<long *>(val));
}

/**
 * @brief Read value from PROGMEM to an empty
 *        or append to existing buffer
 *
 * @param data
 * @param buffer
 */
void pgm_str(const char *data, char *buffer, bool append)
{
    uint8_t l = strlen_P(data);
    if (l != 0)
    {
        uint8_t bdata_length = strlen(buffer);
        if (!append)
        {
            memset(buffer, '\0', bdata_length);
            bdata_length = 0;
        }
        uint8_t cntr = 0;
        while (cntr < l)
        {
            *(buffer + (bdata_length + cntr)) = (char)pgm_read_byte_near((data + cntr));
            cntr++;
        }
    }
}

/**
 * @brief Helper for date conversion functions
 *
 * @param str
 * @param d
 * @param m
 * @param y
 */
static inline void _helper_str2dt(const char *str, char *d, char *m, char *y)
{
    uint8_t c = 0;
    uint8_t i = 0;
    while (c < 6)
    {
        if (c >= 0 && c < 2)
        {
            *(m + i) = str[c];
        }
        else if (c >= 2 && c < 4)
        {
            *(d + i) = str[c];
        }
        else
        {
            *(y + i) = str[c];
        }
        c++;
        i = (i == 1 ? 0 : (i + 1));
    }
}

/**
 * @brief Converts stringified date to formatted string human readable
 *       ex: 010220 (mmddyy) -> 2020-01-02
 *
 * @param str
 * @param buffer
 */
void str2dt(const char *str, char *buffer)
{
    char const *s = str;
    char y[3] = "";
    char m[3] = "";
    char d[3] = "";
    _helper_str2dt(s, d, m, y);
    sprintf(buffer, "20%s-%s-%s", y, m, d);
}

/**
 * @brief Converts stringified date to SNMP format
 *       ex: 010220 (mmddyy) -> 01/02/20
 *
 * @param str
 * @param buffer
 */
void str2snmpdt(const char *str, char *buffer)
{
    char const *s = str;
    char y[3] = "";
    char m[3] = "";
    char d[3] = "";
    _helper_str2dt(s, d, m, y);
    sprintf(buffer, "%s/%s/%s", m, d, y);
}

/**
 * @brief Converts formatted data string to format mmddyy
 *       ex: 2020-01-02 -> 010220 (mmddyy)
 *
 * @param dt
 * @param buffer
 */
void dt2str(const char *dt, char *buffer)
{
    uint8_t c = 2;
    uint8_t i = 0;
    char y[3] = "";
    char m[3] = "";
    char d[3] = "";
    while (c < 10)
    {
        if (*(dt + c) == '-')
        {
            c++;
            continue;
        }
        if (c >= 2 && c < 4)
        {
            *(y + i) = dt[c];
        }
        else if (c >= 5 && c < 7)
        {
            *(m + i) = dt[c];
        }
        else
        {
            *(d + i) = dt[c];
        }
        c++;
        i = (i == 1 ? 0 : (i + 1));
    }
    // dummy check
    if (!std::isdigit((int)*m) || !std::isdigit((int)*d) || !std::isdigit((int)*y))
        return;
    sprintf(buffer, "%s%s%s", m, d, y);
}