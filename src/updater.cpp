/*
#####################################################################################
# File: update.cpp                                                                  #
# File Created: Monday, 8th January 2024 11:04:58 pm                                #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 9th January 2024 1:58:10 am                               #
# Modified By: Sergey Ko                                                            #
# License: CC-BY-NC-4.0 (https://creativecommons.org/licenses/by-nc/4.0/legalcode)  #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "updater.h"

static __attribute__(( aligned(4) )) uint8_t * upFsBuffer = nullptr;
static const esp_partition_t * upFsPartn = nullptr;
static size_t upFsBufferCursor = 0;
// filesystem standard header offset
static size_t upFsOffset = 0x1000;
static esp_err_t upFsErr = 0;


/**
 * @brief Prepeare buffer and retrieve partition pointer
 *
 * @return true
 * @return false
 */
bool updaterGetReady() {
    // prepare buffer
    upFsBuffer = reinterpret_cast<uint8_t *>(malloc(SPI_FLASH_SEC_SIZE));
    // looking for the partition
    upFsPartn = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
    if(!upFsPartn) {
        return false;
    }
    return true;
}

/**
 * @brief Erases and writes data onto data partition
 *        returns true if there is no error
 *
 * @param data
 * @param len
 * @return true
 * @return false
 */
bool updaterWriteData(uint8_t * data, size_t len, bool final) {
    size_t remLen = len;
    while((upFsBufferCursor + remLen) > SPI_FLASH_SEC_SIZE)
    {
        size_t remData = (SPI_FLASH_SEC_SIZE - upFsBufferCursor);
        memcpy(upFsBuffer + upFsBufferCursor, data + (len - remLen), remData);
        // erasing and writing
        if(!_updaterWritePartition(SPI_FLASH_SEC_SIZE))
            return false;
        // tail length
        remLen -= remData;
    }
    // process the remaining data
    memcpy(upFsBuffer + upFsBufferCursor, data + (len - remLen), remLen);
    upFsBufferCursor += remLen;
    // skip any trailing data
    if(remLen == upFsPartn->size - upFsOffset) {
        if(!_updaterWritePartition(remLen))
            return false;
    }
    return true;
}

/**
 * @brief Flush the buffer
 *        returns true if there was no error
 *
 * @return true
 * @return false
 */
bool _updaterWritePartition(size_t len) {
    // erasing
    upFsErr = esp_partition_erase_range(upFsPartn, upFsOffset, len);
    if(upFsErr != ESP_OK) {
    // #if DEBUG == 2
        __DF("(!) partition erase error [%d] at offset [%d] length [%d]\n", upFsErr, upFsOffset, len);
    // #endif
        return false;
    }
    // writing
    upFsErr = esp_partition_write(upFsPartn, upFsOffset, upFsBuffer, len);
    if(upFsErr != ESP_OK) {
    // #if DEBUG == 2
        __DF("(!) partition write error [%d] at offset [%d] length [%d]\n", upFsErr, upFsOffset, len);
    // #endif
        return false;
    }
    // reset cursor
    upFsBufferCursor = 0;
    // touch offset
    upFsOffset += len;

    return true;
}

/**
 * @brief Returns false if there were no update made,
 *        true if update was made
 *
 * @return true
 * @return false
 */
bool updaterInProgress() {
    return (upFsPartn != nullptr);
}

/**
 * @brief Returns ESP_OM if there were no errors during update
 *
 * @return esp_err_t
 */
esp_err_t updaterLastError() {
    return upFsErr;
}

/**
 * @brief Returns true if an error has occurred
 *
 * @return true
 * @return false
 */
bool updaterHasErrors() {
    return (upFsErr != ESP_OK);
}
