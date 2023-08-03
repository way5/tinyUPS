/*
#####################################################################################
# File: httpd.h                                                                     #
# File Created: Monday, 22nd May 2023 4:02:57 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 2nd August 2023 5:05:08 pm                              #
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
        authorized      /app.js     = app.js (404)
        authorized      /app.css    = app.css (404)

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
extern fLogClass sysLog;
extern fLogClass snmpLog;
extern fLogClass monTempLog;
extern fLogClass monDataLog;

static const char _httpTokenSalt[] PROGMEM = "rah2Eingie9aeWi";

static const char _apiKeysDBPath[] PROGMEM = "/data/api";

static const char resPageLogin[] PROGMEM = "/l.htm";
static const char resPageIndex[] PROGMEM = "/i.htm";
static const char resPageError[] PROGMEM = "/e.htm";
static const char resPageSetup[] PROGMEM = "/s.htm";
static const char resCommonCss[] PROGMEM = "/c.css";
static const char resLoginCss[] PROGMEM = "/l.css";
static const char resIndexCss[] PROGMEM = "/i.css";
static const char resErrorCss[] PROGMEM = "/e.css";
static const char resSetupCss[] PROGMEM = "/s.css";
static const char resCommonJs[] PROGMEM = "/c.js";
static const char resLoginJs[] PROGMEM = "/l.js";
static const char resIndexJs[] PROGMEM = "/i.js";
static const char resErrorJs[] PROGMEM = "/e.js";
static const char resSetupJs[] PROGMEM = "/s.js";
static const char resFavicon[] PROGMEM = "/favicon.ico";
static const char resSvgEye[] PROGMEM = "/eye.svg";
static const char resSvgGth[] PROGMEM = "/gth.svg";

static const char headerCookie[] PROGMEM = "Cookie";
static const char headerSetCookie[] PROGMEM = "Set-Cookie";
static const char headerLocation[] PROGMEM = "Location";
static const char cookieName[] PROGMEM = "TINYUPSSID=";
static const char cookieNameReset[] PROGMEM = "TINYUPSSID=0; Max-Age=0";
static const char cookieName2[] PROGMEM = "; path=/; Max-Age=";

static const char mimeTextPlain[] PROGMEM = "text/plain";
static const char mimeTextHtml[] PROGMEM = "text/html";
static const char mimeTextCss[] PROGMEM = "text/css";
static const char mimeAppJS[] PROGMEM = "application/javascript";
static const char mimeAppJSON[] PROGMEM = "application/json";
static const char mimeImgXICN[] PROGMEM = "image/x-icon";
static const char mimeImgSVG[] PROGMEM = "image/svg+xml";

static const char jsonLoginRepeat[] PROGMEM = "{\"login\":\"repeat\"}";
static const char jsonLoginOK[] PROGMEM = "{\"login\":\"ok\"}";
static const char jsonLoginERR[] PROGMEM = "{\"login\":\"err\",\"err\":\"Wrong login or password\"}";

static const char maskDashbrd[] PROGMEM = "{\"ip\":\"%s\",\"sm\":\"%s\",\"gw\":\"%s\",\"mac\":\"%s\",\"ap\":\"%s\","\
                                            "\"apmac\":\"%s\",\"apch\": %d,\"ram\": %.2f,\"ram3\": %.2f,\"cpu\": %d,"\
                                            "\"systmp\": %.2f,\"involt\":%d,\"infreq\":%d,\"outvolt\":%d,\"outfreq\":%d,\"batchd\":\"%s\",\"battmp\":%.2f,"\
                                            "\"snmp\":%d,\"outst\":%d,\"battst\":%d,\"battdiast\":%d,\"battcap\":%d,"\
                                            "\"outload\":%d,\"ltime\":%u,\"isclng\":%d,\"uptm\":%lu,\"ctime\":\"%s\"}";
static const char maskInfoGraph[] PROGMEM = "{\"%s\":{\"st\":%.2f,\"bt\":%.2f,\"r\":%.2f,\"r3\":%.2f}}";
static const char maskGetConfig[] PROGMEM = "{\"battmplt\":%.2f,\"battmput\":%.2f,\"devtmplt\":%.2f,\"devtmput\":%.2f,\"ntpsrv\":\"%s\",\"ntpsrvfb\":\"%s\",\"ntpsrvsitl\":%d,\"ntptmoff\":%d,\"ntpdloff\":%d,"\
                                            "\"ssid\":\"%s\",\"ssidkey\":\"%s\",\"snmpport\":%d,\"snmptraport\":%d,\"snmploctn\":\"%s\","\
                                            "\"snmpcontct\":\"%s\",\"snmpbatrpldt\":\"%s\",\"authtmout\":%d,\"snmpgckey\":\"%s\","\
                                            "\"snmpsckey\":\"%s\",\"adlogin\":\"%s\",\"adpass\":\"%s\",\"api\":[%s]}";
static const char maskSurvey[] PROGMEM = "{\"s\":\"%s\",\"r\":%d,\"e\":%d}";

typedef struct {
    char memo[16] = "";
    time_t created = 0;
    char key[32] = "";
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