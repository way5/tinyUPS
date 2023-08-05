/*
#####################################################################################
# File: httpd.cpp                                                                   #
# File Created: Monday, 22nd May 2023 4:02:52 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Friday, 4th August 2023 9:59:12 am                                 #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "httpd.h"

/**
 * @brief
 *
 */
void httpdInit()
{
    // HTML, CSS & JS
    httpd.on(F("/"), HTTP_GET, httpdGetHtmlPage);
    httpd.on(F("/logout"), HTTP_GET, httpdGetLogout);
    httpd.on(F("/c.css"), HTTP_GET, httpdGetStyleChunk);
    httpd.on(F("/l.css"), HTTP_GET, httpdGetStyleChunk);
    httpd.on(F("/i.css"), HTTP_GET, httpdGetStyleChunk);
    httpd.on(F("/e.css"), HTTP_GET, httpdGetStyleChunk);
    httpd.on(F("/s.css"), HTTP_GET, httpdGetStyleChunk);
    httpd.on(F("/c.js"), HTTP_GET, httpdGetScriptChunk);
    httpd.on(F("/l.js"), HTTP_GET, httpdGetScriptChunk);
    httpd.on(F("/i.js"), HTTP_GET, httpdGetScriptChunk);
    httpd.on(F("/e.js"), HTTP_GET, httpdGetScriptChunk);
    httpd.on(F("/s.js"), HTTP_GET, httpdGetScriptChunk);
    httpd.on(F("/favicon.ico"), HTTP_GET, httpdGetFavicon);
    httpd.on(F("/gth.svg"), HTTP_GET, httpdGetSvgImage);
    httpd.on(F("/eye.svg"), HTTP_GET, httpdGetSvgImage);
    // POST
    httpd.on(F("/survey"), HTTP_POST, httpdPostSiteSurvey);
    httpd.on(F("/setup-init"), HTTP_POST, httpdPostSetupInit);
    httpd.on(F("/setup"), HTTP_POST, httpdPostSetup);
    httpd.on(F("/login"), HTTP_POST, httpdPostLogin);
    httpd.on(F("/syslog"), HTTP_POST, httpdPostSysLog);
    httpd.on(F("/snmplog"), HTTP_POST, httpdPostSnmpLog);
    httpd.on(F("/infograph"), HTTP_POST, httpdPostInfoGraph);
    httpd.on(F("/montmpr"), HTTP_POST, httpdPostMonTmpLog);
    httpd.on(F("/monbdata"), HTTP_POST, httpdPostMonBDtaLog);
    httpd.on(F("/addapikey"), HTTP_POST, httpdPostAPIadd);
    httpd.on(F("/delapikey"), HTTP_POST, httpdPostAPIdel);
    // DASHBOARD DATA
    httpd.on(F("/getdashbrd"), HTTP_POST, httpdPostGetDashbrd);
    // CONFIG
    httpd.on(F("/getconfig"), HTTP_POST, httpdPostGetConfig);
    httpd.on(F("/setconfigsys"), HTTP_POST, httpdPostSetConfigSystem);
    httpd.on(F("/setconfigsnmp"), HTTP_POST, httpdPostSetConfigSNMP);
    httpd.on(F("/setconfigsec"), HTTP_POST, httpdPostSetConfigSecurity);
    // SYSTEM
    httpd.on(F("/reboot"), HTTP_POST, httpdPostReboot);
    httpd.on(F("/coolingctrl"), HTTP_POST, httpdPostControlCooling);
    httpd.on(F("/reset"), HTTP_POST, httpdPostReset);

    httpd.onNotFound(httpdGetHtmlError);

    httpd.begin();
}

/**
 * @brief Service routine
 *
 */
void httpdLoop()
{
    // auth timer
    if (strlen(session.authToken) != 0)
    {
        if ((session.authTimeout + (config.authTimeoutMax * 1000UL)) <= millis())
        {
            // reset timer and token
            session.authTimeout = 0;
            memset(session.authToken, '\0', sizeof(session.authToken));
        }
    }
}

/*
██╗   ██╗████████╗██╗██╗     ███████╗
██║   ██║╚══██╔══╝██║██║     ██╔════╝
██║   ██║   ██║   ██║██║     ███████╗
██║   ██║   ██║   ██║██║     ╚════██║
╚██████╔╝   ██║   ██║███████╗███████║
 ╚═════╝    ╚═╝   ╚═╝╚══════╝╚══════╝
*/

/**
 * @brief
 *
 * @param hash
 */
void hashgen(char *c)
{
    int randNumber = random(100);
    itoa(randNumber, c, 10);
    strcat_P(c, _httpTokenSalt);
}

/**
 * @brief Check if a client has signed in already
 *
 * @param req
 * @return true
 * @return false
 */
