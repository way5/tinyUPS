#include "defs.h"
#include "SNMPParser.h"
#include "BER.h"
#include "ValueCallbacks.h"

/**
 * @brief
 *
 * @param callbacks
 * @param varbindList
 * @param outResponseList
 * @param snmpVersion
 * @param isGetNextRequest
 * @return true
 * @return false
 */
bool handleGetRequestPDU(std::deque<ValueCallback *> &callbacks, std::deque<VarBind> &varbindList,
                            std::deque<VarBind> &outResponseList, SNMP_VERSION snmpVersion, bool isGetNextRequest)
{
    // __DL("handleGetRequestPDU");
    for (const VarBind &requestVarBind : varbindList)
    {
#if DEBUG == 5
        __DF("searching callback for: %s\n", requestVarBind.oid->string().c_str());
#endif
        ValueCallback *callback = ValueCallback::findCallback(callbacks, requestVarBind.oid.get(), isGetNextRequest);
        if (!callback)
        {
            __DF(PSTR("(!) no callback for: %s\n"), requestVarBind.oid->string().c_str());
#if 1
            // According to RFC3416 we should be setting the value to 'noSuchObject' or 'noSuchInstance,
            // but this doesn't seem to render nicely in tools, so possibly revert to old NO_SUCH_NAME error
            if (isGetNextRequest)
            {
                // if it's a walk it's an endOfMibView
                outResponseList.emplace_back(requestVarBind, std::make_shared<ImplicitNullType>(ENDOFMIBVIEW));
            }
            else
            {
                outResponseList.emplace_back(requestVarBind, std::make_shared<ImplicitNullType>(NOSUCHOBJECT));
            }

#else
            outResponseList.emplace_back(generateErrorResponse(SNMP_ERROR_VERSION_CTRL_DEF(NOT_WRITABLE, snmpVersion, NO_SUCH_NAME), requestVarBind.oid));
#endif
            continue;
        }
#if DEBUG == 5
        __DF("callback found for OID: %s\n", callback->OID->string().c_str());
#endif
        // NOTE: we could just use the same pointer as the reqwuest, but delete the value and add a new one. Will have to figure out what to do if it errors, do that later
        auto value = ValueCallback::getValueForCallback(callback);

        if (!value)
        {
            __DL("(!) couldn't get value for callback");
            snmpLog.putts(PSTR("(!) no callback value for OID: %s"), requestVarBind.oid->string().c_str());
            outResponseList.emplace_back(callback->OID, SNMP_ERROR_VERSION_CTRL(GEN_ERR, snmpVersion));
            continue;
        }

        outResponseList.emplace_back(callback->OID, value);
    }
    return true; // we didn't fail in our job, even if we filled in nothing
}

/**
 * @brief
 *
 * @param callbacks
 * @param varbindList
 * @param outResponseList
 * @param snmpVersion
 * @return true
 * @return false
 */
bool handleSetRequestPDU(std::deque<ValueCallback *> &callbacks, std::deque<VarBind> &varbindList, 
                            std::deque<VarBind> &outResponseList, SNMP_VERSION snmpVersion)
{
    // __DL("handleSetRequestPDU");
    for (const VarBind &requestVarBind : varbindList)
    {
#if DEBUG == 5
        __DF("searching callback for OID: %s\n", requestVarBind.oid->string().c_str());
#endif
        ValueCallback *callback = ValueCallback::findCallback(callbacks, requestVarBind.oid.get(), false);
        if (!callback)
        {
            __DF(PSTR("(!) no callback for: %s\n"), requestVarBind.oid->string().c_str());
            outResponseList.emplace_back(requestVarBind.oid, SNMP_ERROR_VERSION_CTRL_DEF(NOT_WRITABLE, snmpVersion, NO_SUCH_NAME));
            continue;
        }
#if DEBUG == 5
        __DF("callback found with OID: %s\n", callback->OID->string().c_str());
#endif
        if (callback->type != requestVarBind.type)
        {
            __DF(PSTR("(!) callback type mismatch: %d\n"), callback->type);
            outResponseList.emplace_back(requestVarBind.oid, SNMP_ERROR_VERSION_CTRL_DEF(WRONG_TYPE, snmpVersion, BAD_VALUE));
            continue;
        }

        if (!callback->isSettable)
        {
            __DL("(!) cannot set this object");
            outResponseList.emplace_back(requestVarBind.oid, SNMP_ERROR_VERSION_CTRL(READ_ONLY, snmpVersion));
            continue;
        }
        // NOTE: we could just use the same pointer as the reqwuest, but delete the value and add a new one. Will have to figure out what to do if it errors, do that later
        SNMP_ERROR_STATUS setError = ValueCallback::setValueForCallback(callback, requestVarBind.value);
        if (setError != NO_ERROR)
        {
            __DF(PSTR("(!) setting variable failed: %d\n"), setError);
            snmpLog.putts(PSTR("(!) error: %d, setting variable failed for OID: %s"), setError, requestVarBind.oid->string().c_str());
            outResponseList.emplace_back(callback->OID, SNMP_ERROR_VERSION_CTRL(setError, snmpVersion));
            continue;
        }

        auto value = ValueCallback::getValueForCallback(callback);

        if (!value)
        {
            __DL("(!) failed to get value for callback");
            snmpLog.putts(PSTR("(!) no callback value for OID: %s"), requestVarBind.oid->string().c_str());
            outResponseList.emplace_back(callback->OID, SNMP_ERROR_VERSION_CTRL(GEN_ERR, snmpVersion));
            continue;
        }

        outResponseList.emplace_back(callback->OID, value);
    }
    return true; // we didn't fail in our job
}

