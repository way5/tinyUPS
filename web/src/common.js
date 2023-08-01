/**
 * ? Change the languages that are going to be built into UI
 * ? the very first language here is also a fallback language
 */
const i18nlang = ['en', 'es', 'ru'];
import "./common.scss";
import 'flowbite';
// See: http://wavded.github.io/humane-js/
const humane = require("./humane.js");
// See: https://github.com/way5/ohSnap-tailwind
import { ohSnap, ohSnapX } from "oh-snap";
// See: https://www.i18next.com/overview
import i18next from 'i18next';
// See: https://github.com/i18next/jquery-i18next
/**
 *
 * @author sk
 *
 * @type {*}
 */
const $i18next = require('jquery-i18next');
// See: https://github.com/i18next/i18next-browser-languageDetector
import LanguageDetector from 'i18next-browser-languagedetector';
/**
 *
 * @author sk
 *
 * @type {*}
 */
const i8nResources = () => {
    let a = {};
    for(let l in i18nlang) {
        a[i18nlang[l]] = require("./lang/"+i18nlang[l]+".json");
    }
    return a;
};

i18next.use(LanguageDetector).init({
    debug: true,
    fallbackLng: i18nlang[0],
    useDataAttrOptions: true,
    resources: i8nResources(),
    detection: {
        lookupQuerystring: 'lng',
        caches: ['localStorage', 'cookie'],
        order: [
            'querystring',
            'navigator',
            'localStorage',
            'sessionStorage',
            'cookie',
            'htmlTag',
        ],
    }
});
$i18next.init(i18next, $, {
    useOptionsAttr: true
});

/**
 *
 * @author sk
 *
 * @type {{ surveyUrl: string; rebootURL: string; rebootCountdownIntl: number; showNotifyIntl: number; err: any; info: any; done: any; commons: any; displayMode: (toggle?: any) => void; getSurvey: () => void; doReboot: (el: any, countdownEl: any) => void; }}
 */
var tinyUPS =  {
    surveyUrl: "/survey",
    rebootURL: "/reboot",
    rebootCountdownIntl: 10000,
    showNotifyIntl: 7000,
    err: null,
    info: null,
    done: null,
    /**
     * Doc-scope initializer. Called manually
     */
    init: function () {
        this.displayMode();
        $('#dmode').on('click', (e) => {
            this.displayMode(true);
        });
        let cont = document.querySelector("header");
        this.info = humane.create({
            addnCls: "info",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: this.showNotifyIntl,
            timeout: this.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        this.err = humane.create({
            addnCls: "error",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: this.showNotifyIntl,
            timeout: this.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        this.done = humane.create({
            addnCls: "success",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: this.showNotifyIntl,
            timeout: this.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        // password unhide
        $("button.showkey").on("click", function (e) {
            e.preventDefault();
            let s = $(this).parent().find("input[type]");
            s.prop("type", (s.prop("type") === "text" ? "password" : "text"));
            if (s.prop("type") === "text") {
                $(this).addClass("active");
            } else {
                $(this).removeClass("active");
            }
        });
        // survey
        $("#dosurvey").on("click", (e) => {
            e.preventDefault();
            this.getSurvey();
        });
        this.initPage();
        // i18n
        $('body').localize();
    },
    /**
     *
     * @param {*} toggle
     */
    displayMode: function (toggle = false) {
        if (localStorage.theme === 'dark' || (!('theme' in localStorage)
                && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
            if (toggle) {
                localStorage.theme = 'light';
                document.documentElement.classList.remove('dark');
            } else {
                document.documentElement.classList.add('dark');
            }
        } else {
            if (toggle) {
                localStorage.theme = 'dark';
                document.documentElement.classList.add('dark');
            } else {
                document.documentElement.classList.remove('dark');
            }
        }
    },
    handleErrorResponse: (o, ts, e) => {
        if (o.status == 403) {
            window.location.reload();
        }
    },
    getSurvey: function () {
        let self = this;
        let list = $('select[name="ssid"]');
        $.ajax({
            url: window.location.protocol + "//" + window.location.hostname + this.surveyUrl,
            // url: "http://local.ims:8888/?test=1",
            type: 'POST',
            type: 'json',
            success: (r) => {
                // returned as an object, so the painful version of length
                // create node
                if (r.delay !== undefined) {
                    let delay = parseInt(r.delay);
                    console.log("delay detected: ", delay);
                    self.info((delay > 0 ? $.t('js.scanRepeatIn', { ds: (delay / 1000.0) }) : $.t("js.scanInProgress")));
                    setTimeout(() => {
                        self.getSurvey();
                    }, delay);
                } else if (r.length != 0) {
                    self.scanResult = r;
                    list.html("");
                    for (let i = 0; i < r.length; i++) {
                        if (r[i].s === "") continue;
                        let node = document.createElement("option");
                        let enc = "TKIP";
                        switch (r[i].e) {
                            case 4: enc = "CCMP"; break;
                            case 5: enc = "WEP"; break;
                            case 7: enc = "NONE"; break;
                            case 8: enc = "AUTO"; break;
                        }
                        node.value = r[i].s;
                        node.innerHTML = r[i].s + " (ENC: " + enc + " RSSI: " + r[i].r + ")";
                        list.append(node);
                    }
                } else {
                    self.err($.t('js.errNoWiFiDetect'));
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },                  // getSurvey
    doReboot: function (el, countdownEl) {
        let self = this;
        $(el).attr("disabled", "disabled");
        $(el).addClass("disabled");
        $.ajax({
            url: window.location.protocol + "//" + window.location.hostname + this.rebootURL,
            // url: "http://local.ims:8888/?test=34",
            dataType: 'json',
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    if (r.done === true) {
                        self.info($.t('js.deviceIsRebooting'));
                        let countdown = (self.rebootCountdownIntl / 1000);
                        $(countdownEl).html(countdown);
                        setInterval(() => {
                            countdown -= 1;
                            if(countdown >= 0) {
                                $(countdownEl).html(countdown);
                                if (countdown == 0) {
                                    location.reload();
                                }
                            }
                        }, 1000);
                    } else {
                        console.log('wrong answer: ', r);
                    }
                } else
                    self.err($.t('js.errDoReboot'));
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },                  // doReboot
};

window.$ = jQuery;
window.tinyUPS = tinyUPS;

export {
    tinyUPS,
    i18next,
    $i18next
};