bool isAuthorized(AsyncWebServerRequest *req)
{
    bool res = false;
    if (strlen(session.authToken) != 0 && req->hasHeader(String(FPSTR(headerCookie))))
    {
        AsyncWebHeader *cookie = req->getHeader(String(FPSTR(headerCookie)));
        if (cookie->toString().indexOf(String(session.authToken)) != -1)
        {
            res = true;
        }
    }
    else if (req->method() == HTTP_POST && req->hasParam("key"))
    {
        AsyncWebParameter *k = req->getParam("key");
        if (k->value().length() != 32)
        {
#ifdef DEBUG
            __DF("wrong key lengh: %s\n", key);
#endif
            res = false;
        }
        else
        {
            api_keys_t **data;
            _ALLOC_API_ARRAY(data);
            int8_t n = loadAPIKeys(data);
            while (n > 0 && strlen(data[n - 1]->key) != 0)
            {
                if (strcmp(data[n - 1]->key, k->value().c_str()) == 0)
                {
#ifdef DEBUG
                    __DL("auth key verified");
#endif
                    res = true;
                    break;
                }
                n--;
            }
            delete[] data;
        }
    }
    return res;
}

/**
 * @brief Prepare and send a response
 *
 * @param req
 * @param file
 * @param mime
 * @param gzipped
 * @param res
 */
void httpdRespond(AsyncWebServerRequest *req, const char *file, const char *mime, bool gzipped, AsyncWebServerResponse *res)
{
    String fname = String(FPSTR(file));
    if (res == nullptr)
    {
        res = req->beginResponse(FFat, fname, mime, false, nullptr);
    }
    // unset previosly created cookie
    if (fname == String(FPSTR(resPageLogin)) && req->hasHeader(String(FPSTR(headerCookie))))
    {
        AsyncWebHeader *cookie = req->getHeader(String(FPSTR(headerCookie)));
        if (!(cookie->toString().isEmpty()) && cookie->toString().indexOf(String(FPSTR(cookieNameReset))) == -1)
        {
            res->addHeader(String(FPSTR(headerSetCookie)), String(FPSTR(cookieNameReset)));
        }
    }
    if (gzipped)
    {
        res->addHeader(String(F("Content-Encoding")), String(F("gzip")));
    }
    req->send(res);
}

/**
 * @brief Returns the source of any *.htm page
 *
 * @param req
 */
void httpdGetHtmlPage(AsyncWebServerRequest *req)
{
    String path = req->url();
    const char * fname;

    if (WiFi.getMode() == WIFI_MODE_APSTA)
    {
        fname = resPageSetup;
    }
    else
    {
        if (strlen(session.authToken) != 0 && req->hasHeader(String(FPSTR(headerCookie))))
        {
            AsyncWebHeader *cookie = req->getHeader(String(FPSTR(headerCookie)));
#if DEBUG == 3
            // cookie has the CR already
            __DF(PSTR("has cookie: %s"), cookie->toString().c_str());
#endif
            if (cookie->toString().indexOf(String(session.authToken)) != -1)
            {
#if DEBUG == 3
                __DL(F("(i) authorized"));
#endif
                if (path == String(F("/")))
                {
                    fname = resPageIndex;
                }
                else
                {
                    fname = resPageError;
                }
            }
            else
            {
                // Not allowed
#if DEBUG == 3
                __DL(F("(!) token-cookie no match"));
#endif
                fname = resPageLogin;
            }
        }
        else
        {
            // Not authorized
#if DEBUG == 3
            __DL(F(">> no cookie|token"));
#endif
            fname = resPageLogin;
        }
    }

    httpdRespond(req, fname, mimeTextHtml);
}

/**
 * @brief
 *
 * @param req
 */
void httpdGetHtmlError(AsyncWebServerRequest *req)
{
    if (req->method() == HTTP_GET)
    {
        httpdRespond(req, resPageError, mimeTextHtml);
    }
    else
    {
        httpdJsonErrResponse(req, "not found", 404);
    }
}

void httpdGetStyleChunk(AsyncWebServerRequest *req)
{
    if (req->url() == String(FPSTR(resCommonCss)))
    {
        httpdRespond(req, resCommonCss, mimeTextCss);
    }
    else if (req->url() == String(FPSTR(resLoginCss)))
    {
        httpdRespond(req, resLoginCss, mimeTextCss);
    }
    else if (req->url() == String(FPSTR(resIndexCss)))
    {
        httpdRespond(req, resIndexCss, mimeTextCss);
    }
    else if (req->url() == String(FPSTR(resErrorCss)))
    {
        httpdRespond(req, resErrorCss, mimeTextCss);
    }
    else if (req->url() == String(FPSTR(resSetupCss)))
    {
        httpdRespond(req, resSetupCss, mimeTextCss);
    }
}

void httpdGetScriptChunk(AsyncWebServerRequest *req)
{
    if (req->url() == String(FPSTR(resCommonJs)))
    {
        httpdRespond(req, resCommonJs, mimeAppJS);
    }
    else if (req->url() == String(FPSTR(resLoginJs)))
    {
        httpdRespond(req, resLoginJs, mimeAppJS);
    }
    else if (req->url() == String(FPSTR(resIndexJs)))
    {
        httpdRespond(req, resIndexJs, mimeAppJS);
    }
    else if (req->url() == String(FPSTR(resErrorJs)))
    {
        httpdRespond(req, resErrorJs, mimeAppJS);
    }
    else if (req->url() == String(FPSTR(resSetupJs)))
    {
        httpdRespond(req, resSetupJs, mimeAppJS);
    }
}

