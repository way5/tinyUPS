import "./index.scss";
import { tinyUPS, ohSnap, ohSnapX } from "./common.js";
// See: https://www.npmjs.com/package/crypto-js
import sha1 from "crypto-js/sha1";
// See: https://www.chartjs.org/docs/latest
import { Chart } from "chart.js/auto";
// See: https://github.com/chartjs/awesome#adapters
import "chartjs-adapter-luxon";
// See: https://github.com/chartjs/chartjs-adapter-date-fns
import "chartjs-adapter-date-fns";
import {
    BubbleMatrixElement,
    BubbleMatrixScale,
    BubbleMatrixController,
} from "chartjs-chart-bubblematrix";
// See: https://nagix.github.io/chartjs-plugin-streaming/master/guide
import StreamingPlugin from "@u306060/chartjs-plugin-streaming";
// See: https://www.chartjs.org/chartjs-plugin-zoom/latest
import zoomPlugin from "chartjs-plugin-zoom";

Chart.register(
    BubbleMatrixController,
    BubbleMatrixScale,
    BubbleMatrixElement,
    StreamingPlugin,
    zoomPlugin
);

/**
 * Do element scroll to bottom
 * @param {*} o
 */
var containerScrollBottom = (o) => {
    o.scroll({ top: o.scrollHeight, behavior: "smooth" });
};

/**
 *
 * @author sk
 *
 * @param {*} i
 * @param {number} [digiLen=2]
 * @returns {*}
 */
var preZerosDigitFormat = (i, digiLen = 2) => {
    return ("0".repeat(digiLen) + i).slice(digiLen * -1);
};

/**
 *
 * @author sk
 *
 * @param {*} t
 * @returns {string}
 */
var secondsToHRts = (t) => {
    let year, day, hour, min;
    year = day = hour = min = 0;
    let ts = "";
    if (t > 31556952) {
        year = Math.floor(t / 31556952);
        t = t % 31556952;
        ts = year + " " + $.t("js.year", { count: year}) + " ";
    }
    if (t > 86400) {
        day = Math.floor(t / 86400);
        t = t % 86400;
        ts = ts + day + " " + $.t("js.day", { count: day }) + " ";
    }
    if (t > 3600) {
        hour = Math.floor(t / 3600);
        t = t % 3600;
        ts = ts + hour + " " + $.t("js.hr", { count: hour }) + " ";
    }
    if (t > 60 && day == 0) {
        min = Math.floor(t / 60);
        t = t % 60;
        ts = ts + min + " " + $.t("js.min") + " ";
    }
    if (t > 0 && hour == 0) {
        ts = ts + t + " " + $.t("js.sec");
    }
    return ts;
};

/**
 *
 * @author sk
 *
 * @param {*} s
 * @returns {*}
 */
var outputStatusToSting = (s) => {
    let st = $.t("js.outstUnknown");
    switch (s) {
        default:
        case 1:
            break;
        case 2:
            st = $.t("js.outstOnLine");
            break;
        case 3:
            st = $.t("js.outstOnBattery");
            break;
        case 4:
            st = $.t("js.outstOnSmartBoost");
            break;
        case 5:
            st = $.t("js.outstTimedSleeping");
            break;
        case 6:
            st = $.t("js.outstSoftwareBypass");
            break;
        case 7:
            st = $.t("js.outstOff");
            break;
        case 8:
            st = $.t("js.outstRebooting");
            break;
        case 9:
            st = $.t("js.outstSwitchedBypass");
            break;
        case 10:
            st = $.t("js.outstHardwareFailureBypass");
            break;
        case 11:
            st = $.t("js.outstSleepingUntilPowerReturn");
            break;
        case 12:
            st = $.t("js.outstOnSmartTrim");
            break;
        case 13:
            st = $.t("js.outstEcoMode");
            break;
        case 14:
            st = $.t("js.outstHotStandby");
            break;
        case 15:
            st = $.t("js.outstOnBatteryTest");
            break;
    }
    return st;
};

/**
 *
 * @author sk
 *
 * @param {*} s
 * @returns {*}
 */
var batteryDiagStatusToString = (s) => {
    let t = $.t("js.battStNormal");
    switch (s) {
        case 3:
        default:
            break;
        case 4:
            t = $.t("js.battStFault");
            break;
        case 7:
            t = $.t("js.battStCapacityLow");
            break;
    }
    return t;
};

/**
 *
 * @author sk
 *
 * @param {*} t
 * @returns {*}
 */
var batteryStatusToString = (t, alterIndicators = true) => {
    let r = $.t("js.battStNormal");
    if (alterIndicators) {
        $(".indicators-wrapper").removeClass("battery-low-alert");
        $(".indicators-wrapper").removeClass("battery-faulty-alert");
        $("menu.sidebar li:first-child").removeClass("battery-low-nav-alert");
        $("menu.sidebar li:first-child").removeClass(
            "battery-faulty-nav-alert"
        );
    }
    switch (t) {
        default:
        case 1:
            alterIndicators &&
                $("#led934").attr("class", "blink-led fill-[#7a7a7a]");
            r = $.t("js.battStUnknown");
            break;
        case 2:
            alterIndicators &&
                $("#led934").attr("class", "blink-led fill-[#8df478]");
            break;
        case 3:
            if (alterIndicators) {
                $("#led934").attr("class", "blink-led fill-[#ffda90]");
                // swith UI innto notification mode
                $(".indicators-wrapper").addClass("battery-low-alert");
                $("menu.sidebar li:first-child").addClass(
                    "battery-low-nav-alert"
                );
            }
            r = $.t("js.battStLow");
            break;
        case 4:
            if (alterIndicators) {
                $("#led934").attr("class", "blink-led fill-[#ff9898]");
                $(".indicators-wrapper").addClass("battery-faulty-alert");
                $("menu.sidebar li:first-child").addClass(
                    "battery-faulty-nav-alert"
                );
            }
            r = $.t("js.battStFaulty");
            break;
    }
    return r;
};

