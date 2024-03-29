/*
#####################################################################################
# File: httpd.cpp                                                                   #
# File Created: Monday, 22nd May 2023 4:02:52 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 9th January 2024 2:32:40 pm                               #
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
    httpd.on("/", HTTP_GET, httpdGetHtmlPage);
    httpd.on("/logout", HTTP_GET, httpdGetLogout);
    httpd.on("/c.css", HTTP_GET, httpdGetStyleChunk);
    httpd.on("/l.css", HTTP_GET, httpdGetStyleChunk);
    httpd.on("/i.css", HTTP_GET, httpdGetStyleChunk);
    httpd.on("/e.css", HTTP_GET, httpdGetStyleChunk);
    httpd.on("/s.css", HTTP_GET, httpdGetStyleChunk);
    httpd.on("/c.js", HTTP_GET, httpdGetScriptChunk);
    httpd.on("/l.js", HTTP_GET, httpdGetScriptChunk);
    httpd.on("/i.js", HTTP_GET, httpdGetScriptChunk);
    httpd.on("/e.js", HTTP_GET, httpdGetScriptChunk);
    httpd.on("/s.js", HTTP_GET, httpdGetScriptChunk);
    httpd.on(resFavicon, HTTP_GET, httpdGetFavicon);
    httpd.on(resSvgGth, HTTP_GET, httpdGetSvgImage);
    httpd.on(resSvgEye, HTTP_GET, httpdGetSvgImage);
    // POST
    httpd.on("/survey", HTTP_POST, httpdPostSiteSurvey);
    httpd.on("/setup-init", HTTP_POST, httpdPostSetupInit);
    httpd.on("/setup", HTTP_POST, httpdPostSetup);
    httpd.on("/login", HTTP_POST, httpdPostLogin);
    httpd.on("/logsys", HTTP_POST, httpdPostSysLog);
    httpd.on("/logsnmp", HTTP_POST, httpdPostSnmpLog);
    httpd.on("/infograph", HTTP_POST, httpdPostInfoGraph);
    httpd.on("/montmpr", HTTP_POST, httpdPostMonTmpLog);
    httpd.on("/monbdata", HTTP_POST, httpdPostMonBDtaLog);
    httpd.on("/addapikey", HTTP_POST, httpdPostAPIadd);
    httpd.on("/delapikey", HTTP_POST, httpdPostAPIdel);
    // DASHBOARD DATA
    httpd.on("/getdashbrd", HTTP_POST, httpdPostGetDashbrd);
    // CONFIG
    httpd.on("/getconfig", HTTP_POST, httpdPostGetConfig);
    httpd.on("/setconfigsys", HTTP_POST, httpdPostSetConfigSystem);
    httpd.on("/setconfigsnmp", HTTP_POST, httpdPostSetConfigSNMP);
    httpd.on("/setconfigsec", HTTP_POST, httpdPostSetConfigSecurity);
    // SYSTEM
    httpd.on("/reboot", HTTP_POST, httpdPostReboot);
    httpd.on("/coolingctrl", HTTP_POST, httpdPostControlCooling);
    httpd.on("/reset", HTTP_POST, httpdPostReset);
    httpd.on("/update", HTTP_POST, httpdPostUpgradeResponder, httpdPostUpgradeReceiver);

    httpd.onNotFound(httpdGetHtmlError);

    httpd.begin();

    systemEvent.isActiveHttpd = true;
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
    strcat(c, _httpTokenSalt);
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
    if (strlen(session.authToken) != 0 && req->hasHeader(String(headerCookie)))
    {
        AsyncWebHeader *cookie = req->getHeader(String(headerCookie));
        if (cookie->toString().indexOf(String(session.authToken)) != -1)
        {
            res = true;
        }
    }
    else if (req->method() == HTTP_POST && req->hasParam(String("key")))
    {
        AsyncWebParameter *k = req->getParam(String("key"));
        if (k->value().length() != 32)
        {
#ifdef DEBUG
            __DF("wrong key lengh: %s\n", k->value().c_str());
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
    String fname = String(file);
    if (res == nullptr)
    {
        res = req->beginResponse(FFat, fname, mime, false, nullptr);
    }
    // res->addHeader(String("Cache-Control"), String("max-age=30"));
    // unset previosly created cookie
    if (fname == String(resPageLogin) && req->hasHeader(String(headerCookie)))
    {
        AsyncWebHeader *cookie = req->getHeader(String(headerCookie));
        if (!(cookie->toString().isEmpty()) && cookie->toString().indexOf(String(cookieNameReset)) == -1)
        {
            res->addHeader(String(headerSetCookie), String(cookieNameReset));
        }
    }
    if (gzipped)
    {
        res->addHeader(String("Content-Encoding"), String("gzip"));
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
        if (strlen(session.authToken) != 0 && req->hasHeader(headerCookie))
        {
            AsyncWebHeader *cookie = req->getHeader(headerCookie);
#if DEBUG == 3
            // cookie has the CR already
            __DF("has cookie: %s", cookie->toString().c_str());
#endif
            if (cookie->toString().indexOf(String(session.authToken)) != -1)
            {
#if DEBUG == 3
                __DL("(i) authorized");
#endif
                if (path.compareTo("/") == 0)
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
                __DL("(!) token-cookie no match");
#endif
                fname = resPageLogin;
            }
        }
        else
        {
            // Not authorized
#if DEBUG == 3
            __DL("no cookie|token");
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
    httpdRespond(req, req->url().c_str(), mimeTextCss);
}

void httpdGetScriptChunk(AsyncWebServerRequest *req)
{
    httpdRespond(req, req->url().c_str(), mimeAppJS);
}

void httpdGetFavicon(AsyncWebServerRequest *req)
{
    httpdRespond(req, resFavicon, mimeImgXICN);
}

void httpdGetSvgImage(AsyncWebServerRequest *req)
{
    if (req->url() == String(resSvgEye))
    {
        httpdRespond(req, resSvgEye, mimeImgSVG);
    }
    else if (req->url() == String(resSvgGth))
    {
        httpdRespond(req, resSvgGth, mimeImgSVG);
    }
}

void httpdJsonErrResponse(AsyncWebServerRequest *req, const char *descr, const int code) // const __FlashStringHelper * descr) {
{
    String str = String(descr);
    str = "{\"err\": \"" + str + "\"}";
    req->send((int)code, String(mimeAppJSON), str);
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
        delay(100);
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
                    strcpy(key->memo, keyData.substring(32, 48).c_str()); // 16 max
                    key->created = static_cast<time_t>(keyData.substring(48).toInt());
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
    char *s, *t;
    size_t m = 0;
    uint8_t i = 0;
    fs::File _f = FFat.open(_apiKeysDBPath, FILE_WRITE);
    if (!_f) {
        delete[] data;
        return false;
    }
    // if an empty array given. nothing to do with it.
    if(data[0] == nullptr)
        goto write_api_data;
    _CHB(s, 128);
    _CHB(t, 24);
    while (data[i] != nullptr && i < 5)
    {
        val2str(data[i]->created, t);
        strcpy(s, data[i]->key);
        // writing exact characters quantity
        strcat(s, data[i]->memo);
        m = strlen(data[i]->memo);
        while (m < 16UL)
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
    _CHBD(t);
    _CHBD(s);
write_api_data:
    delete[] data;
    _f.close();
    return true;
}

// ██████╗ ███████╗ ██████╗ ██╗   ██╗███████╗███████╗████████╗███████╗
// ██╔══██╗██╔════╝██╔═══██╗██║   ██║██╔════╝██╔════╝╚══██╔══╝██╔════╝
// ██████╔╝█████╗  ██║   ██║██║   ██║█████╗  ███████╗   ██║   ███████╗
// ██╔══██╗██╔══╝  ██║▄▄ ██║██║   ██║██╔══╝  ╚════██║   ██║   ╚════██║
// ██║  ██║███████╗╚██████╔╝╚██████╔╝███████╗███████║   ██║   ███████║
// ╚═╝  ╚═╝╚══════╝ ╚══▀▀═╝  ╚═════╝ ╚══════╝╚══════╝   ╚═╝   ╚══════╝

/**
 * @brief Upgrade process result responder
 *
*/
void httpdPostUpgradeResponder(AsyncWebServerRequest *req) {
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON), 128);

    if (!updaterInProgress() && Update.hasError())
    {
        res->setCode(500);
        res->printf(jsonUpdateERR, Update.errorString());
    #if DEBUG == 2
        __DF("(!) update failed: %s\n", Update.errorString());
    #endif
        // logsys.putts("(!) update error: %s", Update.errorString());
    }
    else if(updaterHasErrors())
    {
        res->setCode(500);
        res->printf(jsonUpdateERR, String(updaterLastError()).c_str());
    #if DEBUG == 2
        __DF("(!) filesystem update failed: %d\n", updaterLastError());
    #endif
    }
    else
        res->print(jsonUpdateOK);

    res->addHeader(String("Connection"), String("close"));
    req->send(res);

    systemReboot();
}