void httpdGetFavicon(AsyncWebServerRequest *req)
{
    httpdRespond(req, resFavicon, mimeImgXICN);
}

void httpdGetSvgImage(AsyncWebServerRequest *req)
{
    if (req->url() == String(FPSTR(resSvgEye)))
    {
        httpdRespond(req, resSvgEye, mimeImgSVG);
    }
    else if (req->url() == String(FPSTR(resSvgGth)))
    {
        httpdRespond(req, resSvgGth, mimeImgSVG);
    }
}

void httpdJsonErrResponse(AsyncWebServerRequest *req, const char *descr, const int code) // const __FlashStringHelper * descr) {
{
    String str = String(descr);
    str = "{\"err\": \"" + str + "\"}";
    req->send((int)code, String(FPSTR(mimeAppJSON)), str);
}

/**
 * @brief Fetch API keys into keys object and returs keys count
 *        0 - if there is no keys
 *       -1 - if failed to open file
 *
 * @return api_keys_t
 */
int8_t loadAPIKeys(api_keys_t **keys)
{
    int8_t i = 0;
    fs::File _f;
    if (!FFat.exists(_apiKeysDBPath))
    {
        _f = FFat.open(_apiKeysDBPath, FILE_WRITE);
        _f.close();
    }
    else
    {
        _f = FFat.open(_apiKeysDBPath, FILE_READ);
        if (_f)
        {
            char c;
            String keyData = "";
            // skip consistancy check, rely on addAPIKey()
            while (_f.available())
            {
                c = _f.read();
                if (c != 0x0A)
                {
                    keyData += static_cast<char>(c);
                }
                else
                {
                    api_keys_t *key = new api_keys_t();
                    strcpy(key->key, keyData.substring(0, 32).c_str());   // 32 max
                    strcpy(key->memo, keyData.substring(32, 47).c_str()); // 16 max
                    key->created = static_cast<time_t>(keyData.substring(47).toInt());
                    keys[i] = key;
                    keyData = "";
                    i++;
                }
            }
            _f.close();
        }
        else
            i = -1;
    }
    return i;
}

/**
 * @brief Encode keys data to json format
 *        For careless progr: delete the returned value
 *
 * @return String
 */
char *apiKeysToJSON()
{
    uint8_t i = 0;
    char *t;
    char *api;
    _CHB(t, 0x10);
    _CHB(api, 0x400);
    api_keys_t **data;
    _ALLOC_API_ARRAY(data);
    int8_t n = loadAPIKeys(data);
    if (n > 0)
    {
        while (i < 5 && data[i] != nullptr)
        {
            if (strlen(api) != 0)
                strcat(api, "},");
            val2str(data[i]->created, t);
            strcat(api, "{\"m\":\"");
            strcat(api, data[i]->memo);
            strcat(api, "\",\"k\":\"");
            strcat(api, data[i]->key);
            strcat(api, "\",\"id\":");
            strcat(api, t);
            _CHBC(t);
            i++;
        }
        strcat(api, "}");
    }
    delete[] data;
    _CHBD(t);
    return api;
}

/**
 * @brief Add an API key into file
 *
 * @param k
 * @return true
 * @return false
 */
bool addAPIKey(api_keys_t *k)
{
    api_keys_t **data;
    _ALLOC_API_ARRAY(data);
    int8_t n = loadAPIKeys(data);

    if (n != -1 && n < 5)
    {
        data[n] = k;
        return writeAPIData(data);
    }
#ifdef DEBUG
    __DL("too many api keys");
#endif
    return false;
}
/**
 * @brief Remove the API key record from file
 *
 * @param id
 * @return true
 * @return false
 */
bool removeAPIKey(time_t &id)
{
    bool result = false;
    uint8_t i = 0, k = 0;
    api_keys_t **data;
    _ALLOC_API_ARRAY(data);
    int8_t n = loadAPIKeys(data);
    if (n > 0)
    {
        api_keys_t **dataNew;
        _ALLOC_API_ARRAY(dataNew);
        while (data[i] != nullptr && i < 5)
        {
            if (data[i]->created != id)
            {
                dataNew[k] = data[i];
                k++;
            }
            i++;
        }
        result = writeAPIData(dataNew);
    }
#ifdef DEBUG
    else
    {
        __DL("no API keys or failed to open");
    }
#endif
    delete[] data;
    return result;
}

/**
 * @brief Write data to API keys file
 *
 * @param data
 * @return true
 * @return false
 */