/**
 * @brief
 *
 * @param callbacks
 * @param varbindList
 * @param outResponseList
 * @param nonRepeaters
 * @param maxRepititions
 * @return true
 * @return false
 */
bool handleGetBulkRequestPDU(std::deque<ValueCallback *> &callbacks, std::deque<VarBind> &varbindList, 
                                std::deque<VarBind> &outResponseList, unsigned int nonRepeaters, unsigned int maxRepititions)
{
    // from https://tools.ietf.org/html/rfc1448#page-18
#if DEBUG == 5
    __DF("handleGetBulkRequestPDU, nonRepeaters:%d, maxRepititions:%d, varbindSize:%u\n", nonRepeaters, maxRepititions, varbindList.size());
#endif
    // nonRepeaters is MIN(nonRepeaters, varbindList.size()
    // repeaters is the extra of varbindList.size() - nonRepeaters) which get 'walked' maxRepititions times

    // __DL("handling nonRepeaters");
    if (nonRepeaters > 0)
    {
        // handle GET normally, but mark endOfMibView if not found
        for (unsigned int i = 0; i < nonRepeaters && i < varbindList.size(); i++)
        {
            const VarBind &requestVarBind = varbindList[i];
            ValueCallback *callback = ValueCallback::findCallback(callbacks, requestVarBind.oid.get(), true);
            if (!callback)
            {
                outResponseList.emplace_back(requestVarBind, std::make_shared<ImplicitNullType>(ENDOFMIBVIEW));
                continue;
            }

            auto value = ValueCallback::getValueForCallback(callback);
            if (!value)
            {
                __DL("(!) couldn't get value for callback");
                snmpLog.putts(PSTR("(!) no callback value for OID: %s"), requestVarBind.oid->string().c_str());
                outResponseList.emplace_back(callback->OID, GEN_ERR);
                continue;
            }
            outResponseList.emplace_back(requestVarBind, value);
        }
    }

    if (varbindList.size() > nonRepeaters)
    {
        // For each extra varbind, WALK that tree until maxRepititions or endOfMibView
        // __DL("handling repeaters");
        unsigned int repeatingVarBinds = varbindList.size() - nonRepeaters;

        for (unsigned int i = 0; i < repeatingVarBinds; i++)
        {
            // Store first varbind to get for each line
            auto oid = varbindList[i + nonRepeaters].oid;
            size_t foundAt = 0;

            for (unsigned int j = 0; j < maxRepititions; j++)
            {
#if DEBUG == 5
                __DF("searhing next callback for OID: %s\n", oid->string().c_str());
#endif
                ValueCallback *callback = ValueCallback::findCallback(callbacks, oid.get(), true, foundAt, &foundAt);
                if (!callback)
                {
                    // We're done, mark endOfMibView
                    outResponseList.emplace_back(oid, std::make_shared<ImplicitNullType>(ENDOFMIBVIEW));
                    break;
                }

                auto value = ValueCallback::getValueForCallback(callback);

                if (!value)
                {
                    __DL("(!) couldn't get value for callback");
                    snmpLog.putts(PSTR("(!) no callback value for OID: %s"), oid->string().c_str());
                    outResponseList.emplace_back(callback->OID, GEN_ERR);
                    break;
                }

                outResponseList.emplace_back(callback->OID, value);

                // set next oid to callback OID
                oid = callback->OID->cloneOID();
            }

            //__DF("Walked tree of %s, %d times", (*varbindList)[i+nonRepeaters]->oid->_value, j);
        }
    }

    return true;
}