/**
 * @brief Upgrade process handler
 *
*/
void httpdPostUpgradeReceiver(AsyncWebServerRequest *req, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!isAuthorized(req))
    {
        httpdJsonErrResponse(req, "auth");
        return;
    }
    char const * err = nullptr;

    if(!index)
    {
    #if DEBUG == 2
        __DF("received update: %s\n", filename.c_str());
    #endif
        logsys.putts(PSTR("received update: %s"), filename.c_str());

        systemEvent.updateInProgress = true;

        if (filename.indexOf("filesystem") != -1)
        {
            // stop snmp
            snmpagent.kill();
            // disconnect FS
            FFat.end();
            // prepare buffer and partition
            if(!updaterGetReady()) {
            #if DEBUG == 2
                __DF("(!) partition access error\n");
            #endif
                // logsys.putts("(!) partition access error");
            }
        }
        else
        {
            if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000), U_FLASH)
            {
                err = Update.errorString();
            #if DEBUG == 2
                __DF("(!) firmware update init error: %s\n", err);
            #endif
                logsys.put(PSTR("(!) firmware update init error: %s"), err);
                goto upgrade_end;
            }
        }
    #if DEBUG == 2
        __D("[upgrading].");
    #endif
    }

    // receive data & confirm
    if(updaterInProgress()) {
        if(!updaterWriteData(data, len, final)) {
            goto upgrade_fatal;
        }
    #if DEBUG == 2
        __D(F("."));
    #endif
    }
    else if(!Update.hasError())
    {
        if(Update.write(data, len) != len)
        {
        #if DEBUG == 2
            err = Update.errorString();
            __DF(PSTR("(!) firmware write fatal error: %s\n"), err);
        #endif
            // logsys.put(PSTR("(!) update error: %s"), err);
            goto upgrade_fatal;
        }
    #if DEBUG == 2
        else {
            __D(F("."));
        }
    #endif
    }
    else
    {
    #if DEBUG == 2
        err = Update.errorString();
        __DF(PSTR("(!) update error: %s\n"), err);
        // logsys.put(PSTR("(!) update error: %s"), err);
    #endif
        goto upgrade_fatal;
    }

    // if the data end is reached
    if(final)
    {
        if(updaterInProgress())
        {
        #if DEBUG == 2
            __DF("\n(i) filesystem update success / file size: %uB\n", (index + len));
        #endif
        }
        else if(Update.end(true))
        {
        #if DEBUG == 2
            __DF("\n(i) firmware update success / file size: %uB\n", (index + len));
        #endif
            logsys.putts("(i) firmware update success / file size: %uB\n", (index + len));
        }
        else
        {
            err = Update.errorString();
        #if DEBUG == 2
            __DF(PSTR("\n(!) unexpected termination on update: %s\n"), err);
        #endif
            logsys.put(PSTR("(!) unexpected termination on update: %s\n"), err);
        }
    }