bool writeAPIData(api_keys_t **data)
{
    fs::File _f = FFat.open(_apiKeysDBPath, FILE_WRITE);
    if (!_f || data[0] == nullptr)
        return false;
    char *s, *t;
    size_t m = 0;
    uint8_t i = 0;
    _CHB(s, 72);
    _CHB(t, 24);
    while (data[i] != nullptr && i < 5)
    {
        val2str(data[i]->created, t);
        strcpy(s, data[i]->key);
        // writing exact characters quantity
        strcat(s, data[i]->memo);
        m = strlen(data[i]->memo);
        while (m < sizeof(data[i]->memo))
        {
            strcat(s, " ");
            m++;
        }
        strcat(s, t);
        s[strlen(s)] = 0x0A; // addint LF
        _f.write(reinterpret_cast<const uint8_t *>(s), strlen(s));
        _CHBC(t);
        _CHBC(s);
        i++;
    }
    _f.close();
    delete[] data;
    _CHBD(t);
    _CHBD(s);
    return true;
}

// ██████╗ ███████╗ ██████╗ ██╗   ██╗███████╗███████╗████████╗███████╗
// ██╔══██╗██╔════╝██╔═══██╗██║   ██║██╔════╝██╔════╝╚══██╔══╝██╔════╝
// ██████╔╝█████╗  ██║   ██║██║   ██║█████╗  ███████╗   ██║   ███████╗
// ██╔══██╗██╔══╝  ██║▄▄ ██║██║   ██║██╔══╝  ╚════██║   ██║   ╚════██║
// ██║  ██║███████╗╚██████╔╝╚██████╔╝███████╗███████║   ██║   ███████║
// ╚═╝  ╚═╝╚══════╝ ╚══▀▀═╝  ╚═════╝ ╚══════╝╚══════╝   ╚═╝   ╚══════╝

void httpdPostSiteSurvey(AsyncWebServerRequest *req)
{
    String ssid = "";
    int32_t rssi = 0;
    uint8_t encType = 0;
    uint8_t *bssid;
    int32_t channel = 0;
    // bool hidden = false;
    int8_t result = 0;
    uint8_t cntr = 0;
    String doc = "";
    char *buffer;
    _CHB(buffer, 0x100);

    result = WiFi.scanComplete();

    // Doing one more attempt
    if (result == 0 || result == -2)
    {
        WiFi.scanNetworks(true);
        delay(100);
        result = WiFi.scanComplete();
    }

#if DEBUG == 3
    __DF(PSTR("(i) %d networks:\n"), result);
#endif

    if (result == 0)
    {
#if DEBUG == 3
        __DL(F("(!) no networks"));
#endif
        doc = String(F("{\"err\": \"No networks found\"}"));
    }
    else if (result > 0)
    {
        // print unsorted scan results
        doc = String(F("["));
        while (cntr < result)
        {
            feedLoopWDT();
            WiFi.getNetworkInfo(cntr, ssid, encType, rssi, bssid, channel); // , hidden);

#if DEBUG == 3
            __DF(PSTR(" %02d: %ddBm %s\n"), cntr, rssi, ssid.c_str());
#endif

            sprintf_P(buffer, maskSurvey, ssid.c_str(), rssi, encType);
            doc += String(buffer);
            if (cntr != (result - 1))
            {
                doc += String(F(","));
            }
            cntr++;

            _CHBC(buffer);
        }
        doc += String(F("]"));
    }
    else if (result == -1)
    {
#if DEBUG == 3
        __DL(F("(i) scan in progress"));
#endif
        doc = String(F("{\"delay\":1000}"));
    }
    else
    {
#if DEBUG == 3
        __DL(F("(i) scan repeat"));
#endif
        doc = String(F("{\"delay\":0}"));
    }

    _CHBD(buffer);

    req->send(200, String(FPSTR(mimeAppJSON)), doc);
}

/**
 * @brief doing logout
 *
 * @param req
 */
void httpdGetLogout(AsyncWebServerRequest *req)
{
    AsyncWebServerResponse *res = req->beginResponse(302);
    memset(session.authToken, '\0', sizeof(session.authToken));
    res->addHeader(String(FPSTR(headerLocation)), String(F("/")));
    sysLog.putts(PSTR("(i) %s is logged out"), config.admLogin);
    httpdRespond(req, resPageLogin, mimeTextHtml, true, res);
}

/**
 * @brief Setup page data
 *
 * @param req
 */
void httpdPostSetupInit(AsyncWebServerRequest *req)
{
    if (WiFi.getMode() == WIFI_MODE_APSTA)
    {
        String doc = String("{\"mac\": \"") + WiFi.macAddress() + String("\"}");
        req->send(200, String(FPSTR(mimeAppJSON)), doc);
    }
    else
    {
        httpdJsonErrResponse(req, "setup is already completed", 403);
    }
}

/**
 * @brief Reveive setup data
 *
 * @param req
 */
