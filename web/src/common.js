/**
 * ? Change the languages that are going to be built into UI
 * ? the very first language here is also a fallback language
 */
const i18nlang = ["en", "es", "ru"];
import "./common.scss";
import "flowbite";
// See: https://github.com/way5/ohSnap-tailwind
import "ohsnap-tailwind/ohsnap.scss";
import { ohSnap, ohSnapX } from "ohsnap-tailwind";
// See: https://www.i18next.com/overview
import i18next from "i18next";
// See: https://github.com/i18next/jquery-i18next
/**
 *
 * @author sk
 *
 * @type {*}
 */
const $i18next = require("jquery-i18next");
// See: https://github.com/i18next/i18next-browser-languageDetector
import LanguageDetector from "i18next-browser-languagedetector";
/**
 *
 * @author sk
 *
 * @type {*}
 */
const i8nResources = () => {
    let a = {};
    for (let l in i18nlang) {
        a[i18nlang[l]] = require("./lang/" + i18nlang[l] + ".json");
    }
    return a;
};

i18next.use(LanguageDetector).init({
    debug: false,
    fallbackLng: i18nlang[0],
    useDataAttrOptions: true,
    resources: i8nResources(),
    detection: {
        lookupQuerystring: "lng",
        caches: ["localStorage", "cookie"],
        order: [
            "querystring",
            "navigator",
            "localStorage",
            "sessionStorage",
            "cookie",
            "htmlTag",
        ],
    },
});
$i18next.init(i18next, $, {
    useOptionsAttr: true,
});

const ohSnapConfig = {
    info: {
        title: $.t("js.alertInfo"),
        styles: {
            bg: "bg-green-500 dark:bg-green-600",
            border: "border-green-700",
            icon: "ohsnap-info bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center",
        },
        duration: 7000,
        container: "body",
        "fade-duration": "fast",
    },
    warn: {
        title: $.t("js.alertWarn"),
        styles: {
            bg: "bg-yellow-500 dark:bg-yellow-600",
            border: "border-yellow-700",
            icon: "ohsnap-warn bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center",
        },
        duration: 7000,
        container: "body",
        "fade-duration": "fast",
    },
    err: {
        title: $.t("js.alertErr"),
        styles: {
            bg: "bg-red-500 dark:bg-red-700",
            border: "border-red-700",
            icon: "ohsnap-err bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center",
        },
        duration: 7000,
        container: "body",
        "fade-duration": "fast",
    },
};

/**
 *
 * @author sk
 *
 * @type {{ surveyUrl: string; rebootURL: string; rebootCountdownIntl: number; err: { title: any; styles: { bg: string; border: string; icon: string; }; duration: number; container: string; 'fade-duration': string; }; ... 7 more ...; doReboot: (el: any, countdownEl: any) => void; }}
 */
var tinyUPS = {
    surveyUrl: "/survey",
    rebootURL: "/reboot",
    rebootCountdownIntl: 10000,
    err: ohSnapConfig.err,
    warn: ohSnapConfig.warn,
    info: ohSnapConfig.info,
    uiVer: null,
    /**
     * Doc-scope initializer. Called manually
     */
    init: function () {
        this.displayMode();
        $("#dmode").on("click", (e) => {
            this.displayMode(true);
        });
        // password unhide
        $("button.showkey").on("click", function (e) {
            e.preventDefault();
            let s = $(this).parent().find("input[type]");
            s.prop("type", s.prop("type") === "text" ? "password" : "text");
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
        var pkg = require("../package.json");
        this.uiVer = pkg.version;
        pkg = null;
        // footer
        $('a[data-i18n="[title]gotoGithubLink"]').append(
            new Date().getFullYear() + " (ui: " + this.uiVer + ")"
        );
        this.initPage();
        // i18n
        $("html").localize();
    },
    /**
     *
     * @param {*} toggle
     */
    displayMode: function (toggle = false) {
        if (
            localStorage.theme === "dark" ||
            (!("theme" in localStorage) &&
                window.matchMedia("(prefers-color-scheme: dark)").matches)
        ) {
            if (toggle) {
                localStorage.theme = "light";
                document.documentElement.classList.remove("dark");
            } else {
                document.documentElement.classList.add("dark");
            }
        } else {
            if (toggle) {
                localStorage.theme = "dark";
                document.documentElement.classList.add("dark");
            } else {
                document.documentElement.classList.remove("dark");
            }
        }
    },
    handleErrorResponse: function (o, ts, e) {
        if (o.status == 401) {
            window.location.reload();
        } else {
            ohSnap($.t("js.errCommunication"), this.err);
        }
    },
    getSurvey: function () {
        let self = this;
        let list = $('select[name="ssid"]');
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.surveyUrl,
            // url: "http://local.ims:8888/?test=1",
            type: 'POST',
            dataType: "json",
            success: (r) => {
                // returned as an object, so the painful version of length
                // create node
                if (r.delay !== undefined) {
                    let delay = parseInt(r.delay);
                    console.log("delay detected: ", delay);
                    ohSnap(
                        delay > 0
                            ? $.t("js.scanRepeatIn", { ds: delay / 1000.0 })
                            : $.t("js.scanInProgress"),
                        self.info
                    );
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
                            case 4:
                                enc = "CCMP";
                                break;
                            case 5:
                                enc = "WEP";
                                break;
                            case 7:
                                enc = "NONE";
                                break;
                            case 8:
                                enc = "AUTO";
                                break;
                        }
                        node.value = r[i].s;
                        node.innerHTML =
                            r[i].s + " (ENC: " + enc + " RSSI: " + r[i].r + ")";
                        list.append(node);
                    }
                    ohSnap($.t("js.scanComplete"), self.info);
                } else ohSnap($.t("js.errNoWiFiDetect"), self.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // getSurvey
    doReboot: function (el, countdownEl) {
        let self = this;
        $(el).attr("disabled", "disabled");
        $(el).addClass("disabled");
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.rebootURL,
            // url: "http://local.ims:8888/?test=34",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    if (r.done === true) {
                        ohSnap($.t("js.deviceIsRebooting"), self.info);
                        let countdown = self.rebootCountdownIntl / 1000;
                        $(countdownEl).html(countdown);
                        setInterval(() => {
                            countdown -= 1;
                            if (countdown >= 0) {
                                $(countdownEl).html(countdown);
                                if (countdown == 0) {
                                    location.reload();
                                }
                            }
                        }, 1000);
                    }
                    // else
                    // console.log('wrong answer: ', r);
                } else ohSnap($.t("js.errDoReboot"), self.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // doReboot
};

window.$ = jQuery;
window.tinyUPS = tinyUPS;

export { tinyUPS, i18next, $i18next, ohSnap, ohSnapX };