upgrade_end:
    delay(10);
    return;

upgrade_fatal:
    systemReboot();
}

/**
 * @brief Network survey data
 *
 * @param req
 */
void httpdPostSiteSurvey(AsyncWebServerRequest *req)
{
    String ssid;
    int16_t result = 0;

    result = WiFi.scanComplete();
    if (result == 0 || result == -2) {
        // Doing one more attempt
        result = WiFi.scanNetworks(true);
        while(result == -1) {
            yield();
            result = WiFi.scanComplete();
        }
    }

#if DEBUG == 3
    __DF("(i) %d networks:\n", result);
#endif

    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON), 1024);

    if (result == 0)
    {
    #if DEBUG == 3
        __DL("(!) no networks");
    #endif
        res->print("{\"err\": \"No networks found\"}");
    }
    else if (result > 0)
    {
        uint8_t cntr = 0;
        int32_t rssi = 0;
        uint8_t encType = 0;
        uint8_t * bssid;
        int32_t channel = 0;
        // print unsorted scan results
        res->print("[");
        while (cntr < result)
        {
            WiFi.getNetworkInfo(cntr, ssid, encType, rssi, bssid, channel); // , hidden);

        #if DEBUG == 3
            __DF(" %02d: %ddBm %s\n", cntr, rssi, ssid.c_str());
        #endif

            res->printf(maskSurvey, ssid.c_str(), rssi, encType);
            // doc += String(buffer);
            if (cntr != (result - 1))
            {
                res->print(",");
            }
            cntr++;

        }

        res->print("]");
    }
    else if (result == -1)
    {
    #if DEBUG == 3
        __DL("(i) scan in progress");
    #endif
        res->print("{\"delay\":1000}");
    }
    else
    {
    #if DEBUG == 3
        __DL("(i) scan repeat");
    #endif
        res->print("{\"delay\":0}");
    }
    // WiFi.scanDelete();

    req->send(res);
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
    res->addHeader(String(headerLocation), String("/"));
    logsys.putts("(i) %s is logged out", config.admLogin);
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
        String doc = "{\"mac\": \"" + WiFi.macAddress() + "\"}";
        req->send(200, String(mimeAppJSON), doc);
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
        String login = req->arg("login");
        String pass = req->arg("pass");
        String ssid = req->arg("ssid");
        String ssidkey = req->arg("ssidkey");
        String doc = "{\"setup\": \"done\"}";

    #if DEBUG == 3
        __DF("Login: %s\n", login);
        __DF("Pass: %s\n", pass);
        __DF("SSID: %s\n", ssid);
        __DF("SSID pass: %s\n", ssidkey);
    #endif

        if (ssid.length() == 0 || ssidkey.length() == 0 || login.length() == 0 || pass.length() == 0)
        {
            doc = String("{\"setup\": \"fail\",\"err\":\"data\"}");
            req->send(200, String(mimeAppJSON), doc);
            return;
        }

        eemem.setSSID(ssid.c_str());
        eemem.setSSIDKEY(ssidkey.c_str());
        eemem.setAdmLogin(login.c_str());
        eemem.setAdmPassw(pass.c_str());

        eemem.commit();

        req->send(200, String(mimeAppJSON), doc);
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
    String login = req->arg("login");
    String pass = req->arg("pass");

    if (login.length() == 0 || pass.length() == 0)
    {
        httpdJsonErrResponse(req, "empty request", 400);
        return;
    }