void httpdPostSetup(AsyncWebServerRequest *req)
{
    if (WiFi.getMode() == WIFI_MODE_APSTA)
    {
        String login = req->arg(String(F("login")));
        String pass = req->arg(String(F("pass")));
        String ssid = req->arg(String(F("ssid")));
        String ssidkey = req->arg(String(F("ssidkey")));
        String doc = String(F("{\"setup\": \"done\"}"));

#if DEBUG == 3
        __D(F("Login:"));
        __DL(login);
        __D(F("Pass:"));
        __DL(pass);
        __D(F("SSID:"));
        __DL(ssid);
        __D(F("SSID pass:"));
        __DL(ssidkey);
#endif

        if (ssid.length() == 0 || ssidkey.length() == 0 || login.length() == 0 || pass.length() == 0)
        {
            doc = String("{\"setup\": \"fail\",\"err\":\"data\"}");
            req->send(200, String(FPSTR(mimeAppJSON)), doc);
            return;
        }

        eemem.setSSID(ssid.c_str());
        eemem.setSSIDKEY(ssidkey.c_str());
        eemem.setAdmLogin(login.c_str());
        eemem.setAdmPassw(pass.c_str());

        eemem.commit();

        req->send(200, String(FPSTR(mimeAppJSON)), doc);
        // doing restart
        delay(1000);
        systemReboot();
    }
    else
    {
        httpdJsonErrResponse(req, "setup is already completed", 403);
    }
}

/**
 * @brief
 *
 * @param req
 */
void httpdPostLogin(AsyncWebServerRequest *req)
{
    char *cookie;
    char *doc;
    _CHB(doc, 64);
    _CHB(cookie, 128);
    bool cookieAuth = false;
    String login = req->arg(String(F("login")));
    String pass = req->arg(String(F("pass")));

    if (login.length() == 0 || pass.length() == 0)
    {
        httpdJsonErrResponse(req, "empty request", 400);
        return;
    }

#if DEBUG == 3
    __D(F("login:"));
    __D(login);
    __D(F(" vs "));
    __DL(config.admLogin);
    __D(F("pass:"));
    __D(pass);
    __D(F(" vs "));
    __DL(config.admPassw);
#endif

    // if already authorized
    if (strlen(session.authToken) != 0)
    {
#ifdef DEBUG
        __DL(F("(!) already logged in"));
#endif
        sysLog.putts(PSTR("(i) %s repeated login attempt"), login.c_str());
        strcpy_P(doc, jsonLoginRepeat);
    }
    else
    {
        if (String(config.admLogin) == login && String(config.admPassw) == pass)
        {
            String token = "";
            hashgen(cookie);
            token = sha1(String(cookie));
            memcpy(session.authToken, token.c_str(), token.length());
            strcpy(cookie, cookieName);
            strcat(cookie, session.authToken);
            strcat(cookie, cookieName2);
            val2str(config.authTimeoutMax, cookie);
            // set login timeout (resets automatically)
            session.authTimeout = millis();
            strcpy_P(doc, jsonLoginOK);
            cookieAuth = true;
#if DEBUG == 3
            __DF(PSTR("(i) %s is logged-in\n"), cookie);
#endif
            sysLog.putts(PSTR("(i) %s is logged-in"), login.c_str());
            WiFi.scanNetworks(true, false);
        }
        else
        {
            sysLog.putts(PSTR("(!) login rejected: %s"), login.c_str());
#ifdef DEBUG
            __DF(PSTR("(!) %s login rejected\n"), login.c_str());
#endif
            strcpy_P(doc, jsonLoginERR);
        }
    }

    AsyncWebServerResponse *res = req->beginResponse(200, String(FPSTR(mimeAppJSON)), doc);
    if (cookieAuth)
    {
        res->addHeader(String(headerSetCookie), String(cookie));
    }

    _CHBD(cookie);

    req->send(res);

    _CHBD(doc);
}

/**
 * @brief returns JSON encoded status data for dashboard
 *
 * @param req
 */
