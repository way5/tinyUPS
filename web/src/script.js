import "./chartist.scss";
import "./style.scss";
import 'flowbite';
// import dateRangePicker from 'flowbite-datepicker/DateRangePicker';
// See: http://wavded.github.io/humane-js/
const humane = require("./humane.js");
// See: https://gionkunz.github.io/chartist-js
import { LineChart } from 'chartist';
// See: http://smoothiecharts.org/tutorial.html
import { SmoothieChart, TimeSeries } from "smoothie";
// See: https://bitstorm.org/js/miq/documentation.html
var $ = require("./miq/miq");

/**
 * xmlHttp error handler for ajaxCallback
 */
var handleErrorResponse = (o) => {
    if (o == 403) {
        location.reload();
    }
}

/**
 * Do element scroll to bottom
 * @param {*} o
 */
var containerScrollBottom = (o) => {
    o.scroll({ top: o.scrollHeight, behavior: "smooth" });
}

/**
 * Main object
 * @todo: \
 *      1. js preloader \
 *      2. dark, light mode identification \
 *      3. config autosave \
 */
var tinyUPS = {
    surveyUrl: "/ajax-survey",
    setupUrl: "/ajax-setup",
    setupPageDataUrl: "/ajax-setup-init",
    loginUrl: "/ajax-login",
    getCfgUrl: "/ajax-get-config",
    sysSetCfgUrl: "/ajax-set-configsys",
    snmpSetCfgUrl: "/ajax-set-configsnmp",
    secSetCfgUrl: "/ajax-set-configsec",
    getDashDataURL: "/ajax-get-dashbrd",
    tempChartUrl: "/ajax-montmpr",
    infoGraphUrl: "/ajax-infograph",
    rebootURL: "/ajax-reboot",
    resetURL: "/ajax-reset",
    toggleCooling: "/ajax-coolingctrl",
    sysLog: null,
    snmpLog: null,
    err: null,
    info: null,
    done: null,
    dashboardData: {},
    // local variables
    scanResult: null,
    // TODO
    configData: null,
    showNotifyIntl: 7000,
    refreshLogsIntl: 15000,
    refreshDashIntl: 13000,
    refreshtempChartIntl: 10000,
    rebootCountdownIntl: 10000,
    resetCountdownIntl: 12000,
    // charts
    charts: {},
    intls: {},
    /**
     * Doc-scope initializer. Called manually
     */
    initCommon: function () {
        var self = this;
        this.displayMode();
        $('#dmode').on('click', function (e) {
            self.displayMode(true);
        });
        let cont = document.querySelector("header");
        this.info = humane.create({
            addnCls: "info",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: self.showNotifyIntl,
            // timeout: self.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        this.err = humane.create({
            addnCls: "error",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: self.showNotifyIntl,
            // timeout: self.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        this.done = humane.create({
            addnCls: "success",
            clickToClose: true,
            waitForMove: true,
            timeoutAfterMove: self.showNotifyIntl,
            // timeout: self.showNotifyIntl,
            container: cont,
            baseCls: "humane"
        }).spawn({});
        // PASSWORD UNHIDE
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
        // SURVEY
        $("#dosurvey").on("click", function (e) {
            e.preventDefault();
            tinyUPS.getSurvey();
        });
    },
    /**
     * 
     * @param {*} toggle 
     */
    displayMode: function (toggle = false) {
        if (localStorage.theme === 'dark' || (!('theme' in localStorage) && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
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
    /**
     * Setup page
     */
    initSetup: function () {
        let self = this;
        let resetBtn = $("button.reset-solo");
        // reset button
        $(resetBtn).on("click", (e) => {
            // let countdown = 7;
            self.doReboot(resetBtn, resetBtn);
            $(resetBtn).attr('disabled', 'disabled');
            // setInterval(() => {
            //     countdown -= 1;
            //     $(resetBtn).html(countdown);
            //     if (countdown == 0) {
            //         location.reload();
            //     }
            // }, 1000);
        });
        // only for POSTS
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.setupPageDataUrl,
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            // returned as an object, so the painful version of lenght
            var length = self.count(r);
            // create node
            if (length != 0) {
                $(".ap-mac-address").html("MAC: " + r.mac);
            } else {
                self.err("Confusing data, check page initializer...");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    /**
     * Admin panel initializer
     */
    initHome: function () {
        // APPLY PAGE CONTENTS
        let self = this;
        let lhash = location.hash;
        // let formCfgSys = $("form[name=confsys]").find("input, select");
        // let formCfgSNMP = $("form[name=confsnmp]").find("input, select");
        // let formCfgSec = $("form[name=confsec]").find("input, select");
        if (lhash != "") {
            // make corresponding objects active
            $(".sidebar li.item").forEach(function (el) {
                if ($(el).attr("href") == lhash)
                    $(el).addClass("active");
                else
                    $(el).removeClass("active");
            });
            // page contents
            let pid;
            $("div.pagecontainer").forEach(function (el) {
                pid = $(el).attr("id");
                if (pid == lhash.slice(1, lhash.length)) {
                    $(el).removeClass("hidden");
                    $(el).addClass("grid");
                } else {
                    $(el).removeClass("grid");
                    $(el).addClass("hidden");
                }
            });
        }
        // SERVICE MENU
        $(".cooler-switch-onoff").on("click", (e) => {
            $.ajax(
                window.location.protocol + "//" + window.location.hostname + this.toggleCooling,
                // "http://" + local.ims:8888 + "/?test=33",
                {
                    dataType: "json",
                    method: "POST"
                }
            ).then((r) => {
                if (r.length != 0) {
                    if (r.isclng === true) {
                        self.info("Cooler is ON");
                    } else if (r.isclng === false) {
                        self.info("Cooler is OFF");
                    } else {
                        console.log('wrong answer: ', r);
                    }
                } else
                    self.err("Failed to switch the cooler ON");
            }).catch((o) => {
                handleErrorResponse(o);
            });
        });
        // SIDEBAR
        this.initSidebar();
        // HIDE MENUS
        $("menu").on("mouseleave", function () {
            $(this).addClass("hidden");
        });
        // TABS
        $(".tabselector").forEach(function (ts) {
            $(ts).find("a.tab").forEach(function (el) {
                $(el).on("click", function (e) {
                    e.preventDefault();
                    let parent = ts;
                    let tgt = $(this).attr("href");
                    // tabs
                    $(this).parent().find("a.tab").forEach(function (t) {
                        $(t).removeClass("active");
                    });
                    $(el).addClass("active");
                    // contents
                    $(parent).find(".tab-conts").forEach(function (tc) {
                        if ($(tc).prop("id") == tgt)
                            $(tc).removeClass("hidden");
                        else
                            $(tc).addClass("hidden");
                    });
                });
            });
        });
        // FLATNAV
        $(".flatnav").forEach((o) => {
            $(o).find("a").forEach(function (el) {
                $(el).on("click", function (e) {
                    e.preventDefault();
                    let id = $(el).attr("href");
                    // hide all
                    $(o).find("a").forEach(function (t) {
                        let id = $(t).attr("href");
                        $(t).removeClass("active");
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
        // LOGS
        this.sysLog = new logArea($("#syslog"));
        this.sysLog.reload();
        this.snmpLog = new logArea($("#snmplog"));
        this.snmpLog.reload();
        // HASHCHANGE
        window.onhashchange = function () {
            switch (location.hash) {
                case "#home":
                    self.getDashData();
                    self.sysLog.reload();
                    self.snmpLog.reload();
                    break;
                case "#conf":
                    self.getSurvey();
                    self.getCfg();
                    break;
            }
        }
        if (location.hash == "#home" || location.hash == "") self.getDashData();
        window.dispatchEvent(new Event("hashchange"));
        // SUBMIT CONFIG
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
        // DO REFRESH LOGS
        if (localStorage.getItem("logarf") === "true") {
            self.setRefreshLogs(localStorage.getItem("logarf"));
            $("#logafertg").attr("checked", true);
        } else {
            $("#logafertg").removeAttr("checked");
        }
        $("#logafertg").on("change", function () {
            let v = $(this).val();
            self.setRefreshLogs(v);
            localStorage.setItem("logarf", v);
        });
        // if(localStorage.getItem('logarf') === 'true') {
        //     $("#logafertg").attr('checked', true);
        // }
        // HIDE LOADER
        setTimeout(function () {
            $("#loader").addClass("hidden");
        }, 1000);
        // UPDATE DASHBOARD EVERY refreshDashIntl
        self.intls['dd'] = setInterval(function () {
            self.getDashData();
        }, self.refreshDashIntl);
        // REBOOT BUTTON
        $("#modal-rbt-alert button.submit").on("click", function (e) {
            let el = e.target || e.srcElement;
            self.doReboot(el, $("#cntr831"));
        });
        // RESET BUTTON
        $("#modal-rst-alert button.submit").on("click", function (e) {
            let el = e.target || e.srcElement;
            self.doReset(el, $("#cntr833"));
        });
        // INIT CHARTS
        this.initCharts();
        // // config form events
        // $(formCfgSys).forEach((el) => {
        //     $(el).on('change', (e) => {
        //         let form = e.target || e.srcElement;
        //         $(form).find('input[type="submit"]').removeAttr('disabled');
        //     });
        // });
        // $(formCfgSNMP).forEach((el) => {
        //     $(el).on('change', (e) => {
        //         let form = e.target || e.srcElement;
        //         $(form).find('input[type="submit"]').removeAttr('disabled');
        //     });
        // });
        // $(formCfgSec).forEach((el) => {
        //     $(el).on('change', (e) => {
        //         let form = e.target || e.srcElement;
        //         $(form).find('input[type="submit"]').removeAttr('disabled');
        //     });
        // });
    },      // initHome
    /**
     * Dashboard charts
     */
    initCharts: function () {
        let self = this;
        // SMOOTHIE rt chart
        this.charts['optmp'] = new SmoothieChart({
            responsive: true,
            millisPerPixel: 2000,
            // interpolation:'bezier',
            grid: {
                fillStyle: 'transparent',
                lineWidth: 1,
                strokeStyle: (localStorage.theme === 'dark' ? 'rgba(121, 102, 188, 1.0)' : 'rgb(229, 231, 235)'),
                sharpLines: true,
                millisPerLine: 60000,
                verticalSections: 5,
                borderVisible: true
            },
            labels: {
                fillStyle: (localStorage.theme === 'dark' ? 'rgba(255, 255, 255, 1)' : 'rgba(52, 41, 85, 1)'),
                fontFamily: 'exo',
                // showIntermediateLabels:true,
                // intermediateLabelSameAxis:true,
            },
            // title: {
            // text:'',
            // fillStyle:'#000',
            // },
            // see stylesheet to customize
            tooltip: true,
            tooltipLine: {
                lineWidth: 1,
                strokeStyle: (localStorage.theme === 'dark' ? 'rgba(255, 147, 75, 0.8)' : 'rgba(255, 172, 29, 0.8)')
            },
            limitFPS: 15,
            // timestampFormatter: SmoothieChart.timeFormatter,
            timestampFormatter: (t) => {
                let ts = new Date(t);
                return ts.getHours() + ":" + ts.getMinutes();
            },
            maxValue: 80,
            minValue: 0
        });
        let canvasTmp = document.getElementById('op-chart-tmp');
        this.charts['opss1'] = new TimeSeries();    // sys
        this.charts['opss2'] = new TimeSeries();    // batt
        // temperature
        this.charts['optmp'].addTimeSeries(this.charts['opss1'], {
            lineWidth: 2,
            strokeStyle: 'rgba(245, 109, 4, 1)',
            fillStyle: 'rgba(245, 108, 4, 0.4)'
        });
        this.charts['optmp'].addTimeSeries(this.charts['opss2'], {
            lineWidth: 3,
            strokeStyle: 'rgba(225, 3, 40, 1)',
            fillStyle: 'rgba(225, 3, 40, 0.4)'
        });

        this.charts['opram'] = new SmoothieChart({
            responsive: true,
            millisPerPixel: 2000,
            // interpolation:'bezier',
            grid: {
                fillStyle: 'transparent',
                lineWidth: 1,
                strokeStyle: (localStorage.theme === 'dark' ? 'rgba(121, 102, 188, 1.0)' : 'rgb(229, 231, 235)'),
                sharpLines: true,
                millisPerLine: 60000,
                verticalSections: 5,
                borderVisible: true
            },
            labels: {
                fillStyle: (localStorage.theme === 'dark' ? 'rgba(255, 255, 255, 1)' : 'rgba(52, 41, 85, 1)'),
                fontFamily: 'exo',
                // showIntermediateLabels:true,
                // intermediateLabelSameAxis:true,
            },
            // title: {
            //     text:'RAM usage',
            //     fillStyle:'#000',
            // },
            // see stylesheet to customize
            tooltip: true,
            tooltipLine: {
                lineWidth: 1,
                strokeStyle: (localStorage.theme === 'dark' ? 'rgba(132, 255, 75, 0.8)' : 'rgba(110, 204, 28, 0.8)')
            },
            limitFPS: 15,
            // timestampFormatter: SmoothieChart.timeFormatter,
            timestampFormatter: (t) => {
                let ts = new Date(t);
                return ts.getHours() + ":" + ts.getMinutes();
            },
            maxValue: 160,
            minValue: 0
        });
        let canvasRam = document.getElementById('op-chart-ram');
        this.charts['opss3'] = new TimeSeries();    // free total
        this.charts['opss4'] = new TimeSeries();    // max block
        // ram
        this.charts['opram'].addTimeSeries(this.charts['opss3'], {
            lineWidth: 3,
            strokeStyle: 'rgba(56, 201, 73, 1)',
            fillStyle: 'rgba(74, 232, 93, 0.4)'
        });
        this.charts['opram'].addTimeSeries(this.charts['opss4'], {
            lineWidth: 2,
            strokeStyle: 'rgba(39, 127, 26, 1)',
            fillStyle: 'rgba(48, 129, 35, 0.6)'
        });

        this.charts['optmp'].streamTo(canvasTmp);
        this.charts['opram'].streamTo(canvasRam);
        // smoothie rt chart
        this.intls['op'] = setInterval(() => this.opchPoll(), this.refreshtempChartIntl);
        this.opchPoll();

        // CHARTIST
        this.charts['tct'] = new LineChart('.tct-chart', {
            labels: [],                                                    // x
            series: [{ name: 'system-temp', data: [10, 20, 30, 40, 50] }]        // y (dummy data)
        }, {
            fullWidth: true,
            showPoint: true,
            lineSmooth: true,
            axisX: {
                showGrid: false,
                showLabel: true,
                // labelInterpolationFnc: function(value) {
                //     return value;
                // }
            },
            axisY: {
                showLabel: true,
                showGrid: true,
                offset: 30
            },
            series: {
                'system-temp': {
                }
            }
        });
        // filter change
        $("#dc-period").on("change", function () {
            self.tctPoll();
        });
        // update now
        this.tctPoll();
    },
    // SMOOTHIE
    opchPoll: function () {
        let self = this;

        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.infoGraphUrl,
            // "http://" + local.ims:8888 + "/?test=5",
            {
                dataType: "json",
                method: "POST"
            }
        ).then((r) => {
            if (r.length != 0) {
                for (let v in r) {
                    self.charts['opss1'].append((parseInt(v) * 1000), r[v].st);
                    self.charts['opss2'].append((parseInt(v) * 1000), r[v].bt);
                    self.charts['opss3'].append((parseInt(v) * 1000), r[v].r);
                    self.charts['opss4'].append((parseInt(v) * 1000), r[v].r3);
                }
            } else {
                console.log("(!) opchpoll failed");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    // CHARTIST
    tctPoll: function () {
        let self = this;
        // start with a period
        let sd = $("#dc-period").val();
        // TODO: select parameter
        // let ps = $("#dc-param");
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.tempChartUrl,
            // "http://" + local.ims:8888 + "/?test=4",
            {
                dataType: "plain",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                $("#dc-nodata").addClass("hidden");
                let dl = r.split("\n");
                let p = [],
                    x = [];
                let y1 = { name: 'system-temp', data: [] };
                let i = 0,
                    dt = 0,
                    ts = new Date(),
                    te = new Date();
                // period
                switch (sd) {
                    default:
                    case 0: // today
                        ts.setHours(0, 0, 0, 0);
                        te.setHours(23, 59, 59, 59);
                        ts = ts.getTime();
                        te = te.getTime();
                        break;
                    case 1: // yesterday
                        ts.setHours(0, 0, 0, 0);
                        te.setHours(23, 59, 59, 59);
                        ts = ts.getTime() - 86400000;
                        te = te.getTime() - 86400000;
                        break;
                    case 2: // last 7 days
                        ts.setHours(0, 0, 0, 0);
                        te.setHours(23, 59, 59, 59);
                        ts = ts.getTime() - (86400000 * 6);
                        te = te.getTime();
                        break;
                    case 3: // last month (30 days)
                        ts.setHours(0, 0, 0, 0);
                        te.setHours(23, 59, 59, 59);
                        ts = ts.getTime() - (86400000 * 29);
                        te = te.getTime();
                        break;
                    case 4: // all time
                        ts = te = 0;
                        break;
                }
                console.log("from: " + ts + " to: " + te);
                // display data
                dl.forEach((ln) => {
                    // skip last empty row
                    if (ln.length != 0) {
                        p = ln.split(";");
                        dt = new Date(parseInt(p[0]) * 1000);
                        if ((ts == 0 && te == 0) || (ts <= dt && dt <= te)) {
                            x[i] = dt.getHours() + ':' + ('00' + dt.getMinutes()).slice(-2) + ', '
                                + ('00' + dt.getDate()).slice(-2) + '/' + ('00' + (dt.getMonth() + 1)).slice(-2);
                            y1.data[i] = parseInt(p[1]);
                            i++;
                        }
                    }
                });
                // update chart
                self.charts['tct'].update({
                    labels: x,
                    series: [y1]
                });
            } else {
                console.log("(i) no data for data chart");
                $("#dc-nodata").removeClass("hidden");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    /**
     * For the sake of comparibility
     */
    count: (r) => {
        var l = 0;
        for (var k in r) {
            if (r.hasOwnProperty(k)) {
                l++;
            }
        }
        return l;
    },
    setRefreshLogs: function (s) {
        let self = this;
        if (s) {
            self.intls['logarf'] = setInterval(function () {
                self.sysLog.reload();
                self.snmpLog.reload();
            }, self.refreshLogsIntl);
        } else {
            clearInterval(self.intls.logarf);
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
        $(".sidebar li.item").on("click", function (e) {
            // e.preventDefault();
            let tgt = $(this).find("a").attr("href");
            // page contents
            $("div.pagecontainer").forEach(function (el) {
                $(el).removeClass("grid");
                $(el).addClass("hidden");
            });
            $(tgt).removeClass("hidden");
            $(tgt).addClass("grid");
            // main menu
            $(".sidebar li.item").forEach(function (el) {
                $(el).removeClass("active");
            });
            $(this).addClass("active");
            // SET LOCATION.HASH
            location.hash = tgt;
            $(this).find("a").on("click", function (e) {
                e.preventDefault();
                return false;
            });
            $("menu").addClass("hidden");
        });
        // ACTIVATE MENU ITEM BY LOCATION HASH
        $(".sidebar li.item").forEach(function (el) {
            if (location.hash == $(el).find("a").attr("href")) {
                $(el).addClass("active");
            }
        });
    },
    /**
     * Survey
     */
    getSurvey: function () {
        let self = this;
        let list = $('select[name="ssid"]');
        // TODO: dosurvey button decoration
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.surveyUrl,
            // "http://" + local.ims:8888 + "/?test=1",
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            // returned as an object, so the painful version of lenght
            var length = self.count(r);
            // create node
            if (r.delay !== undefined) {
                let delay = Integer.parseInt(r.delay);
                console.log("delay detected: ", delay);
                self.info((delay > 0 ? "Repeating scan in" + (r / 1000.0) + "sec..." : "Scan in progress..."));
                setTimeout(() => {
                    self.getSurvey();
                }, delay);
            } else if (length != 0) {
                self.scanResult = r;
                list.html("");
                for (let i = 0; i < length; i++) {
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
                    /* RSSI: " + r[i].r + ", */
                    node.innerHTML = r[i].s + " (ENC: " + enc + " RSSI: " + r[i].r + ")";

                    // console.log(node);
                    list.append(node);
                }
            } else {
                self.err("No WiFi networks detected...");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },      // getSurvey
    doReboot: function (el, countdownEl) {
        let self = this;
        $(el).attr("disabled", "disabled");
        $(el).addClass("disabled");
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.rebootURL,
            // "http://" + local.ims:8888 + "/?test=34",
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.done === true) {
                    self.info("Device is rebooting");
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
                self.err("Something's wrong when rebooting");
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },      // doReboot
    doReset: function (el, countdownEl) {
        let self = this;
        $(el).attr("disabled", "disabled");
        $(el).addClass("disabled");
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.resetURL,
            // "http://" + local.ims:8888 + "/?test=35",
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.done === true) {
                    self.info("Configuration's being erased");
                    let countdown = (self.resetCountdownIntl / 1000);
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
                self.err("Something's wrong when erasing config");
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },      // doReset
    /**
     * Send setup form
     */
    doSetup: function () {
        let self = this;
        let form = $("form[name=setup]");
        let modalDone = new Modal(document.getElementById('modal'));
        // let modalTest = new Modal(document.getElementById('modal-test'));
        // check form
        /*
        (?=.*[a-z]) must contain at least 1 lowercase alphabetical character
        (?=.*[A-Z]) must contain at least 1 uppercase alphabetical character
        (?=.*[0-9]) must contain at least 1 numeric character
        (?=.*[!@#$%^&*])    must contain at least one special character
        (?=.{6,})   must be 6 characters or longer
        */
        let loginFormat = new RegExp("^([a-z0-9]+)(?=.{2,})");
        if (!loginFormat.test(form.find("[name=login]").val())) {
            self.info("Login must contain only lowercase characters and numbers, it must not be shorter than 2 characters");
        }
        let passformat = new RegExp("^(((?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])))(?=.{6,})");
        if (!passformat.test(form.find("[name=pass]").val())) {
            self.info("Password must contain lovercase, uppercase, numeric characters and must be at least 6 characters long");
            return;
        }
        // testing parameters
        // modalTest.show();
        // only for POSTS
        let formdata = new FormData(form[0]);
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.setupUrl,
            // "http://" + local.ims:8888 + "/?test=22",
            {
                dataType: "json",
                method: "POST",
                data: formdata,
            }
        ).then((r) => {
            if (r.setup === 'fail') {
                // modalTest.hide();
                switch (r.err) {
                    case 'sta':
                        self.info("Cannot connnect to WiFi, check access point credentials");
                        return;
                    case 'data':
                        self.info("Wrong parameters provided, check the payload");
                        return;
                    default:
                        break;
                }
            } else if (r.setup === 'done') {
                modalDone.show();
                return;
            }
            self.err("Unexpected server response. Please try again");
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    /**
     * Login
     */
    doLogin: function () {
        let self = this;
        let form = $("form[name=login]");
        let formdata = new FormData(form[0]);
        // Flowbite bug! Doesn't work like so
        // let modalAlert = new Modal(document.getElementById("modal-alert"), {
        //     backdrop: 'dynamic',
        //     closable: true
        // });
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.loginUrl,
            // "http://" + local.ims:8888 + "/?test=23",
            {
                dataType: "json",
                method: "POST",
                data: formdata,
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.login === "err") {
                    self.err(r.err);
                } else {
                    // auth results
                    if (r.login === "repeat") {
                        $("#modal-alert").removeClass("hidden");
                        // modalAlert.show();
                    } else {
                        console.log("logged in");
                        document.location.reload();
                    }
                }
            } else {
                self.err("Login failed");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    secondsToHRts: (t) => {
        let year,
            day,
            hour,
            min;
        let ts = "";
        if (t > 31556952) {
            year = Math.floor(t / 31556952);
            t = t % 31556952;
            ts = year + " year ";
        }
        if (t > 86400) {
            day = Math.floor(t / 86400);
            t = t % 86400;
            ts = ts + day + " day ";
        }
        if (t > 3600) {
            hour = Math.floor(t / 3600);
            t = t % 3600;
            ts = ts + hour + " hrs ";
        }
        if (t > 60) {
            min = Math.floor(t / 60);
            t = t % 60;
            ts = ts + min + " min ";
        }
        if (t > 0) {
            ts = ts + t + " sec";
        }
        return ts;
    },
    outputStatusToSting: (s) => {
        let st = 'on-line';
        switch (s) {
            case 2:
            default:
                // online
                break;
            case 3:
                // battery
                st = 'on battery';
                break;
        }
        return st;
    },
    batteryDiagStatusToString: (s) => {
        let t = 'normal';
        switch (s) {
            case 3:
            default:
                break;
            case 4:
                t = 'fault';
                break;
            case 7:
                t = 'capacity low';
                break;
        }
        return t;
    },
    batteryStatusToString: function (t) {
        let r = "normal";
        $('.indicators-wrapper').removeClass('battery-low-alert');
        $('.indicators-wrapper').removeClass('battery-faulty-alert');
        $('menu.sidebar li:first-child').removeClass('battery-low-nav-alert');
        $('menu.sidebar li:first-child').removeClass('battery-faulty-nav-alert');
        switch (t) {
            default:
            case 2:
                $("#led934").attr("class", "blink-led fill-[#8df478]");
                break;
            case 3:
                $("#led934").attr("class", "blink-led fill-[#ffda90]");
                // swith UI innto notification mode
                $('.indicators-wrapper').addClass('battery-low-alert');
                $('menu.sidebar li:first-child').addClass('battery-low-nav-alert');
                r = "low";
                break;
            case 4:
                $("#led934").attr("class", "blink-led fill-[#ff9898]");
                $('.indicators-wrapper').addClass('battery-faulty-alert');
                $('menu.sidebar li:first-child').addClass('battery-faulty-nav-alert');
                r = "faulty";
                break;
        }
        return r;
    },
    /**
     * Parse dashboard data
     */
    parseDashboardData: function () {
        let sc = $(".syscard");
        let bt = $(".battcard");
        let nw = $(".netwcard");
        let battstate = this.batteryStatusToString(this.dashboardData.battst);
        let uptime = this.secondsToHRts(this.dashboardData.uptm);
        let lchd = new Date(this.dashboardData.batchd);
        let outputstatus = this.outputStatusToSting(this.dashboardData.outst);
        let battdiast = this.batteryDiagStatusToString(this.dashboardData.battdiast);
        let battlifetime = this.secondsToHRts(this.dashboardData.ltime);
        // CARD: SYSTEM
        sc.find(".hdr").html("Load: " + this.dashboardData.outload + "%");
        sc.find(".systmp").html(this.dashboardData.systmp + "&#8451;");
        sc.find(".ram").html(this.dashboardData.ram + " / " + this.dashboardData.ram3);
        sc.find(".uptm").html(uptime);
        sc.find(".battst").html(battstate);
        sc.find(".outst").html(outputstatus);
        // CARD: BATTERY
        bt.find(".hdr").html("Charge: " + this.dashboardData.battcap + "%");
        bt.find(".batchd").html(lchd.toLocaleDateString());
        if (this.dashboardData.isclng) {
            $("#fan748").attr("class", "fill-[#b7ccf9] rotate-center mr-2 mt-1");
        } else {
            $("#fan748").attr("class", "hidden");
        }
        bt.find(".battmp").html(this.dashboardData.battmp + "&#8451;");
        bt.find(".involt").html(this.dashboardData.involt + "V / " + this.dashboardData.infreq + "Hz");
        bt.find(".outvolt").html(this.dashboardData.outvolt + "V / " + this.dashboardData.outfreq + "Hz");
        bt.find(".battdiast").html(battdiast);
        bt.find(".ltime").html(battlifetime);
        // CARD: NETWORK
        nw.find(".hdr").html("IP: " + this.dashboardData.ip);
        nw.find(".ap").html(this.dashboardData.ap);
        nw.find(".sm").html(this.dashboardData.sm);
        nw.find(".gw").html(this.dashboardData.gw);
        nw.find(".mac").html(this.dashboardData.mac);
        nw.find(".ctime").html(this.dashboardData.ctime);
    },
    /**
     * Query dashboard
     */
    getDashData: function () {
        let self = this;
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.getDashDataURL,
            // "http://" + local.ims:8888 + "/?test=3",
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.err !== undefined) {
                    self.err(r.err);
                } else {
                    self.dashboardData = r;
                    self.parseDashboardData();
                }
            } else {
                self.err("Dashboard has no data");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    getCfg: function () {
        let self = this;
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.getCfgUrl,
            // "http://" +local.ims:8888+ "/?test=7",
            {
                dataType: "json",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.err !== undefined) {
                    self.err(r.err);
                } else {
                    let list, opt;
                    $("input[name=battmplt]").val(r.battmplt);
                    $("input[name=battmput]").val(r.battmput);
                    $("input[name=devtmplt]").val(r.devtmplt);
                    $("input[name=devtmput]").val(r.devtmput);
                    $("input[name=ntpsrv]").val(r.ntpsrv);
                    $("input[name=ntpsrvfb]").val(r.ntpsrvfb);
                    $("input[name=ntpsrvsitl]").val(r.ntpsrvsitl);
                    // TIMEZONEs
                    list = $("select[name=ntptmoff]");
                    list.find("option").forEach(function (el) {
                        if (parseInt($(el)[0].value) == r.ntptmoff) {
                            console.log("TZ exists: " + $(el)[0].index);
                            // $(el).attr("selected", true);
                            list[0].selectedIndex = $(el)[0].index;
                        }
                    });
                    // SSIDs
                    list = $("select[name=ssid]");
                    opt = list.find("option[value=" + r.ssid + "]");
                    if (opt.length != 0) {
                        console.log("AP exists: " + opt[0].index);
                        list[0].selectedIndex = opt[0].index;
                    } else {
                        console.log("AP n/a: " + list.length);
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
                    // $('#conf').find('input[type="submit"]').forEach((el) => {
                    //     $(el).attr('disabled', 'disabled');
                    // });
                }
            } else {
                self.err("Cannot retrieve configuration data");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    setSysCfg: function () {
        let self = this;
        let form = $("form[name=confsys]");
        let formdata = new FormData(form[0]);
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.sysSetCfgUrl,
            {
                dataType: "json",
                method: "POST",
                data: formdata,
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.err !== undefined) {
                    self.err(r.err);
                } else {
                    // success
                    console.log(r);
                    self.done("Configuration updated");
                    // $(form).find('input[type="submit"]').attr('disabled', 'disabled');
                }
            } else {
                self.err("Empty reponse...");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    setSNMPCfg: function () {
        let self = this;
        let form = $("form[name=confsnmp]");
        let formdata = new FormData(form[0]);
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.snmpSetCfgUrl,
            {
                dataType: "json",
                method: "POST",
                data: formdata,
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.err !== undefined) {
                    self.err(r.err);
                } else {
                    // success
                    console.log(r);
                    self.done("Configuration updated");
                    // $(form).find('input[type="submit"]').attr('disabled', 'disabled');
                }
            } else {
                self.err("Empty reponse...");
            }
        }).catch((o) => {
            handleErrorResponse(o);
        });
    },
    setSecCfg: function () {
        let self = this;
        let form = $("form[name=confsec]");
        let formdata = new FormData(form[0]);
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + this.secSetCfgUrl,
            {
                dataType: "json",
                method: "POST",
                data: formdata,
            }
        ).then((r) => {
            if (r.length != 0) {
                if (r.err !== undefined) {
                    self.err(r.err);
                } else {
                    // success
                    console.log(r);
                    self.done("Configuration updated");
                    // $(form).find('input[type="submit"]').attr('disabled', 'disabled');
                }
            } else {
                self.err("Empty reponse...");
            }
        }).catch((o) => {
            errorRequest(o);
        });
    }
};

/**
* Displays logs
*/
class logArea {
    constructor(o) {
        this.obj = o;
        this.url = $(o).attr("aria-url");
        this.name = $(o).attr("aria-name");
    }
    reload() {
        let self = this;
        $.ajax(
            window.location.protocol + "//" + window.location.hostname + self.url,
            // "http://" + local.ims:8888 + "/?test=6",
            {
                dataType: "plain",
                method: "POST",
            }
        ).then((r) => {
            if (r.length != 0) {
                let a = $(self.obj).find(".logdata");
                a.html(r);
                containerScrollBottom($(a)[0]);
            } else
                console.log("Empty " + self.name + "...");
        }).catch((o) => {
            handleErrorResponse(o);
        });
    }
};

// Globalize
window['$'] = miq;
window['scrollBottom'] = containerScrollBottom;
window['humane'] = humane;
window['tinyUPS'] = tinyUPS;
window['logArea'] = logArea;

// export { tinyUPS, logArea, containerScrollBottom };