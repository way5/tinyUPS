//
const isProd = process.env.NODE_ENV === "production";
/**
 * REST API URLs
*/
const $baseURL = isProd ? window.location.protocol + "//" + window.location.hostname : 'http://localhost:8888';
const $restURL = {
    // index
    getCfgUrl: `${$baseURL}/getconfig`,
    sysSetCfgUrl: `${$baseURL}/setconfigsys`,
    snmpSetCfgUrl: `${$baseURL}/setconfigsnmp`,
    secSetCfgUrl: `${$baseURL}/setconfigsec`,
    updateUrl: `${$baseURL}/update`,
    getDashDataUrl: `${$baseURL}/getdashbrd`,
    tempChartUrl: `${$baseURL}/montmpr`,
    dataChartUrl: `${$baseURL}/monbdata`,
    infoGraphUrl: `${$baseURL}/infograph`,
    resetUrl: `${$baseURL}/reset`,
    genSerial: `${$baseURL}/genserial`,
    // addAPIkey: `${$baseURL}/addapikey`,
    // delAPIkey: `${$baseURL}/delapikey`,
    toggleCoolingUrl: `${$baseURL}/coolingctrl`,
    // login
    loginUrl: `${$baseURL}/login`,
    // common
    surveyUrl: `${$baseURL}/survey`,
    rebootURL: `${$baseURL}/reboot`,
    // setup
    setupUrl: `${$baseURL}/setup`,
    setupPageDataUrl: `${$baseURL}/setup-init`,
};

export {
    $baseURL,
    $restURL
}