void httpdPostGetDashbrd(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    char *doc;
    char *cb;
    char *dt;
    _CHB(cb, 48);
    _CHB(doc, 512);
    _CHB(dt, 16);

    ntp.getDatetime(cb);
    multi_heap_info_t mem;
    heap_caps_get_info(&mem, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    str2dt(config.BatteryLastReplaceDate, dt);

    sprintf_P(doc,
              maskDashbrd,
              WiFi.localIP().toString().c_str(),
              WiFi.subnetMask().toString().c_str(),
              WiFi.gatewayIP().toString().c_str(),
              WiFi.BSSIDstr().c_str(),
              WiFi.SSID().c_str(),
              WiFi.macAddress().c_str(),
              WiFi.channel(),
              (mem.total_free_bytes / 1024.00),
              (mem.largest_free_block / 1024.0),
              ESP.getCpuFreqMHz(),
              monitor.getSysTemp(),
              monitorData.upsAdvInputLineVoltage,
              (monitorData.upsHighPrecInputFrequency / 10),
              monitorData.upsAdvOutputVoltage,
              monitorData.upsAdvOutputFrequency,
              dt,
              monitor.getBatTemp(),
              snmpagent.isActive(),
              monitorData.upsBasicOutputStatus,
              monitorData.upsBasicBatteryStatus,
              monitorData.upsDiagBatteryStatus,
              monitorData.upsAdvBatteryCapacity,
              monitorData.upsAdvOutputLoad,
              monitor.getUPSLifeTimeSeconds(),
              monitor.isCooling(),
              ntp.uptimeSeconds(),
              cb);

    _CHBD(dt);
    _CHBD(cb);
    req->send(200, String(FPSTR(mimeAppJSON)), doc);
    _CHBD(doc);
}

//  ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗
// ██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝
// ██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗
// ██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║
// ╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝

/**
 * @brief GET SYSTEM
 *
 * @param req
 */
void httpdPostGetConfig(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    char *doc;
    char *b;
    _CHB(b, 0x10);
    _CHB(doc, 0x800);
    str2dt(config.BatteryLastReplaceDate, b);
    char *keys = apiKeysToJSON();

    sprintf_P(doc,
              maskGetConfig,
              config.batteryTempLT,
              config.batteryTempUT,
              config.deviceTempLT,
              config.deviceTempUT,
              config.ntpServer,
              config.ntpServerFB,
              config.ntpSyncInterval,
              config.ntpTimeOffset,
              config.ntpDaylightOffset,
              config.ssid,
              config.ssidkey,
              config.snmpPort,
              config.snmpTrapPort,
              config.sysLocation,
              config.sysContact,
              b,
              config.authTimeoutMax,
              config.snmpGetCN,
              config.snmpSetCN,
              config.admLogin,
              config.admPassw,
              keys);

    _CHBD(b);
    _CHBD(keys);

    req->send(200, String(F(mimeAppJSON)), doc);

    _CHBD(doc);
}

/**
 * @brief POST SYSTEM
 *
 * @param req
 */
void httpdPostSetConfigSystem(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{";

    String battmplt = req->arg(String(F("battmplt")));
    String battmput = req->arg(String(F("battmput")));
    String devtmplt = req->arg(String(F("devtmplt")));
    String devtmput = req->arg(String(F("devtmput")));
    String ntpsrv = req->arg(String(F("ntpsrv")));
    String ntpsrvfb = req->arg(String(F("ntpsrvfb")));
    String ntpsrvsitl = req->arg(String(F("ntpsrvsitl")));
    String ntptmoff = req->arg(String(F("ntptmoff")));
    String ntpdloff = req->arg(String(F("ntpdloff")));
    String ssid = req->arg(String(F("ssid")));
    String ssidkey = req->arg(String(F("ssidkey")));

#if DEBUG == 3
    __D(F("sysfntmp:"));
    __DL(sysfntmp);
    __D(F("ntpsrv:"));
    __DL(ntpsrv);
    __D(F("ntpsrvsitl:"));
    __DL(ntpsrvsitl);
    __D(F("ntptmoff:"));
    __DL(ntptmoff);
    __D(F("ssid:"));
    __DL(ssid);
    __D(F("ssidkey:"));
    __DL(ssidkey);
#endif

    float t1 = 0;
    t1 = (float)atof(battmplt.c_str());
    t1 = (t1 != 0 ? t1 : 60);
    doc += String(F("\"battmplt\":")) + (eemem.setBatteryTempLT(t1) ? String(F("true")) : String(F("false")));

    t1 = (float)atof(battmput.c_str());
    t1 = (t1 != 0 ? t1 : 60);
    doc += String(F(",\"battmput\":")) + (eemem.setBatteryTempUT(t1) ? String(F("true")) : String(F("false")));

    t1 = (float)atof(devtmplt.c_str());
    t1 = (t1 != 0 ? t1 : 65);
    doc += String(F(",\"devtmplt\":")) + (eemem.setDeviceTempLT(t1) ? String(F("true")) : String(F("false")));

    t1 = (float)atof(devtmput.c_str());
    t1 = (t1 != 0 ? t1 : 65);
    doc += String(F(",\"devtmput\":")) + (eemem.setDeviceTempUT(t1) ? String(F("true")) : String(F("false")));

    if (ntpsrv.length() == 0)
        ntpsrv = "pool.ntp.org";
    doc += String(F(",\"ntpsrv\":")) + (eemem.setNTPServer(ntpsrv.c_str()) ? String(F("true")) : String(F("false")));

    if (ntpsrvfb.length() == 0)
        ntpsrvfb = "time.google.com";
    doc += String(F(",\"ntpsrvfb\":")) + (eemem.setNTPServerFB(ntpsrvfb.c_str()) ? String(F("true")) : String(F("false")));

    uint16_t t2 = atoi(ntpsrvsitl.c_str());
    t2 = (t2 != 0 ? t2 : 1800);
    doc += String(F(",\"ntpsrvsitl\":")) + (eemem.setNTPSyncInterval(t2) ? String(F("true")) : String(F("false")));

    if (eemem.setNTPTimeOffset(ntptmoff.c_str()))
    {
        doc += String(F(",\"ntptmoff\":true"));
    }
    else
    {
        doc += String(F(",\"ntptmoff\":false"));
    }

    if (eemem.setNTPDaylightOffset(ntpdloff.c_str()))
    {
        doc += String(F(",\"ntpdloff\":true"));
    }
    else
    {
        doc += String(F(",\"ntpdloff\":false"));
    }

    if (ssid.length() != 0 && ssidkey.length() != 0)
    {
        doc += String(F(",\"ssid\":")) + (eemem.setSSID(ssid.c_str()) ? String(F("true")) : String(F("false")));
        doc += String(F(",\"ssidkey\":")) + (eemem.setSSIDKEY(ssidkey.c_str()) ? String(F("true")) : String(F("false")));
    }
    else
    {
        doc += String(F(",\"ssid\":false,\"ssidkey\":false"));
    }
    eemem.commit();

    doc += String(F("}"));

    req->send(200, String(F(mimeAppJSON)), doc);

    ntp.forceUpdate();
}

/**
 * @brief POST SNMP
 *
 * @param req
 */
void httpdPostSetConfigSNMP(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{";

    String snmpport = req->arg(String(F("snmpport")));
    String snmptrapport = req->arg(String(F("snmptraport")));
    String snmploctn = req->arg(String(F("snmploctn")));
    String snmpcontct = req->arg(String(F("snmpcontct")));
    String snmpbatrpldt = req->arg(String(F("snmpbatrpldt")));

#if DEBUG == 3
    __D(F("snmpport:"));
    __DL(snmpport);
    __D(F("snmploctn:"));
    __DL(snmploctn);
    __D(F("snmpcontct:"));
    __DL(snmpcontct);
    __D(F("snmpbatrpldt:"));
    __DL(snmpbatrpldt);
#endif

    uint16_t t1 = atoi((char *)snmpport.c_str());
    t1 = (t1 != 0 ? t1 : 161);
    doc += String(F("\"snmpport\":")) + (eemem.setSNMPPort(t1) ? String(F("true")) : String(F("false")));

    t1 = atoi(snmptrapport.c_str());
    t1 = (t1 != 0 ? t1 : 162);
    doc += String(F(",\"snmptraport\":")) + (eemem.setSNMPTrapPort(t1) ? String(F("true")) : String(F("false")));

    doc += String(F(",\"snmploctn\":")) + (eemem.setSysLocation(snmploctn.c_str()) ? String(F("true")) : String(F("false")));
    doc += String(F(",\"snmpcontct\":")) + (eemem.setSysContact(snmpcontct.c_str()) ? String(F("true")) : String(F("false")));

    if (snmpbatrpldt.length() != 0)
    {
        char *buffer;
        _CHB(buffer, 24);
        dt2str(snmpbatrpldt.c_str(), buffer);
        doc += String(F(",\"snmpbatrpldt\":")) + (eemem.setBatteryLastReplaceDate(buffer) ? String(F("true")) : String(F("false")));
        _CHBD(buffer);
    }
    else
    {
        doc += String(",\"snmpbatrpldt\":false");
    }
    eemem.commit();

    doc += String("}");

    req->send(200, String(mimeAppJSON), doc);
}

/**
 * @brief Set Security settings
 *
 * @param req
 */
void httpdPostSetConfigSecurity(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{";

    String authtmout = req->arg(String(F("authtmout")));
    String snmpgckey = req->arg(String(F("snmpgckey")));
    String snmpsckey = req->arg(String(F("snmpsckey")));
    String adlogin = req->arg(String(F("adlogin")));
    String adpass = req->arg(String(F("adpass")));

#if DEBUG == 3
    __D(F("authtmout:"));
    __DL(authtmout);
    __D(F("snmpgckey:"));
    __DL(snmpgckey);
    __D(F("snmpsckey:"));
    __DL(snmpsckey);
    __D(F("adlogin:"));
    __DL(adlogin);
    __D(F("adpass:"));
    __DL(adpass);
#endif

    uint16_t t1 = atoi(authtmout.c_str());
    t1 = (t1 != 0 ? t1 : 1800);

    doc += String(F("\"authtmout\":")) + (eemem.setAuthTimeoutMax((uint16_t &)t1) ? String(F("true")) : String(F("false")));
    doc += String(F(",\"snmpgckey\":")) + (eemem.setGetCN(snmpgckey.c_str()) ? String(F("true")) : String(F("false")));
    doc += String(F(",\"snmpsckey\":")) + (eemem.setSetCN(snmpsckey.c_str()) ? String(F("true")) : String(F("false")));

    if (adlogin.length() != 0 && adpass.length() != 0)
    {
        doc += String(F(",\"adlogin\":")) + (eemem.setAdmLogin(adlogin.c_str()) ? String(F("true")) : String(F("false")));
        doc += String(F(",\"adpass\":")) + (eemem.setAdmPassw(adpass.c_str()) ? String(F("true")) : String(F("false")));
    }
    else
    {
        doc += String(F(",\"adlogin\":false,\"adpass\":false"));
    }
    eemem.commit();
    doc += String(F("}"));

    req->send(200, mimeAppJSON, doc);
}

/**
 * @brief Returns raw data from sysLog
 *
 * @param req
 */
void httpdPostSysLog(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    httpdRespond(req, sysLog.getName(), mimeTextPlain, false);
}

/**
 * @brief Returns raw data from httpd_log
 *
 * @param req
 */
void httpdPostSnmpLog(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    httpdRespond(req, snmpLog.getName(), mimeTextPlain, false);
}

/**
 * @brief Returns JSON array of the latest system state parameters
 *
 * @param req
 */
void httpdPostInfoGraph(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    char *ts;
    _CHB(ts, 64);
    char *buffer;
    _CHB(buffer, 128);
    multi_heap_info_t mem;
    heap_caps_get_info(&mem, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    ntp.timestampToString(ts);

    sprintf_P(buffer,
              maskInfoGraph,
              ts,
              monitor.getSysTemp(),
              monitor.getBatTemp(),
              (mem.total_free_bytes / 1024.00),
              (mem.largest_free_block / 1024.0));

    _CHBD(ts);

    req->send(200, mimeAppJSON, buffer);
    _CHBD(buffer);
}

/**
 * @brief Temperature log
 *
 * @param req
 */
void httpdPostMonTmpLog(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    httpdRespond(req, monTempLog.getName(), mimeTextPlain, false);
}

/**
 * @brief Battery/System state data log
 *
 * @param req
 */
void httpdPostMonBDtaLog(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    httpdRespond(req, monDataLog.getName(), mimeTextPlain, false);
}

/**
 * @brief Add an API key
 *
 * @param req
 */
void httpdPostAPIadd(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{";

    String memo = req->arg(String(F("apikm")));
    String key = req->arg(String(F("apik")));

    if (memo != "" && key != "")
    {
        char *keys;
        api_keys_t *ak = new api_keys_t();
        if (key.length() > sizeof(ak->key))
        {
            doc += "\"err\":\"key too large\"}";
            goto sendData;
        }
        if (memo.length() > sizeof(ak->memo))
        {
            doc += "\"err\":\"memo too large\"}";
            goto sendData;
        }
        // memset(ak->key, '\0', sizeof(ak->key));
        ak->created = ntp.getTimestamp();
        strcpy(ak->memo, memo.c_str());
        strcpy(ak->key, key.c_str());
        if (!addAPIKey(ak))
        {
            doc += "\"err\":\"too many keys\"}";
            goto sendData;
        }
        keys = apiKeysToJSON();
        doc += "\"api\": [";
        doc += keys;
        doc += "]}";
        _CHBD(keys);
    }
    else
    {
        doc += "\"err\":\"";
        if (memo == "")
        {
            doc += "empty memo\"}";
        }
        else
        {
            doc += "empty key\"}";
        }
    }
sendData:
    req->send(200, mimeAppJSON, doc);
}

/**
 * @brief
 *
 * @param req
 */
void httpdPostAPIdel(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{";
    char *keys;

    String id = req->arg(String(F("id")));
    if (id == "")
    {
        doc += "\"err\":\"wrong id\"}";
        req->send(200, mimeAppJSON, doc);
        return;
    }
    time_t idv = static_cast<time_t>(atoi(id.c_str()));
    if (!removeAPIKey(idv))
    {
        doc += "\"err\":\"failed\"}";
        req->send(200, mimeAppJSON, doc);
        return;
    }
    keys = apiKeysToJSON();
    doc += "\"api\": [";
    doc += keys;
    doc += "]}";
    _CHBD(keys);
    req->send(200, mimeAppJSON, doc);
}

/**
 * @brief
 *
 * @param req
 */
void httpdPostReboot(AsyncWebServerRequest *req)
{
    if (isAuthorized(req) || WiFi.getMode() == WIFI_MODE_APSTA)
    {
        String doc = "{\"done\":true}";
        req->send(200, mimeAppJSON, doc);
        delay(1000);
        systemReboot();
    }
    else
    {
        httpdJsonErrResponse(req, "auth");
    }
}

/**
 * @brief Toggles cooler fan switch
 *
 * @param req
 */
void httpdPostControlCooling(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{\"isclng\":";
    if (monitor.isCooling())
    {
        monitor.coolingSwitchOff();
        doc += "false}";
        sysLog.putts(PSTR("(i) switch cooler OFF manually"));
    }
    else
    {
        monitor.coolingSwitchOn();
        doc += "true}";
        sysLog.putts(PSTR("(i) switch cooler ON manually"));
    }
    req->send(200, mimeAppJSON, doc);
}

/**
 * @brief Reset the device eemem to defaults values
 *
 * @param req
 */
void httpdPostReset(AsyncWebServerRequest *req)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    String doc = "{\"done\":true}";
    sysLog.putts(PSTR("(i) eemem erase started"));
    eemem.restore();
    req->send(200, mimeAppJSON, doc);
    delay(1000);
    systemReboot();
}