#if DEBUG == 3
    __DF("login: %s vs %s\n", login, config.admLogin);
    __DF("pass: %s vs %s\n", pass, config.admPassw);
#endif

    // if already authorized
    if (strlen(session.authToken) != 0)
    {
    #ifdef DEBUG
        __DL("(!) already logged in");
    #endif
        logsys.putts("(i) %s repeated login attempt", login.c_str());
        strcpy(doc, jsonLoginRepeat);
    }
    else
    {
        if (strcmp(config.admLogin, login.c_str()) == 0 && strcmp(config.admPassw, pass.c_str()) == 0)
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
            strcpy(doc, jsonLoginOK);
            cookieAuth = true;
        #if DEBUG == 3
            __DF("(i) %s is logged-in\n", cookie);
        #endif
            logsys.putts("(i) %s is logged-in", login.c_str());
            WiFi.scanNetworks(true, false);
        }
        else
        {
            logsys.putts("(!) login rejected: %s", login.c_str());
        #ifdef DEBUG
            __DF("(!) %s login rejected\n", login.c_str());
        #endif
            strcpy(doc, jsonLoginERR);
        }
    }

    AsyncWebServerResponse *res = req->beginResponse(200, String(mimeAppJSON), doc);
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

    multi_heap_info_t * mem = new multi_heap_info_t();
    heap_caps_get_info(mem, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));

    res->printf(maskDashbrd01,
                WiFi.localIP().toString().c_str(),
                WiFi.subnetMask().toString().c_str(),
                WiFi.gatewayIP().toString().c_str(),
                WiFi.BSSIDstr().c_str(),
                WiFi.SSID().c_str(),
                WiFi.macAddress().c_str(),
                (mem->total_free_bytes / 1024.00),
                (mem->largest_free_block / 1024.0)
                );
    res->printf(maskDashbrd02,
                monitor.getSysTemp(),
                monitorData.upsAdvInputLineVoltage,
                (monitorData.upsHighPrecInputFrequency / 10),
                monitorData.upsAdvOutputVoltage,
                monitorData.upsAdvOutputFrequency,
                monitor.getBatTemp(),
                systemEvent.isActiveSnmpAgent,
                monitorData.upsBasicOutputStatus,
                monitorData.upsBasicBatteryStatus,
                monitorData.upsDiagBatteryStatus,
                monitorData.upsAdvBatteryCapacity);
    res->printf(maskDashbrd03,
                monitorData.upsAdvOutputLoad,
                monitor.getUPSLifeTimeSeconds(),
                monitor.isCooling(),
                ntp.uptimeSeconds(),
                ntp.getTimestamp());

    delete mem;

    req->send(res);
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
    char *b;
    _CHB(b, 0x10);
    str2dt(config.BatteryLastReplaceDate, b);
    char *keys = apiKeysToJSON();

    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));

    res->printf(maskGetConfig01,
                config.batteryTempLT,
                config.batteryTempUT,
                config.deviceTempLT,
                config.deviceTempUT,
                config.ntpServer,
                config.ntpServerFB,
                config.ntpSyncInterval);
    res->printf(maskGetConfig02,
                config.ntpTimeOffset,
                config.ntpDaylightOffset,
                config.ssid,
                config.ssidkey,
                config.snmpPort,
                config.snmpTrapPort,
                config.sysLocation);
    res->printf(maskGetConfig03,
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

    req->send(res);
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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{");

    String battmplt = req->arg("battmplt");
    String battmput = req->arg("battmput");
    String devtmplt = req->arg("devtmplt");
    String devtmput = req->arg("devtmput");
    String ntpsrv = req->arg("ntpsrv");
    String ntpsrvfb = req->arg("ntpsrvfb");
    String ntpsrvsitl = req->arg("ntpsrvsitl");
    String ntptmoff = req->arg("ntptmoff");
    String ntpdloff = req->arg("ntpdloff");
    String ssid = req->arg("ssid");
    String ssidkey = req->arg("ssidkey");

#if DEBUG == 3
    __DF("battmplt: %s\n", battmplt);
    __DF("battmput: %s\n", battmput);
    __DF("ntpsrv: %s\n", ntpsrv);
    __DF("ntpsrvsitl: %s\n", ntpsrvsitl);
    __DF("ntptmoff: %s\n", ntptmoff);
    __DF("ssid: %s\n", ssid);
    __DF("ssidkey: %s\n", ssidkey);
#endif

    float t1 = 0;
    t1 = (float)atof(battmplt.c_str());
    t1 = (t1 != 0 ? t1 : 60);
    res->print("\"battmplt\":");
    res->print(eemem.setBatteryTempLT(t1) ? String("true") : String("false"));

    t1 = (float)atof(battmput.c_str());
    t1 = (t1 != 0 ? t1 : 60);
    res->print(",\"battmput\":");
    res->print(eemem.setBatteryTempUT(t1) ? String("true") : String("false"));

    t1 = (float)atof(devtmplt.c_str());
    t1 = (t1 != 0 ? t1 : 65);
    res->print(",\"devtmplt\":");
    res->print(eemem.setDeviceTempLT(t1) ? String("true") : String("false"));

    t1 = (float)atof(devtmput.c_str());
    t1 = (t1 != 0 ? t1 : 65);
    res->print(",\"devtmput\":");
    res->print(eemem.setDeviceTempUT(t1) ? String("true") : String("false"));

    if (ntpsrv.length() == 0)
        ntpsrv = "pool.ntp.org";
    res->print(",\"ntpsrv\":");
    res->print(eemem.setNTPServer(ntpsrv.c_str()) ? String("true") : String("false"));

    if (ntpsrvfb.length() == 0)
        ntpsrvfb = "time.google.com";
    res->print(",\"ntpsrvfb\":");
    res->print(eemem.setNTPServerFB(ntpsrvfb.c_str()) ? String("true") : String("false"));

    uint16_t t2 = atoi(ntpsrvsitl.c_str());
    t2 = (t2 != 0 ? t2 : 1800);
    res->print(",\"ntpsrvsitl\":");
    res->print(eemem.setNTPSyncInterval(t2) ? String("true") : String("false"));

    if (eemem.setNTPTimeOffset(ntptmoff.c_str()))
    {
        res->print(",\"ntptmoff\":true");
    }
    else
    {
        res->print(",\"ntptmoff\":false");
    }

    if (eemem.setNTPDaylightOffset(ntpdloff.c_str()))
    {
        res->print(",\"ntpdloff\":true");
    }
    else
    {
        res->print(",\"ntpdloff\":false");
    }

    if (ssid.length() != 0 && ssidkey.length() != 0)
    {
        res->print(",\"ssid\":");
        res->print(eemem.setSSID(ssid.c_str()) ? String("true") : String("false"));
        res->print(",\"ssidkey\":");
        res->print(eemem.setSSIDKEY(ssidkey.c_str()) ? String("true") : String("false"));
    }
    else
    {
        res->print(",\"ssid\":false,\"ssidkey\":false");
    }
    eemem.commit();

    res->print("}");

    req->send(res);

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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{");

    String snmpport = req->arg("snmpport");
    String snmptrapport = req->arg("snmptraport");
    String snmploctn = req->arg("snmploctn");
    String snmpcontct = req->arg("snmpcontct");
    String snmpbatrpldt = req->arg("snmpbatrpldt");

#if DEBUG == 3
    __DF("snmpport: %s\n", snmpport);
    __DF("snmploctn: %s\n", snmploctn);
    __DF("snmpcontct: %s\n", snmpcontct);
    __DF("snmpbatrpldt: %s\n", snmpbatrpldt);
#endif

    uint16_t t1 = atoi((char *)snmpport.c_str());
    t1 = (t1 != 0 ? t1 : 161);
    res->print("\"snmpport\":");
    res->print(eemem.setSNMPPort(t1) ? String("true") : String("false"));

    t1 = atoi(snmptrapport.c_str());
    t1 = (t1 != 0 ? t1 : 162);
    res->print(",\"snmptraport\":");
    res->print(eemem.setSNMPTrapPort(t1) ? String("true") : String("false"));

    res->print(",\"snmploctn\":");
    res->print(eemem.setSysLocation(snmploctn.c_str()) ? String("true") : String("false"));
    res->print(",\"snmpcontct\":");
    res->print(eemem.setSysContact(snmpcontct.c_str()) ? String("true") : String("false"));

    if (snmpbatrpldt.length() != 0)
    {
        char *buffer;
        _CHB(buffer, 24);
        dt2str(snmpbatrpldt.c_str(), buffer);
        res->print(",\"snmpbatrpldt\":");
        res->print(eemem.setBatteryLastReplaceDate(buffer) ? String("true") : String("false"));
        _CHBD(buffer);
    }
    else
    {
        res->print(",\"snmpbatrpldt\":false");
    }
    eemem.commit();

    res->print("}");

    req->send(res);
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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{");

    String authtmout = req->arg("authtmout");
    String snmpgckey = req->arg("snmpgckey");
    String snmpsckey = req->arg("snmpsckey");
    String adlogin = req->arg("adlogin");
    String adpass = req->arg("adpass");

#if DEBUG == 3
    __DF("authtmout: %s\n", authtmout);
    __DF("snmpgckey: %s\n", snmpgckey);
    __DF("snmpsckey: %s\n", snmpsckey);
    __DF("adlogin: %s\n", adlogin);
    __DF("adpass: %s\n", adpass);
#endif

    uint16_t t1 = atoi(authtmout.c_str());
    t1 = (t1 != 0 ? t1 : 1800);

    res->print("\"authtmout\":");
    res->print(eemem.setAuthTimeoutMax((uint16_t &)t1) ? String("true") : String("false"));
    res->print(",\"snmpgckey\":");
    res->print(eemem.setGetCN(snmpgckey.c_str()) ? String("true") : String("false"));
    res->print(",\"snmpsckey\":");
    res->print(eemem.setSetCN(snmpsckey.c_str()) ? String("true") : String("false"));

    if (adlogin.length() != 0 && adpass.length() != 0)
    {
        res->print(",\"adlogin\":");
        res->print(eemem.setAdmLogin(adlogin.c_str()) ? String("true") : String("false"));
        res->print(",\"adpass\":");
        res->print(eemem.setAdmPassw(adpass.c_str()) ? String("true") : String("false"));
    }
    else
    {
        res->print(",\"adlogin\":false,\"adpass\":false");
    }
    eemem.commit();
    res->print("}");

    req->send(res);
}

/**
 * @brief Returns raw data from logsys
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
    httpdRespond(req, logsys.getName(), mimeTextPlain, false);
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
    httpdRespond(req, logsnmp.getName(), mimeTextPlain, false);
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
    _CHB(ts, 24);
    multi_heap_info_t * mem = new multi_heap_info_t();
    heap_caps_get_info(mem, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    ntp.timestampToString(ts);
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));

    res->printf(maskInfoGraph,
                ts,
                monitor.getSysTemp(),
                monitor.getBatTemp(),
                (mem->total_free_bytes / 1024.00),
                (mem->largest_free_block / 1024.0));

    _CHBD(ts);
    delete mem;

    req->send(res);
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
    httpdRespond(req, logTempMon.getName(), mimeTextPlain, false);
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
    httpdRespond(req, logDataMon.getName(), mimeTextPlain, false);
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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{");

    String memo = req->arg(String("apikm"));
    String key = req->arg(String("apik"));

#if DEBUG == 3
    __DF("memo: %s (%d)\n", memo.c_str(), memo.length());
    __DF("key: %s (%d)\n", key.c_str(), key.length());
#endif

    if (memo != "" && key != "")
    {
        char *keys;
        api_keys_t *ak = new api_keys_t();
        if (key.length() > 32)
        {
            res->print("\"err\":\"key too large\"}");
            goto sendData;
        }
        if (memo.length() > 16)
        {
            res->print("\"err\":\"memo too large\"}");
            goto sendData;
        }
        // memset(ak->key, '\0', sizeof(ak->key));
        ak->created = ntp.getTimestamp();
        strcpy(ak->memo, (memo.substring(0, 16)).c_str());
        strcpy(ak->key, (key.substring(0, 32)).c_str());
        if (!addAPIKey(ak))
        {
            res->print("\"err\":\"too many keys\"}");
            goto sendData;
        }
        keys = apiKeysToJSON();
        res->printf("\"api\": [%s]}", keys);
        _CHBD(keys);
    }
    else
    {
        res->print("\"err\":\"");
        if (memo == "")
        {
            res->print("empty memo\"}");
        }
        else
        {
            res->print("empty key\"}");
        }
    }

sendData:
    req->send(res);
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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{");
    char *keys;

    String id = req->arg(String("id"));
    if (id == "")
    {
        res->print("\"err\":\"wrong id\"}");
        req->send(res);
        return;
    }
    time_t idv = static_cast<time_t>(atoi(id.c_str()));
    if (!removeAPIKey(idv))
    {
        res->print("\"err\":\"failed\"}");
        req->send(res);
        return;
    }
    keys = apiKeysToJSON();
    res->printf("\"api\": [%s]}", keys);
    _CHBD(keys);
    req->send(res);
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
        req->send(200, mimeAppJSON, "{\"done\":true}");
        delay(100);
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
    AsyncResponseStream *res = req->beginResponseStream(String(mimeAppJSON));
    res->print("{\"isclng\":");
    if (monitor.isCooling())
    {
        monitor.coolingSwitchOff();
        res->print("false}");
        logsys.putts("(i) switch cooler OFF manually");
    }
    else
    {
        monitor.coolingSwitchOn();
        res->print("true}");
        logsys.putts("(i) switch cooler ON manually");
    }
    req->send(res);
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
    logsys.putts("(i) eemem erase started");
    eemem.restore();
    req->send(200, mimeAppJSON, "{\"done\":true}");
    delay(100);
    systemReboot();
}