$.extend(tinyUPS, {
    getCfgUrl: "/getconfig",
    sysSetCfgUrl: "/setconfigsys",
    snmpSetCfgUrl: "/setconfigsnmp",
    secSetCfgUrl: "/setconfigsec",
    getDashDataUrl: "/getdashbrd",
    tempChartUrl: "/montmpr",
    dataChartUrl: "/monbdata",
    infoGraphUrl: "/infograph",
    resetUrl: "/reset",
    addAPIkey: "/addapikey",
    delAPIkey: "/delapikey",
    toggleCoolingUrl: "/coolingctrl",
    logsys: null,
    logsnmp: null,
    dashboardData: {},
    // local variables
    scanResult: null,
    tctChartData: [],
    pwstChartData: [],
    configData: null,
    refreshLogsIntl: 15000,
    refreshDashIntl: 12000,
    refreshtempChartIntl: 10000,
    resetCountdownIntl: 12000,
    // charts
    charts: {},
    intls: {},
    opchOptions: {
        plugins: {
            streaming: {
                duration: 900000,
                ttl: 1100000,
                frameRate: 10,
                pause: false,
                delay: 10000,
            },
            legend: {
                display: true,
                position: "bottom",
                labels: {
                    color:
                        localStorage.theme === "dark"
                            ? "rgba(255, 255, 255, 1)"
                            : "rgba(52, 41, 85, 1)",
                    boxWidth: 10,
                    boxHeight: 10,
                    padding: 16,
                },
            },
            title: {
                display: false,
            },
            zoom: {
                pan: {
                    enabled: true,
                    mode: "x",
                },
                zoom: {
                    wheel: {
                        enabled: true,
                    },
                    pinch: {
                        enabled: true,
                    },
                    mode: "x",
                },
                limits: {
                    x: {
                        minDelay: null,
                        maxDelay: null,
                        minDuration: 60000,
                        maxDuration: 1100000,
                    },
                },
            },
            tooltip: {
                position: "average",
                usePointStyle: true,
                backgroundColor:
                    localStorage.theme === "dark"
                        ? "rgba(62, 46, 87, 0.8)"
                        : "rgba(116, 83, 204, 0.8)",
                bodyFont: {
                    family: "Exo",
                },
                mode: "nearest",
                intersect: false,
            },
        },
        grid: {
            display: true,
            color:
                localStorage.theme === "dark"
                    ? "rgba(228, 228, 228, 0.2)"
                    : "rgba(164, 120, 247, 0.5)",
            // lineWidth: 0.4,
            tickLength: 5,
            tickColor:
                localStorage.theme === "dark"
                    ? "rgba(255, 176, 176, 1)"
                    : "rgba(253, 77, 77, 1)",
            offset: true,
            // tickBorderDash: []
        },
        tick: {
            color:
                localStorage.theme === "dark"
                    ? "rgba(227, 227, 227, 0.8)"
                    : "rgba(90, 70, 147, 0.8)",
        },
    },
    initPage: function () {
        // APPLY PAGE CONTENTS
        const self = this;
        let lhash = location.hash;
        if (lhash !== "") {
            // make corresponding objects active
            $(".sidebar li.item").each(function (i) {
                if ($(this).attr("href") === lhash) $(this).addClass("active");
                else $(this).removeClass("active");
            });
            // page contents
            $("div.pagecontainer").each(function (i) {
                let pid = $(this).attr("id");
                if (pid === lhash.slice(1, lhash.length)) {
                    $(this).removeClass("hidden");
                    $(this).addClass("grid");
                } else {
                    $(this).removeClass("grid");
                    $(this).addClass("hidden");
                }
            });
        }
        // service menu
        $(".cooler-switch-onoff").on("click", (e) => {
            self.toggleCooling();
        });
        // sidebar
        this.initSidebar();
        // hide menus
        $("menu").on("mouseleave", function () {
            $(this).addClass("hidden");
        });
        // tabs
        $(".tabselector").each(function (i) {
            let ts = this;
            $(ts)
                .find("a.tab")
                .on("click", function (e) {
                    e.preventDefault();
                    let tgt = $(this).attr("href");
                    // tabs
                    $(this)
                        .parent()
                        .find("a.tab")
                        .each(function (i) {
                            $(this).removeClass("active");
                        });
                    $(this).addClass("active");
                    // contents
                    $(ts)
                        .find(".tab-conts")
                        .each(function (i) {
                            if ($(this).prop("id") === tgt)
                                $(this).removeClass("hidden");
                            else $(this).addClass("hidden");
                        });
                });
        });
        // flatnav
        $(".flatnav").each(function (i) {
            let o = this;
            $(o)
                .find("a")
                .each(function (i) {
                    let el = this;
                    $(el).on("click", function (e) {
                        e.preventDefault();
                        let id = $(el).attr("href");
                        // hide all
                        $(o)
                            .find("a")
                            .each(function (i) {
                                let id = $(this).attr("href");
                                $(this).removeClass("active");
                                $(id).addClass("hidden");
                            });
                        // current tab
                        $(el).addClass("active");
                        $(id).removeClass("hidden");
                        // callback if exists
                        let cb = $(el).attr("aria-click-callback");
                        if (cb != null) {
                            // let cbf = new Function("o", "e", cb + "($(o)[0], $(e)[0])");
                            let cbf = new Function(cb);
                            // ATTN: What to do in case if not a tab container but the whole tab would be needed (arg #1)?
                            // cbf($(id)[0].firstChild, el);
                            cbf();
                        }
                    });
                });
        });
        // logs
        this.logsys = new logArea($("#logsys"));
        this.logsys.reload();
        this.logsnmp = new logArea($("#logsnmp"));
        this.logsnmp.reload();
        // hashchange
        $(window).on("hashchange", (e) => {
            switch (location.hash) {
                case "#home":
                    this.getDashData();
                    this.logsys.reload();
                    this.logsnmp.reload();
                    break;
                case "#conf":
                    this.getSurvey();
                    this.getCfg();
                    break;
            }
        });
        if (location.hash === "#home" || location.hash === "") this.getDashData();
        window.dispatchEvent(new Event("hashchange"));
        // submit config
        $("input[name=configsys]").on("click", function (e) {
            e.preventDefault();
            self.setSysCfg();
        });
        $("input[name=configsnmp]").on("click", function (e) {
            e.preventDefault();
            self.setSNMPCfg();
        });
        $("input[name=configsec]").on("click", function (e) {
            e.preventDefault();
            self.setSecCfg();
        });
        // do refresh logs
        if (localStorage.getItem("logarf") === "true") {
            this.setRefreshLogs(localStorage.getItem("logarf"));
            $("#log-aref-tg").attr("checked", true);
        } else {
            $("#log-aref-tg").removeAttr("checked");
        }
        $("#log-aref-tg").on("change", function () {
            let v = $(this).prop("checked");
            self.setRefreshLogs(v);
            localStorage.setItem("logarf", v);
        });
        if (localStorage.getItem("logarf") === "true") {
            $("#log-aref-tg").attr("checked", true);
        }
        // update dashboard every refreshdashintl
        this.intls["dd"] = setInterval(function () {
            self.getDashData();
        }, this.refreshDashIntl);
        // reboot button
        $("#modal-rbt-alert button.submit").on("click", function (e) {
            // let el = e.target || e.srcElement;
            self.doReboot(this, "#cntr831");
        });
        //reset button
        $("#modal-rst-alert button.submit").on("click", function (e) {
            // let el = e.target || e.srcElement;
            self.doReset(this, "#cntr833");
        });
        // init charts
        this.initCharts();
        // chart refresh buttons
        $("#pwst-chart-reload").on("click", function () {
            self.pwstChartDataReload();
            ohSnap($.t("js.chartReloaded"), self.info);
        });
        $("#trecs-chart-reload").on("click", function () {
            self.tctChartDataReload();
            ohSnap($.t("js.chartReloaded"), self.info);
        });

        // hide loader
        setTimeout(function () {
            $("#loader").addClass("hidden");
        }, 1000);

        // API table
        $('[data-i18n="index.apiKeyBtnAdd"]').on("click", async () => {
            const dt = new Date();
            const form = $('form[name="add-api-key-form"]');
            // let key = await strToHash(Math.random().toString().slice(2) + dt.getTime().toString()).catch(console.error);
            let key = sha1(
                Math.random().toString().slice(2) + dt.getTime().toString()
            ).toString();
            key = key.slice(0, 32);
            const formAPIk = form.find("input[name=apik]");
            const formAPIkStr = form.children("p").eq(2);
            formAPIkStr.html(key);
            formAPIk.val(key);
        });
        $('form[name="add-api-key-form"]').on("submit", (e) => {
            e.preventDefault();
            self.createAPIkey();
            return false;
        });
    }, // initPage
    toggleCooling: function () {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.toggleCoolingUrl,
            // url: "http://local.ims:8888/?test=33",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.isclng === true) {
                    ohSnap($.t("index.js.coolerIsOn"), this.info);
                } else if (r.isclng === false) {
                    ohSnap($.t("index.js.coolerIsOff"), this.info);
                } else ohSnap($.t("index.js.failedCoolerOnOff"), this.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // toggleCooling
    createAPIkey: function () {
        const self = this;
        const form = $('form[name="add-api-key-form"]');
        const data = form.serializeArray();
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.addAPIkey,
            // url: "http://local.ims:8888/?test=36",
            dataType: "json",
            type: 'POST',
            data: data,
            success: (r) => {
                if (r.api !== undefined) {
                    ohSnap($.t("index.js.apiKeyCreated"), self.info);
                    self.appendAPIKeys(r.api);
                    $(form)[0].reset();
                } else
                    ohSnap(
                        $.t("js.errDataNotSaved") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
            },
            error: (o, ts, e) => {
                self.handleErrorResponse(o, ts, e);
            },
        });
    }, // createAPIkey
    appendAPIKeys: function (api) {
        const self = this;
        $("#api-keys tbody").html("");
        if (api.length != 0) {
            $.each(api, function (k, v) {
                const tr = $("<tr>", { class: "row" });
                let ts = new Date(v.id * 1000);
                const td0 = $("<td>", { scope: "row" });
                td0.html(
                    v.m +
                        '<p class="text-sm text-gray-500 dark:text-base-200">' +
                        $.t("js.createdAt") +
                        ": " +
                        ts.toLocaleString() +
                        "</p>"
                );
                const td1 = $("<td>");
                td1.html(v.k);
                const lnk = $("<a>", { class: "link", href: v.id });
                lnk.html($.t("js.btnDelete"));
                lnk.on("click", function (e) {
                    e.preventDefault();
                    self.removeAPIKey(v.id);
                    return false;
                });
                const td2 = $("<td>");
                td2.append(lnk);
                tr.append(td0);
                tr.append(td1);
                tr.append(td2);
                $("#api-keys tbody").append(tr);
            });
        } else {
            const tr = $("<tr>", { class: "row" });
            const td0 = $("<td>", { scope: "row", colspan: "3" });
            td0.html(
                '<p class="w-full text-center text-base">' +
                    $.t("index.js.noAPIKeys") +
                    "</p>"
            );
            tr.append(td0);
            $("#api-keys tbody").html(tr);
        }
    }, // appendAPIKey
    removeAPIKey: function (id) {
        const self = this;
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.delAPIkey,
            // url: "http://local.ims:8888/?test=37",
            dataType: "json",
            type: 'POST',
            data: { id: id },
            success: (r) => {
                if (r.api !== undefined) {
                    ohSnap($.t("index.js.apiKeyDeleted"), self.info);
                    self.appendAPIKeys(r.api);
                } else
                    ohSnap(
                        $.t("js.errDataNotSaved") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        self.err
                    );
            },
            error: (o, ts, e) => {
                self.handleErrorResponse(o, ts, e);
            },
        });
    }, // removeAPIKey
    initCharts: function () {
        // @remind temerature charts
        this.charts["optmp"] = new Chart(
            document.getElementById("op-chart-tmp").getContext("2d"),
            {
                type: "line",
                data: {
                    datasets: [
                        {
                            label: $.t("index.js.opchDeviceTemp"),
                            data: [],
                            borderColor: "rgba(245, 109, 4, 1)",
                            backgroundColor: "rgba(245, 108, 4, 0.4)",
                            fill: true,
                            cubicInterpolationMode: "monotone",
                        },
                        {
                            label: $.t("index.js.opchBatteryTemp"),
                            data: [],
                            borderColor: "rgba(225, 3, 40, 1)",
                            backgroundColor: "rgba(225, 3, 40, 0.4)",
                            fill: true,
                            cubicInterpolationMode: "monotone",
                        },
                    ],
                },
                options: {
                    maintainAspectRatio: false,
                    // animation: true,
                    plugins: this.opchOptions.plugins,
                    scales: {
                        x: {
                            grid: this.opchOptions.grid,
                            type: "realtime",
                            ticks: this.opchOptions.tick,
                        },
                        y: {
                            grid: this.opchOptions.grid,
                            title: {
                                display: true,
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(139, 114, 194, 1.0)"
                                        : "rgba(111, 88, 179, 1)",
                                text: $.t("index.js.opchTitleTemp"),
                            },
                            ticks: this.opchOptions.tick,
                        },
                    },
                    // tooltips: this.opchOptions.tooltips,
                },
            }
        );
        // @remind ram chart
        this.charts["opram"] = new Chart(
            document.getElementById("op-chart-ram").getContext("2d"),
            {
                type: "line",
                data: {
                    datasets: [
                        {
                            label: $.t("index.js.opchTotalRam"),
                            data: [],
                            borderColor: "rgba(56, 201, 73, 1)",
                            backgroundColor: "rgba(74, 232, 93, 0.4)",
                            fill: true,
                            cubicInterpolationMode: "monotone",
                        },
                        {
                            label: $.t("index.js.opchRAMmaxFreeBlk"),
                            data: [],
                            borderColor: "rgba(39, 127, 26, 1)",
                            backgroundColor: "rgba(48, 129, 35, 0.6)",
                            fill: true,
                            cubicInterpolationMode: "monotone",
                        },
                    ],
                },
                options: {
                    maintainAspectRatio: false,
                    // animation: true,
                    plugins: this.opchOptions.plugins,
                    scales: {
                        x: {
                            grid: this.opchOptions.grid,
                            type: "realtime",
                            ticks: this.opchOptions.tick,
                        },
                        y: {
                            grid: this.opchOptions.grid,
                            title: {
                                display: true,
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(139, 114, 194, 1.0)"
                                        : "rgba(111, 88, 179, 1)",
                                text: $.t("index.js.opchTitleRAM"),
                            },
                            ticks: this.opchOptions.tick,
                        },
                    },
                },
            }
        );
        // rt chart
        this.intls["opch"] = setInterval(
            () => this.opchChartDataReload(),
            this.refreshtempChartIntl
        );
        this.opchChartDataReload();
        // @remind power status matrix
        // let weekMin = new Date();
        // weekMin.setHours(0,0,0,0);
        // weekMin.setMonth(0);
        // weekMin.setDate(1);
        // let weekMax = new Date();
        // weekMax.setHours(23, 59, 59, 99);
        // weekMax.setMonth(11);
        // weekMax.setDate(31);
        this.charts["pwrmx"] = new Chart(
            document.getElementById("pwr-chart").getContext("2d"),
            {
                type: "bubblematrix",
                data: {
                    datasets: [
                        {
                            // power/output status
                            data: [],
                            pointStyle: "circle",
                            hitRadius: 3,
                            hoverBorderWidth: 3,
                            borderColor: "rgba(242, 119, 38, 1)",
                            backgroundColor: "rgba(237, 177, 57, 0.8)",
                        },
                        {
                            // battery status
                            data: [],
                            pointStyle: "triangle",
                            rotation: 60,
                            hitRadius: 3,
                            hoverBorderWidth: 3,
                            borderColor: "rgba(242, 38, 38, 1)",
                            backgroundColor: "rgba(237, 60, 57, 0.8)",
                        },
                    ],
                },
                options: {
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            display: false,
                        },
                        title: {
                            display: false,
                        },
                        /* zoom: {
                            pan: {
                                enabled: true,
                                mode: "x",
                                scaleMode: 'x',
                                threshold: 100
                            },
                            zoom: {
                                wheel: {
                                    enabled: true,
                                },
                                pinch: {
                                    enabled: true,
                                },
                                mode: "x",
                            },
                            limits: {
                                x: {
                                    min: weekMin.getTime(),
                                    max: weekMax.getTime(),
                                    minRange: 604800000 * 3,
                                },
                            },
                        }, */
                        tooltip: {
                            position: "average",
                            usePointStyle: true,
                            backgroundColor:
                                localStorage.theme === "dark"
                                    ? "rgba(62, 46, 87, 0.8)"
                                    : "rgba(116, 83, 204, 0.8)",
                            bodyFont: {
                                family: "Exo",
                            },
                            mode: "nearest",
                            intersect: false,
                            callbacks: {
                                title(ct, it) {
                                    return "";
                                },
                                label(ct, it) {
                                    // console.log(ct);
                                    let ttp = [];
                                    if (ct.datasetIndex === 0) {
                                        let st = new Date(
                                            ct.raw.x
                                        ).toLocaleDateString();
                                        let lng = secondsToHRts(ct.raw.l);
                                        ttp = [
                                            " " +
                                                $.t(
                                                    "index.js.powerOutageDetected"
                                                ) +
                                                ": " +
                                                st,
                                            "   - " +
                                                $.t(
                                                    "index.js.powerOutageDuration"
                                                ) +
                                                ": " +
                                                lng,
                                            "   - " +
                                                $.t(
                                                    "index.js.powerOutageUPSPrg"
                                                ) +
                                                ": ",
                                        ];
                                        $.each(ct.raw.prg, (i, v) => {
                                            let tm = new Date(v[0]);
                                            ttp.push(
                                                "     " +
                                                    outputStatusToSting(v[1]) +
                                                    " (" +
                                                    preZerosDigitFormat(
                                                        tm.getHours()
                                                    ) +
                                                    ":" +
                                                    preZerosDigitFormat(
                                                        tm.getMinutes()
                                                    ) +
                                                    ":" +
                                                    preZerosDigitFormat(
                                                        tm.getSeconds()
                                                    ) +
                                                    ")"
                                            );
                                        });
                                    } else if (ct.datasetIndex === 1) {
                                        let st = new Date(ct.raw.x);
                                        ttp = [
                                            " " +
                                                $.t(
                                                    "index.js.batteryStatusChanged"
                                                ) +
                                                ": " +
                                                ct.raw.e,
                                            " " +
                                                $.t(
                                                    "index.js.batteryStatusSince"
                                                ) +
                                                ": " +
                                                preZerosDigitFormat(
                                                    st.getHours()
                                                ) +
                                                ":" +
                                                preZerosDigitFormat(
                                                    st.getMinutes()
                                                ) +
                                                ":" +
                                                preZerosDigitFormat(
                                                    st.getSeconds()
                                                ),
                                        ];
                                    }
                                    return ttp;
                                },
                                footer(ct, it) {
                                    return "";
                                },
                            },
                        },
                    },
                    scales: {
                        x: {
                            position: "top",
                            isoWeekday: true,
                            border: {
                                display: false,
                            },
                            ticks: {
                                maxRotation: 90,
                                font: {
                                    size: 10,
                                },
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(227, 227, 227, 0.8)"
                                        : "rgba(90, 70, 147, 0.8)",
                            },
                            grid: {
                                display: true,
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(228, 228, 228, 0.2)"
                                        : "rgba(164, 120, 247, 0.5)",
                                // lineWidth: 0.4,
                                tickLength: 5,
                                tickColor:
                                    localStorage.theme === "dark"
                                        ? "rgba(255, 176, 176, 1)"
                                        : "rgba(253, 77, 77, 1)",
                                offset: true,
                            },
                        },
                        y: {
                            reverse: true,
                            position: "right",
                            isoWeekday: true,
                            border: {
                                display: false,
                            },
                            ticks: {
                                maxRotation: 0,
                                font: {
                                    size: 10,
                                },
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(227, 227, 227, 0.8)"
                                        : "rgba(90, 70, 147, 0.8)",
                            },
                            grid: {
                                display: true,
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(228, 228, 228, 0.2)"
                                        : "rgba(164, 120, 247, 0.5)",
                                // lineWidth: 1,
                                tickLength: 5,
                                tickColor:
                                    localStorage.theme === "dark"
                                        ? "rgba(255, 176, 176, 1)"
                                        : "rgba(253, 77, 77, 1)",
                                offset: true,
                            },
                        },
                    },
                },
            }
        );
        // update now
        this.pwstChartDataReload();
        // @remind trecs chart
        let zMinTct = new Date();
        zMinTct.setHours(0, 0, 0, 0);
        let zMaxTct = new Date();
        zMaxTct.setHours(23, 59, 59, 99);
        this.charts["trecs"] = new Chart(
            document.getElementById("trecs-chart").getContext("2d"),
            {
                type: "line",
                data: {
                    datasets: [
                        {
                            label: $.t("index.js.opchDeviceTemp"),
                            data: [],
                            borderColor: "rgba(245, 109, 4, 1)",
                            backgroundColor: "rgba(245, 108, 4, 0.4)",
                            cubicInterpolationMode: "monotone",
                        },
                        {
                            label: $.t("index.js.opchBatteryTemp"),
                            data: [],
                            borderColor: "rgba(225, 3, 40, 1)",
                            backgroundColor: "rgba(225, 3, 40, 0.4)",
                            cubicInterpolationMode: "monotone",
                        },
                    ],
                },
                options: {
                    maintainAspectRatio: false,
                    // animation: true,
                    plugins: {
                        legend: {
                            display: true,
                            position: "bottom",
                            labels: {
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(255, 255, 255, 1)"
                                        : "rgba(52, 41, 85, 1)",
                                boxWidth: 10,
                                boxHeight: 10,
                                padding: 16,
                            },
                        },
                        title: {
                            display: false,
                        },
                        zoom: {
                            pan: {
                                enabled: true,
                                mode: "x",
                                scaleMode: "x",
                                threshold: 30,
                            },
                            zoom: {
                                wheel: {
                                    enabled: true,
                                },
                                pinch: {
                                    enabled: true,
                                },
                                mode: "x",
                            },
                            limits: {
                                x: {
                                    min: zMinTct.getTime() - 11037600000,
                                    max: zMaxTct.getTime() + 11037600000,
                                    minRange: 3600000,
                                },
                            },
                        },
                        tooltip: {
                            position: "average",
                            usePointStyle: true,
                            backgroundColor:
                                localStorage.theme === "dark"
                                    ? "rgba(62, 46, 87, 0.8)"
                                    : "rgba(116, 83, 204, 0.9)",
                            bodyFont: {
                                family: "Exo",
                            },
                            mode: "nearest",
                            intersect: false,
                        },
                    },
                    scales: {
                        x: {
                            grid: this.opchOptions.grid,
                            type: "time",
                            ticks: this.opchOptions.tick,
                        },
                        y: {
                            grid: this.opchOptions.grid,
                            title: {
                                display: true,
                                color:
                                    localStorage.theme === "dark"
                                        ? "rgba(139, 114, 194, 1.0)"
                                        : "rgba(111, 88, 179, 1)",
                                text: $.t("index.js.opchTitleTemp"),
                            },
                            ticks: this.opchOptions.tick,
                        },
                    },
                },
            }
        );
        // filter change
        $("#dc-period").on("change", () => {
            this.tctChartRedraw();
        });
        // update now
        this.tctChartDataReload();
    }, // initCharts
    // @remind poll data for opcharts
    opchChartDataReload: function () {
        const self = this;
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.infoGraphUrl,
            // url: "http://local.ims:8888/?test=5",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    let tm = 0;
                    for (let v in r) {
                        tm = parseInt(v) * 1000;
                        self.charts["optmp"].data.datasets[0].data.push({
                            x: tm,
                            y: r[v].st,
                        });
                        self.charts["optmp"].data.datasets[1].data.push({
                            x: tm,
                            y: r[v].bt,
                        });
                        self.charts["opram"].data.datasets[0].data.push({
                            x: tm,
                            y: r[v].r,
                        });
                        self.charts["opram"].data.datasets[1].data.push({
                            x: tm,
                            y: r[v].r3,
                        });
                        self.charts["optmp"].update("quiet");
                        self.charts["opram"].update("quiet");
                    }
                } else console.log("no data for ops");
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // opchChartDataReload
    // Power status records
    pwstChartDataReload: function () {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.dataChartUrl,
            // url: "http://local.ims:8888/?test=8",
            dataType: "text",
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    this.pwstChartData = r.split("\n");
                    this.pwstChartRedraw();
                } else {
                    // ohSnap($.t("js.chartHasNoData"), this.info);
                    console.log("no data for pwst");
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // pwstChartDataReload
    pwstChartRedraw: function () {
        if (this.pwstChartData.length != 0) {
            this.charts["pwrmx"].resetZoom();
            let dayNumber = (dt) => {
                let d = new Date(dt);
                return ((d.getDay() + 6) % 7) + 1;
            };
            let radiusViaSec = (sec) => {
                if (sec <= 1000) {
                    return 4;
                } else if (sec > 1000 && sec <= 3000) {
                    return 6;
                } else if (sec > 3000 && sec <= 4000) {
                    return 8;
                } else if (sec > 4000 && sec <= 6000) {
                    return 11;
                } else if (sec > 6000) {
                    return 15;
                }
            };
            let p = [],
                prg = [];
            let ds = 0;
            let xoffset = 0;
            this.charts["pwrmx"].data.datasets[0].data = [];
            $.each(this.pwstChartData, (ind, ln) => {
                // skip last empty row
                if (ln.length != 0) {
                    p = ln.split(";");
                    p[0] = parseInt(p[0]) * 1000;
                    p[1] = parseInt(p[1]);
                    p[2] = parseInt(p[2]);
                    // 3 - on battery, 2 - on-line
                    if (p[2] === 0) {
                        prg.push([p[0], p[1]]);
                        if (p[1] === 3) {
                            ds = p[0];
                        } else if (p[1] === 2 && ds !== 0) {
                            this.charts["pwrmx"].data.datasets[0].data.push({
                                x: ds,
                                y: dayNumber(ds),
                                r: radiusViaSec((p[0] - ds) / 1000),
                                l: (p[0] - ds) / 1000, // in seconds
                                prg: prg,
                            });
                            ds = 0;
                            prg = [];
                        }
                    } else if (p[2] === 1) {
                        this.charts["pwrmx"].data.datasets[1].data.push({
                            x: p[0],
                            y: dayNumber(p[0]),
                            r: 5,
                            e: batteryStatusToString(p[1], false),
                        });
                    }
                }
            });
            this.charts["pwrmx"].update();
        }
    }, // pwstChartRedraw
    // Temperature records
    tctChartDataReload: function () {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.tempChartUrl,
            // url: "http://local.ims:8888/?test=4",
            dataType: "text",
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    this.tctChartData = r.split("\n");
                    this.tctChartRedraw();
                } else {
                    // ohSnap($.t("js.chartHasNoData"), this.warn);
                    console.log("no data for tct");
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
    tctChartRedraw: function () {
        if (this.tctChartData.length != 0) {
            this.charts["trecs"].resetZoom();
            // start with a period
            let sd = $("#dc-period")
                .children("option")
                .filter(":selected")
                .val();
            // TODO: select parameter
            // let ps = $("#dc-param");
            let p = [];
            let dt = 0,
                ts = new Date(),
                te = new Date();
            ts.setHours(0, 0, 0, 0);
            te.setHours(23, 59, 59, 59);
            // period
            switch (sd) {
                default:
                case "0": // today
                    ts = ts.getTime();
                    te = te.getTime();
                    break;
                case "1": // yesterday
                    ts = ts.getTime() - 86400000;
                    te = te.getTime() - 86400000;
                    break;
                case "2": // last 7 days
                    ts = ts.getTime() - 86400000 * 6;
                    te = te.getTime();
                    break;
                case "3": // last month (30 days)
                    ts = ts.getTime() - 86400000 * 29;
                    te = te.getTime();
                    break;
                case "4": // all time
                    ts = te = 0;
                    break;
            }
            // reset all
            this.charts["trecs"].data.datasets[0].data = [];
            this.charts["trecs"].data.datasets[1].data = [];
            // display data
            $.each(this.tctChartData, (ind, ln) => {
                // skip last empty row
                if (ln.length != 0) {
                    p = ln.split(";");
                    dt = parseInt(p[0]) * 1000;
                    if ((ts == 0 && te == 0) || (ts <= dt && dt <= te)) {
                        this.charts["trecs"].data.datasets[0].data.push({
                            x: dt,
                            y: parseInt(p[1]),
                        });
                        this.charts["trecs"].data.datasets[1].data.push({
                            x: dt,
                            y: parseInt(p[2]),
                        });
                        // i++;
                    }
                }
            });
            this.charts["trecs"].update("quiet");
        }
    }, // tctChartRedraw
    setRefreshLogs: function (s) {
        const self = this;
        if (s) {
            this.intls["logarf"] = setInterval(function () {
                self.logsys.reload();
                self.logsnmp.reload();
            }, self.refreshLogsIntl);
        } else {
            clearInterval(this.intls.logarf);
        }
    },
    initSidebar: function () {
        // TODO: use data-dropdown-toggle
        $("button[type=menu]").on("click", function (e) {
            let m = $("menu.sidebar");
            if (m.hasClass("hidden")) {
                m.removeClass("hidden");
            } else {
                m.addClass("hidden");
            }
        });
        // SIDEBAR MENU
        $("menu.sidebar li.item").on("click", (event) => {
            // e.preventDefault();
            let tab = $(event.currentTarget).find("a").attr("href");
            $("div.pagecontainer").each(function (i) {
                $(this).removeClass("grid");
                $(this).addClass("hidden");
            });
            $(tab).removeClass("hidden");
            $(tab).addClass("grid");
            // main menu
            $("menu.sidebar li.item").each(function (i) {
                $(this).removeClass("active");
            });
            $(event.currentTarget).addClass("active");
            // set location.hash
            location.hash = tab;
        });
        // activate menu item by location hash
        $("menu.sidebar li.item").each(function (i) {
            if (location.hash === $(this).find("a").attr("href")) {
                $(this).addClass("active");
            }
        });
    }, // initSidebar
    doReset: function (el, countdownEl) {
        $(el).attr("disabled", "disabled");
        $(el).addClass("disabled");
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.resetUrl,
            // url: "http://local.ims:8888/?test=35",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.length == 0 || r.done !== true || r.err !== undefined) {
                    ohSnap(
                        $.t("index.js.errErasingConfigErr") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                } else {
                    ohSnap($.t("index.js.infoErasingConfig"), this.info);
                    let countdown = this.resetCountdownIntl / 1000;
                    $(countdownEl).html(countdown);
                    setInterval(() => {
                        countdown -= 1;
                        if (countdown >= 0) {
                            $(countdownEl).html(countdown);
                            if (countdown == 0) {
                                window.location.reload();
                            }
                        }
                    }, 1000);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // doReset
    /**
     * Parse dashboard data
     */
    parseDashboardData: function () {
        let sc = $(".syscard");
        let bt = $(".battcard");
        let nw = $(".netwcard");
        let battstate = batteryStatusToString(this.dashboardData.battst);
        let uptime = secondsToHRts(this.dashboardData.uptm);
        let lchd = new Date(this.dashboardData.batchd);
        let outputstatus = outputStatusToSting(this.dashboardData.outst);
        let battdiast = batteryDiagStatusToString(this.dashboardData.battdiast);
        let battlifetime = secondsToHRts(this.dashboardData.ltime);
        // card: system
        sc.find(".hdr").html(
            $.t("index.js.titleLoad") + ": " + this.dashboardData.outload + "%"
        );
        sc.find(".systmp").html(this.dashboardData.systmp + "&#8451;");
        sc.find(".ram").html(
            this.dashboardData.ram + " / " + this.dashboardData.ram3
        );
        sc.find(".uptm").html(uptime);
        sc.find(".battst").html(battstate);
        sc.find(".outst").html(outputstatus);
        // adding text decoration
        if (this.dashboardData.outst != 2)
            sc.find(".outst").addClass("text-blink");
        else sc.find(".outst").removeClass("text-blink");
        // card: battery
        bt.find(".hdr").html(
            $.t("index.js.titleCharge") +
                ": " +
                this.dashboardData.battcap +
                "%"
        );
        bt.find(".batchd").html(lchd.toLocaleDateString());
        if (this.dashboardData.isclng) {
            $("#fan748").attr(
                "class",
                "fill-[#b7ccf9] rotate-center mr-2 mt-1"
            );
        } else {
            $("#fan748").attr("class", "hidden");
        }
        bt.find(".battmp").html(this.dashboardData.battmp + "&#8451;");
        bt.find(".involt").html(
            this.dashboardData.involt +
                "V / " +
                this.dashboardData.infreq +
                "Hz"
        );
        bt.find(".outvolt").html(
            this.dashboardData.outvolt +
                "V / " +
                this.dashboardData.outfreq +
                "Hz"
        );
        bt.find(".battdiast").html(battdiast);
        bt.find(".ltime").html(battlifetime);
        // CARD: NETWORK
        nw.find(".hdr").html("IP: " + this.dashboardData.ip);
        nw.find(".ap").html(this.dashboardData.ap);
        nw.find(".sm").html(this.dashboardData.sm);
        nw.find(".gw").html(this.dashboardData.gw);
        nw.find(".mac").html(this.dashboardData.mac);
        nw.find(".ctime").html(this.dashboardData.ctime);
    }, // parseDashboardData
    getDashData: function () {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.getDashDataUrl,
            // url: "http://local.ims:8888/?test=3",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.err !== undefined || r.length == 0) {
                    ohSnap(
                        $.t("index.js.errNoDataDashbrd") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                } else {
                    this.dashboardData = r;
                    this.parseDashboardData();
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // getDashData
    // @remind getCfg
    getCfg: function () {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.getCfgUrl,
            // url: "http://local.ims:8888/?test=7",
            dataType: "json",
            type: 'POST',
            success: (r) => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap(
                        $.t("index.js.errNoConfigData") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                } else {
                    $("input[name=battmplt]").val(r.battmplt);
                    $("input[name=battmput]").val(r.battmput);
                    $("input[name=devtmplt]").val(r.devtmplt);
                    $("input[name=devtmput]").val(r.devtmput);
                    $("input[name=ntpsrv]").val(r.ntpsrv);
                    $("input[name=ntpsrvfb]").val(r.ntpsrvfb);
                    $("input[name=ntpsrvsitl]").val(r.ntpsrvsitl);
                    // TIMEZONEs
                    let list = $("select[name=ntptmoff]");
                    list.find("option").each(function (i) {
                        if (parseInt($(this)[0].value) === r.ntptmoff) {
                            // console.log("TZ exists: " + $(this)[0].index);
                            // $(el).attr("selected", true);
                            list[0].selectedIndex = $(this)[0].index;
                        }
                    });
                    // SSIDs
                    list = $("select[name=ssid]");
                    const opt = list.find("option[value=" + r.ssid + "]");
                    if (opt.length != 0) {
                        // console.log("AP exists: " + opt[0].index);
                        list[0].selectedIndex = opt[0].index;
                    } else {
                        // console.log("AP n/a: " + list.length);
                        let node = document.createElement("option");
                        node.value = r.ssid;
                        node.innerHTML = r.ssid;
                        list.append(node);
                        list[0].selectedIndex = list.length - 1;
                    }
                    $("input[name=ssidkey]").val(r.ssidkey);
                    $("input[name=configsys]").removeAttr("disabled");
                    // SNMP
                    $("input[name=snmpport]").val(r.snmpport);
                    $("input[name=snmptrapport]").val(r.snmptraport);
                    $("input[name=snmploctn]").val(r.snmploctn);
                    $("input[name=snmpcontct]").val(r.snmpcontct);
                    $("input[name=snmpbatrpldt]").val(r.snmpbatrpldt);
                    $("input[name=configsnmp]").removeAttr("disabled");
                    // SECURITY
                    $("input[name=authtmout]").val(r.authtmout);
                    $("input[name=snmpgckey]").val(r.snmpgckey);
                    $("input[name=snmpsckey]").val(r.snmpsckey);
                    $("input[name=adlogin]").val(r.adlogin);
                    $("input[name=adpass]").val(r.adpass);
                    $("input[name=configsec]").removeAttr("disabled");
                    // prevent submit button changes
                    // $('#conf').find('input[type="submit"]').each(function(i) {
                    //     $(this).attr('disabled', 'disabled');
                    // });
                    // @remind get config
                    // empty the table body
                    $("#api-keys tbody").html("");
                    this.appendAPIKeys(r.api);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
    setSysCfg: function () {
        let form = $("form[name=confsys]");
        // let formdata = new FormData(form[0]);
        let formdata = form.serializeArray();
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.sysSetCfgUrl,
            dataType: "json",
            type: 'POST',
            data: formdata,
            success: (r) => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap(
                        $.t("js.errEmptyResponse") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                } else {
                    // success
                    ohSnap($.t("index.js.doneConfigUpdated"), this.info);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
    setSNMPCfg: function () {
        let form = $("form[name=confsnmp]");
        // let formdata = new FormData(form[0]);
        let formdata = form.serializeArray();
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.snmpSetCfgUrl,
            dataType: "json",
            type: 'POST',
            data: formdata,
            success: (r) => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap(
                        $.t("js.errEmptyResponse") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                } else {
                    // success
                    ohSnap($.t("index.js.doneConfigUpdated"), this.info);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
    setSecCfg: function () {
        let form = $("form[name=confsec]");
        // let formdata = new FormData(form[0]);
        let formdata = form.serializeArray();
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.secSetCfgUrl,
            dataType: "json",
            type: 'POST',
            data: formdata,
            success: (r) => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap(
                        $.t("js.errEmptyResponse") +
                            (r.err !== undefined ? " (" + r.err + ")" : ""),
                        this.err
                    );
                }
                // TODO
                else ohSnap($.t("index.js.doneConfigUpdated"), this.info);
            },
            error: (o, ts, e) => {
                errorRequest(o);
            },
        });
    },
});

/**
 * Log area objects
 * @author sk
 *
 * @class logArea
 * @typedef {logArea}
 */
class logArea {
    constructor(o) {
        this.obj = o;
        this.url = $(o).attr("aria-url");
        this.name = $(o).attr("aria-name");
    }
    reload() {
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.url,
            // url: "http://local.ims:8888/?test=6",
            dataType: "text",
            type: 'POST',
            success: (r) => {
                if (r.length != 0) {
                    let a = $(this.obj).find(".logdata");
                    a.html(r);
                    containerScrollBottom($(a)[0]);
                } else console.log("empty " + this.name + " log");
            },
            error: (o, ts, e) => {
                tinyUPS.handleErrorResponse(o, ts, e);
            },
        });
    }
}

// Globalize
window.scrollBottom = containerScrollBottom;
window.logArea = logArea;

$(function () {
    tinyUPS.init();
});
