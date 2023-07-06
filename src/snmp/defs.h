#ifndef SNMP_DEFS_h
#define SNMP_DEFS_h

#include <stdint.h>
#include <functional>
#include "helpers.h"
#include "flog.h"

extern fLogClass snmpLog;

typedef enum {
    SNMP_NO_UDP = -10,
    SNMP_REQUEST_TOO_LARGE = -5,
    SNMP_REQUEST_INVALID = -4,
    SNMP_REQUEST_INVALID_COMMUNITY = -3,
    SNMP_FAILED_SERIALISATION = -2,
    SNMP_GENERIC_ERROR = -1,
    SNMP_NO_PACKET = 0,
    SNMP_NO_ERROR = 1,
    SNMP_GET_OCCURRED = 2,
    SNMP_GETNEXT_OCCURRED = 3,
    SNMP_GETBULK_OCCURRED = 4,
    SNMP_SET_OCCURRED = 5,
    SNMP_ERROR_PACKET_SENT = 6, // A packet indicating that an error occurred was sent
    SNMP_INFORM_RESPONSE_OCCURRED = 7,
    SNMP_UNKNOWN_PDU_OCCURRED = 8
} SNMP_ERROR_RESPONSE;

typedef unsigned long snmp_request_id_t;

#define INVALID_SNMP_REQUEST_ID 0

typedef enum SnmpVersionEnum {
    SNMP_VERSION_1,
    SNMP_VERSION_2C,
    SNMP_VERSION_MAX
} SNMP_VERSION;

typedef enum {
     SNMP_PERM_NONE,
     SNMP_PERM_READ_ONLY,
     SNMP_PERM_READ_WRITE
} SNMP_PERMISSION;

#define SNMP_MAX_PACKET_LENGTH 1400
#define SNMP_OCTET_MAX_LENGTH 500

#define SNMP_ERROR_OK 1

#define SNMP_PACKET_PARSE_ERROR_OFFSET -20
#define SNMP_BUFFER_PARSE_ERROR_OFFSET -10
#define SNMP_BUFFER_ENCODE_ERROR_OFFSET -30

typedef enum ERROR_STATUS_WITH_VALUE {
    // V1 Errors
    NO_ERROR = 0,
    TOO_BIG = 1,
    NO_SUCH_NAME = 2,
    BAD_VALUE = 3,
    READ_ONLY = 4,
    GEN_ERR = 5,
        
    // V2c Errors
    NO_ACCESS = 6,
    WRONG_TYPE = 7,
    WRONG_LENGTH = 8,
    WRONG_ENCODING = 9,
    WRONG_VALUE = 10,
    NO_CREATION = 11,
    INCONSISTENT_VALUE = 12,
    RESOURCE_UNAVAILABLE = 13,
    COMMIT_FAILED = 14,
    UNDO_FAILED = 15,
    AUTHORIZATION_ERROR = 16,
    NOT_WRITABLE = 17,
    INCONSISTENT_NAME = 18
} SNMP_ERROR_STATUS;

#define SNMP_V1_MAX_ERROR GEN_ERR

#define SNMP_ERROR_VERSION_CTRL(error, version) (((version) == SNMP_VERSION_1 && (error) > SNMP_V1_MAX_ERROR) ? SNMP_V1_MAX_ERROR : (error))

// Used for situations where in V2 an error exists but in V1 a less-specific error exists that isn't GEN_ERR
#define SNMP_ERROR_VERSION_CTRL_DEF(error, version, elseError) (((version) == SNMP_VERSION_1 && (error) > SNMP_V1_MAX_ERROR) ? SNMP_ERROR_VERSION_CTRL(elseError, version) : error)

// CALLBACKS
typedef std::function<int (void)> cbfGetInteger;
typedef std::function<void (int val)> cbfSetInteger;
typedef std::function<uint32_t (void)> cbfGetTimestamp;
typedef std::function<void (uint32_t val)> cbfSetTimestamp;
typedef std::function<std::string(void)> cbfString;
typedef std::function<char *(void)> cbfGetString;
typedef std::function<void (const char * str, size_t length)> cbfSetString;
typedef std::function<char *(void)> cbfGetOID;
typedef std::function<void (const char * str, size_t length)> cbfSetOID;
// buffer must be initialiized before
typedef std::function<void (uint8_t * &buffer, size_t &blength)> cbfGetOpaque;
// data shall be encoded to binary octets inside this method
typedef std::function<void (uint8_t * val, size_t &length)> cbfSetOpaque;
typedef std::function<uint32_t (void)> cbfGetGauge;
typedef std::function<void (uint32_t val)> cbfSetGauge;
typedef std::function<uint32_t (void)> cbfGetCounter32;
typedef std::function<void (uint32_t val)> cbfSetCounter32;
typedef std::function<uint64_t (void)> cbfGetCounter64;
typedef std::function<void (uint64_t val)> cbfSetCounter64;

/**
 * @brief Converts status code to string
 *        See: upsBasicOutputStatus
 * 
 * @param code 
 * @param buffer 
*/
inline std::string upsOutputStatusCodeToString(uint8_t &code) {
    std::string str;
    switch(code) {
        default:
        case 1:
            str = "unknown";
            break;
        case 2:
            str = "on line";
            break;
        case 3:
            str = "on battery";
            break;
        case 4:
            str = "on smart boost";
            break;
        case 5:
            str = "tiimed sleeping";
            break;
        case 6:
            str = "software bypass";
            break;
        case 7:
            str = "off";
            break;
        case 8:
            str = "rebooting";
            break;
        case 9:
            str = "switched bypass";
            break;
        case 10:
            str = "hardware failure bypass";
            break;
        case 11:
            str = "sleeping until power return";
            break;
        case 12:
            str = "on smart trim";
            break;
        case 13:
            str = "eco mode";
            break;
        case 14:
            str = "hot standby";
            break;
        case 15:
            str = "onn battery test";
            break;
    }
    return str;
}
/**
 * @brief Converts status code to string.
 *        See: upsBasicBatteryStatus
 * 
 * @param code 
 * @param buffer 
*/
inline std::string upsBatteryStatusCodeToString(uint8_t &code) {
    std::string str;
    switch(code) {
        default:
        case 1:
            str = "unknown";
            break;
        case 2:
            str = "normal";
            break;
        case 3:
            str = "low";
            break;
        case 4:
            str = "faulty";
            break;
    }
    return str;
}

#endif                                  // SNMP_DEFS_h
