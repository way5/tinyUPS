#include "SNMPInform.h"
#include "SNMPTrap.h"
#include <list>

inline void delete_inform(struct InformItem* inform){
    free(inform);
}

static void remove_inform_from_list(std::list<struct InformItem *> &list,
                             std::function<bool(struct InformItem *)> predicate) {
    list.remove_if([&predicate](struct InformItem* item){
        if(predicate(item)){
            delete_inform(item);
            return true;
        }
        return false;
    });
}

snmp_request_id_t
queue_and_send_trap(std::list<struct InformItem *> &informList, SNMPTrap *trap, const IPAddress& ip, bool replaceQueuedRequests,
                    int retries, int delay_ms) {
    bool buildStatus = trap->buildForSending();
    if(!buildStatus) {
        __DL("(!) couldn't build trap");
        snmpLog.putts(PSTR("(!) couldn't build trap"));
        return INVALID_SNMP_REQUEST_ID;
    };
    __DF("%u informs in informList\n", informList.size());
    //TODO: could be race condition here, buildStatus to return packet?
    if(replaceQueuedRequests){
#if DEBUG == 5
        __DL("removing any outstanding informs for this trap");
#endif
        remove_inform_from_list(informList, [trap](struct InformItem* informItem) -> bool {
            return informItem->trap == trap;
        });
    }

    if(trap->inform){
        struct InformItem* item = (struct InformItem*)calloc(1, sizeof(struct InformItem));
        item->delay_ms = delay_ms;
        item->received = false;
        item->requestID = trap->requestID;
        item->retries = retries;
        item->ip = ip;
        item->lastSent = millis();
        item->trap = trap;
        item->missed = false;

        __DF("adding inform request to queue: %lu\n", item->requestID);

        informList.push_back(item);

        trap->sendTo(ip, true);
    } else {
        // normal send
    #if DEBUG == 5
        __DL("(i) sending normal trap");
    #endif
        trap->sendTo(ip);
    }

    return trap->requestID;
}

void inform_callback(std::list<struct InformItem *> &informList, snmp_request_id_t requestID, bool responseReceiveSuccess) {
    (void)responseReceiveSuccess;
#if DEBUG == 5
    __DF("receiving informCallback for requestID: %lu, success: %d\n", requestID, responseReceiveSuccess);
#endif
    //TODO: if we ever want to keep received informs, change this logic
    remove_inform_from_list(informList, [requestID](struct InformItem* informItem) -> bool {
        return informItem->requestID == requestID;
    });
#if DEBUG == 5
    __DF("informs waiting for responses: %u\n", informList.size());
#endif
}

void handle_inform_queue(std::list<struct InformItem *> &informList) {
    auto thisLoop = millis();
    for(auto informItem : informList){
        if(!informItem->received && thisLoop - informItem->lastSent > informItem->delay_ms) {
        #if DEBUG == 5
            __DL("(!) missed Inform receive");
        #endif
            // check if sending again
            informItem->missed = true;
            if(!informItem->retries){
            #if DEBUG == 5
                __DF("no more retries for inform: %lu, removing\n", informItem->requestID);
            #endif
                continue;
            }
            if(informItem->trap){
            #if DEBUG == 5
                __DF("no response received in %lums, resending Inform: %lu\n", thisLoop - informItem->lastSent, informItem->requestID);
            #endif
                informItem->trap->sendTo(informItem->ip, true);
                informItem->lastSent = thisLoop;
                informItem->missed = false;
                informItem->retries--;
            }
        }
    }
    remove_inform_from_list(informList, [](struct InformItem* informItem) -> bool {
        return informItem->received || (informItem->retries == 0 && informItem->missed);
    });
}

/**
 * @brief 
 * 
 * @param informList 
 * @param trap 
*/
void mark_trap_deleted(std::list<struct InformItem *> &informList, SNMPTrap *trap) {
#if DEBUG == 5
    __DL("removing waiting informs tied to Trap");
#endif
    remove_inform_from_list(informList, [trap](struct InformItem* informItem) -> bool {
        return informItem->trap == trap;
    });
}
