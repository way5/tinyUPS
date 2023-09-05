/*
#####################################################################################
# File: httpd.h                                                                     #
# File Created: Monday, 22nd May 2023 4:02:57 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 10:29:37 pm                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/* NOTE:
    ******** DEVELOPERs' MEMO
    - SITEMAP (See: webpack.config.js):
        unauthorized    /           = s.htm
        unauthorized    /           = l.htm
        authorized      /           = i.htm
        authorized      /(.+)       = e.htm (404)
        authorized      /app.js     = app.js
        authorized      /app.css    = app.css

    -   AJAX RESPOMSE: ERROR
        { "err": "(code|description)" }

*/

#ifndef HTTPD_SERVER_H
#define HTTPD_SERVER_H

#include "helpers.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include "FS.h"
#include "FFat.h"
#include "eemem.h"
#include "monitor.h"
#include "agent.h"
#include "flog.h"
#include "ntpc.h"

extern AsyncWebServer httpd;
extern void systemReboot();
extern fLogClass logsys;
extern fLogClass logsnmp;
extern fLogClass logTempMon;
extern fLogClass logDataMon;

const char _httpTokenSalt[] = "rah2Eingie9aeWi";

const char _apiKeysDBPath[] = "/data/api";

const char resPageLogin[] = "/l.htm";
const char resPageIndex[] = "/i.htm";
const char resPageError[] = "/e.htm";
const char resPageSetup[] = "/s.htm";
const char resFavicon[] = "/favicon.ico";
const char resSvgEye[] = "/eye.svg";
const char resSvgGth[] = "/gth.svg";

const char headerCookie[] = "Cookie";
const char headerSetCookie[] = "Set-Cookie";
const char headerLocation[] = "Location";
const char cookieName[] = "TINYUPSSID=";
const char cookieNameReset[] = "TINYUPSSID=0; Max-Age=0";
const char cookieName2[] = "; path=/; Max-Age=";

const char mimeTextPlain[] = "text/plain";
const char mimeTextHtml[] = "text/html";
const char mimeTextCss[] = "text/css";
const char mimeAppJS[] = "application/javascript";
const char mimeAppJSON[] = "application/json";
const char mimeImgXICN[] = "image/x-icon";
const char mimeImgSVG[] = "image/svg+xml";

const char jsonLoginRepeat[] = "{\"login\":\"repeat\"}";
const char jsonLoginOK[] = "{\"login\":\"ok\"}";
const char jsonLoginERR[] = "{\"login\":\"err\",\"err\":\"Wrong login or password\"}";

const char maskDashbrd[] = "{\"ip\":\"%s\",\"sm\":\"%s\",\"gw\":\"%s\",\"mac\":\"%s\",\"ap\":\"%s\","\
                                            "\"apmac\":\"%s\",\"apch\": %d,\"ram\": %.2f,\"ram3\": %.2f,\"cpu\": %d,"\
                                            "\"systmp\": %.2f,\"involt\":%d,\"infreq\":%d,\"outvolt\":%d,\"outfreq\":%d,\"batchd\":\"%s\",\"battmp\":%.2f,"\
                                            "\"snmp\":%d,\"outst\":%d,\"battst\":%d,\"battdiast\":%d,\"battcap\":%d,"\
                                            "\"outload\":%d,\"ltime\":%u,\"isclng\":%d,\"uptm\":%lu,\"ctime\":\"%s\"}";
const char maskInfoGraph[] = "{\"%s\":{\"st\":%.2f,\"bt\":%.2f,\"r\":%.2f,\"r3\":%.2f}}";
const char maskGetConfig[] = "{\"battmplt\":%.2f,\"battmput\":%.2f,\"devtmplt\":%.2f,\"devtmput\":%.2f,\"ntpsrv\":\"%s\",\"ntpsrvfb\":\"%s\",\"ntpsrvsitl\":%d,\"ntptmoff\":%d,\"ntpdloff\":%d,"\
                                            "\"ssid\":\"%s\",\"ssidkey\":\"%s\",\"snmpport\":%d,\"snmptraport\":%d,\"snmploctn\":\"%s\","\
                                            "\"snmpcontct\":\"%s\",\"snmpbatrpldt\":\"%s\",\"authtmout\":%d,\"snmpgckey\":\"%s\","\
                                            "\"snmpsckey\":\"%s\",\"adlogin\":\"%s\",\"adpass\":\"%s\",\"api\":[%s]}";
const char maskSurvey[] = "{\"s\":\"%s\",\"r\":%d,\"e\":%d}";

typedef struct ApiKeysT {
    char * memo;
    time_t created = 0;
    char * key;
    ApiKeysT() {
        this->memo = reinterpret_cast<char *>(malloc((size_t)16));
        this->key = reinterpret_cast<char *>(malloc((size_t)32));
    }
    ~ApiKeysT() {
        free(this->memo);
        free(this->key);
    }
} api_keys_t;

#define _ALLOC_API_ARRAY(A)        do {                                 \
    A = new api_keys_t * [5];                                           \
    uint8_t i = 0;                                                      \
    while(i < 5) {                                                      \
        A[i] = nullptr;                                                 \
        i++;                                                            \
    }                                                                   \
} while(0)

void httpdInit();
// utils
bool isAuthorized(AsyncWebServerRequest * req);
void httpdRespond(AsyncWebServerRequest * req, const char * file, const char* mime, bool gzipped = true, AsyncWebServerResponse * res = nullptr);
void httpdLoop();
int8_t loadAPIKeys(api_keys_t ** keys);
char * apiKeysToJSON();
bool addAPIKey(api_keys_t * k);
bool removeAPIKey(time_t &id);
bool writeAPIData(api_keys_t ** data);
// HTML & assets
void httpdGetHtmlPage(AsyncWebServerRequest *req);
void httpdGetHtmlError(AsyncWebServerRequest * req);
void httpdGetStyleChunk(AsyncWebServerRequest *req);
void httpdGetScriptChunk(AsyncWebServerRequest *req);
void httpdGetFavicon(AsyncWebServerRequest *req);
void httpdGetSvgImage(AsyncWebServerRequest *req);
void httpdGetLogout(AsyncWebServerRequest *req);
// API
void httpdPostSiteSurvey(AsyncWebServerRequest *req);
void httpdPostSetupInit(AsyncWebServerRequest *req);
void httpdPostSetup(AsyncWebServerRequest *req);
void httpdPostLogin(AsyncWebServerRequest *req);
void httpdPostSysLog(AsyncWebServerRequest *req);
void httpdPostSnmpLog(AsyncWebServerRequest *req);
void httpdPostInfoGraph(AsyncWebServerRequest *req);
void httpdPostMonTmpLog(AsyncWebServerRequest *req);
void httpdPostMonBDtaLog(AsyncWebServerRequest *req);
void httpdPostAPIadd(AsyncWebServerRequest * req);
void httpdPostAPIdel(AsyncWebServerRequest * req);
void httpdPostGetDashbrd(AsyncWebServerRequest *req);
// config
void httpdPostGetConfig(AsyncWebServerRequest *req);
void httpdPostSetConfigSystem(AsyncWebServerRequest *req);
void httpdPostSetConfigSNMP(AsyncWebServerRequest *req);
void httpdPostSetConfigSecurity(AsyncWebServerRequest *req);
void httpdPostReboot(AsyncWebServerRequest *req);
void httpdPostControlCooling(AsyncWebServerRequest *req);
void httpdPostReset(AsyncWebServerRequest *req);
// common JSON responses
void httpdJsonErrResponse(AsyncWebServerRequest *req, const char * descr, const int code = 401);

#endif                              // HTTPD_SERVER_H