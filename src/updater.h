/*
#####################################################################################
# File: update.h                                                                    #
# File Created: Monday, 8th January 2024 11:05:06 pm                                #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 9th January 2024 12:01:03 am                              #
# Modified By: Sergey Ko                                                            #
# License: CC-BY-NC-4.0 (https://creativecommons.org/licenses/by-nc/4.0/legalcode)  #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef UPDATER_H
#define UPDATER_H

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "helpers.h"

bool updaterGetReady();
bool updaterWriteData(uint8_t * data, size_t len, bool final);
bool _updaterWritePartition(size_t len);
bool updaterInProgress();
bool updaterHasErrors();
esp_err_t updaterLastError();

#endif