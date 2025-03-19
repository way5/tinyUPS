// we're going to have a trap object, which is created in setup. 
// it includes the main trapOID, other trap-ish options, and an 
// attached list of the OIDCallback objects (the ones you get back 
// from addIntegerHandler etc) to contain the values that need 
// to be ent with this Trap.
// it ca then be called from code, trap->send or whatever. 
// trap receivers should also be attached to a list that can be changed

#ifndef SNMPTrap_h
#define SNMPTrap_h

#include <list>
#include "defs.h"
#include "ValueCallbacks.h"
#include "SNMPPacket.h"
#include <stdlib.h>

static WiFiUDP snmpTrapUDP;

extern uint8_t WORD_ALIGNED_ATTR _snmpPacketBuffeer[SNMP_MAX_PACKET_LENGTH];

class SNMPTrap : public SNMPPacket
{
public:
    SNMPTrap(const char *community, SNMP_VERSION version)
    {
        this->setVersion(version);
        // Version two will use the SNMPPacket builder which uses the member pduType
        // Version one will use special builder with hardcoded PDU Type
        this->setPDUType(Trapv2PDU);
        this->setCommunityString(community);
    };
    virtual ~SNMPTrap();

    IPAddress agentIP;
    OIDType *trapOID = nullptr;

    TimestampCallback *uptimeCallback = nullptr;
    short genericTrap = 6;
    short specificTrap = 1;
    bool inform = false;
    bool setInform(bool inf)
    {
        this->inform = inf;
        ASN_TYPE pduType = this->packetPDUType;
        switch (this->snmpVersion)
        {
        case SNMP_VERSION_1:
            pduType = TrapPDU;
            break;
        case SNMP_VERSION_2C:
            pduType = this->inform ? InformRequestPDU : Trapv2PDU;
            break;
        default:
            break;
        }
        this->setPDUType(pduType);
        return true;
    }
    // the setters that need to be configured for each trap.

    void setTrapOID(OIDType *oid)
    {
        trapOID = oid;
    }

    void setSpecificTrap(short num)
    {
        specificTrap = num;
    }

    void setIP(IPAddress ip)
    { // sets our IP
        agentIP = ip;
    }

    void stop()
    {
        snmpTrapUDP.stop();
    }

    void setUptimeCallback(TimestampCallback *uptime)
    {
        uptimeCallback = uptime;
    }

    void addOIDPointer(ValueCallback *callback);

    bool buildForSending()
    {
        // This is the start of a fresh send, we're going to reset our requestID, and build the packet
        this->setRequestID(SNMPPacket::generate_request_id());

        // Flow for v2Trap/v2Inform is SNMPPacket::build()  -> SNMPTrap::generateVarBindList() -> v2 logic
        // flow for v1Trap is          SNMPTrap::build()    -> SNMPTrap::generateVarBindList() -> v1 logic

        if (this->snmpVersion == SNMP_VERSION_1)
        {
            // Version 1 needs a special structure, so we overwrite the building part
            return this->build();
        }
        else
        {
            // Version 2 will use regular packet building but call back our generateVarBindList, so we can still use callbacks
            return SNMPPacket::build();
        }
    }

    bool sendTo(const IPAddress &ip, bool skipBuild = false)
    {
        bool buildStatus = true;
        if (!skipBuild)
        {
            buildStatus = this->buildForSending();
        }

        if (!this->packet)
        {
            return false;
        }

        if (!buildStatus)
        {
            __DL("(!) failed to build packet");
            logsnmp.putts("(!) trap: failed to build packet");
            return false;
        }

        int length = packet->serialise(_snmpPacketBuffeer, SNMP_MAX_PACKET_LENGTH);

        if (length <= 0)
            return false;

        snmpTrapUDP.beginPacket(ip, config.snmpTrapPort);
        snmpTrapUDP.write(_snmpPacketBuffeer, length);
        return snmpTrapUDP.endPacket();
    }

protected:
    std::list<ValueCallback *> callbacks;
    std::shared_ptr<ComplexType> generateVarBindList() override;
    OIDType *timestampOID = new OIDType(".1.3.6.1.2.1.1.3.0");
    OIDType *snmpTrapOID = new OIDType(".1.3.6.1.2.1.1.2.0");
    bool build() override;
};

#endif                          // SNMPTrap_h
