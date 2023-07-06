#include "SNMPParser.h"


static SNMP_PERMISSION getPermissionOfRequest(const SNMPPacket& request) { // , const std::string& _community, const std::string& _readOnlyCommunity){
    SNMP_PERMISSION requestPermission = SNMP_PERM_NONE;
#if DEBUG == 5
    __DF(PSTR("community string: %s\n"), request.communityString.c_str());
#endif
    // if(!_readOnlyCommunity.empty() && _readOnlyCommunity == request.communityString) { // snmprequest->version != 1
    if(strlen(config.snmpGetCN) != 0 && strcmp(config.snmpGetCN, request.communityString.c_str()) == 0) { 
        requestPermission = SNMP_PERM_READ_ONLY;
    }

    if(strcmp(config.snmpSetCN, request.communityString.c_str()) == 0) { // snmprequest->version != 1
        requestPermission = SNMP_PERM_READ_WRITE;
    }
    return requestPermission;
}

/**
 * @brief 
 * 
 * @param buffer 
 * @param packetLength 
 * @param responseLength 
 * @param max_packet_size 
 * @param callbacks 
 * @param informCallback 
 * @param ctx 
 * @return SNMP_ERROR_RESPONSE 
*/
SNMP_ERROR_RESPONSE handlePacket(uint8_t* buffer, int packetLength, int* responseLength, 
                                /* int max_packet_size, */ std::deque<ValueCallback*> &callbacks, 
                                /* const std::string& _community, const std::string& _readOnlyCommunity, */ 
                                informCB informCallback, void* ctx) {
    SNMPPacket request;

    SNMP_PACKET_PARSE_ERROR parseResult = request.parseFrom(buffer, packetLength);
    if(parseResult <= 0) {
        __DF("error code: %d when attempting to parse\n", parseResult);
        return SNMP_REQUEST_INVALID;
    }
#if DEBUG == 5
    __DL("-  valid SNMP packet");
#endif
    if(request.packetPDUType == GetResponsePDU) {
        __DF("received GetResponse! probably as a result of a recent informTrap: %lu\n", request.requestID);
        if(informCallback){
            informCallback(ctx, request.requestID, !request.errorStatus.errorStatus);
        } else {
            __DL("(i) nothing to do with Inform");
        }
        return SNMP_INFORM_RESPONSE_OCCURRED;
    }

    SNMP_PERMISSION requestPermission = getPermissionOfRequest(request); //, _community, _readOnlyCommunity);
    if(requestPermission == SNMP_PERM_NONE){
        __DF("(!) invalid community provided: %s, no response to give\n", request.communityString.c_str());
        snmpLog.putts(PSTR("(!) invalid community provided: %s, no response to give"), request.communityString.c_str());
        return SNMP_REQUEST_INVALID_COMMUNITY;
    }
    
    // this will take the required stuff from request - like requestID and community string etc
    SNMPResponse response = SNMPResponse(request);

    std::deque<VarBind> outResponseList;

    bool pass = false;
    SNMP_ERROR_RESPONSE handleStatus = SNMP_NO_ERROR;
    SNMP_ERROR_STATUS globalError = GEN_ERR;

    switch(request.packetPDUType){
        case GetRequestPDU:
        case GetNextRequestPDU:
            pass = handleGetRequestPDU(callbacks, request.varbindList, outResponseList, request.snmpVersion, request.packetPDUType == GetNextRequestPDU);
            handleStatus = request.packetPDUType == GetRequestPDU ? SNMP_GET_OCCURRED : SNMP_GETNEXT_OCCURRED;
        break;
        case GetBulkRequestPDU:
            if(request.snmpVersion != SNMP_VERSION_2C) {
                __DL("(i) received GetBulkRequest in SNMP_VERSION_1");
                snmpLog.putts(PSTR("(i) received GetBulkRequest in SNMP_VERSION_1"));
                pass = false;
                globalError = GEN_ERR;
            } else {
                pass = handleGetBulkRequestPDU(callbacks, request.varbindList, outResponseList, request.errorStatus.nonRepeaters, request.errorIndex.maxRepititions);
                handleStatus = SNMP_GETBULK_OCCURRED;
            }
        break;
        case SetRequestPDU:
            if(requestPermission != SNMP_PERM_READ_WRITE){
                __DL("(!!) attempt to SET without permissions");
                snmpLog.putts(PSTR("(!!) attempt to SET without permissions"));
                pass = false;
                globalError = NO_ACCESS;
            } else {
                pass = handleSetRequestPDU(callbacks, request.varbindList, outResponseList, request.snmpVersion);
                handleStatus = SNMP_SET_OCCURRED;
            }
        break;
        default:
            __DF("(i) not sure what to do with SNMP PDU of type: %d\n", request.packetPDUType);
            handleStatus = SNMP_UNKNOWN_PDU_OCCURRED;
            pass = false;
        break;
    }

    if(pass) {
        for(const auto& item : outResponseList){
            if(item.errorStatus != NO_ERROR){
                response.addErrorResponse(item);
            } else {
                response.addResponse(item);
            }
        }
    } else {
        // Something went wrong, generic error response
        __DF("(!) error: %d while building request, sending error PDU\n", globalError);
        response.setGlobalError(globalError, 0, true);
        handleStatus = SNMP_ERROR_PACKET_SENT;
    }

    // memset(buffer, 0, max_packet_size);
    memset(buffer, 0, SNMP_MAX_PACKET_LENGTH);

    // *responseLength = response.serialiseInto(buffer, max_packet_size);
    *responseLength = response.serialiseInto(buffer, SNMP_MAX_PACKET_LENGTH);
    if(*responseLength <= 0){
        __DL("(!) failed to build response packet\n");
        snmpLog.putts(PSTR("(!) failed to build response packet"));
        return SNMP_FAILED_SERIALISATION;
    }

    return handleStatus;
}