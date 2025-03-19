import './index.scss';
// Helpers
import { secondsToHRts, batteryDiagStatusToString, batteryStatusToString, daysInMonth } from './includes/helpers.js';
//
import { tinyUPS, ohSnap, ohSnapX, $restURL } from './common.js';
// See: https://www.npmjs.com/package/md5
import md5 from 'crypto-js/md5';
// Charts
import tempChart from './includes/chart.temp.js';
import ramChart from './includes/chart.ram.js';
import tempTimelineChart from './includes/chart.temptimeline.js';
import powerStateChart from './includes/chart.powerstate.js';
import logArea from './includes/log.area.js';
import { outputStatusToSting } from './includes/helpers.js';
// Components
import Tooltip from './includes/tooltip.js';
import Dropdown from './includes/dropdown.js';
import Modal from './includes/modal.js';

$.extend(tinyUPS, {
    logsys: null,
    logsnmp: null,
    dashboardData: {},
    // local variables
    scanResult: null,
    tctChartData: [],
    pwstChartData: [],
    configData: null,
    refreshLogsIntl: 30000,
    refreshDashIntl: 15000,
    refreshtempChartIntl: 10000,
    resetCountdown: 15000,
    updateCountdown: 10000,
    // charts
    charts: {},
    intls: {},
    /**
     * Initializer
     */
    initPage: function () {
        // APPLY PAGE CONTENTS
        const self = this;
        let lhash = window.location.hash;
        if (lhash !== '') {
            // make corresponding objects active
            $('.sidebar li.item > a').each(function (i) {
                if ($(this).attr('href') === lhash) {
                    $(this).addClass('active');
                } else {
                    $(this).removeClass('active');
                }
            });
            // page contents
            $('div.pagecontainer').each(function (i) {
                let pid = $(this).attr('id');
                if (pid === lhash.slice(1, lhash.length)) {
                    $(this).removeClass('hidden');
                    $(this).addClass('grid');
                } else {
                    $(this).removeClass('grid');
                    $(this).addClass('hidden');
                }
            });
        }
        // service menu
        $('.cooler-switch-onoff').on('click', e => {
            self.toggleCooling();
        });
        // sidebar
        this.initSidebar();
        // hide menus
        $('menu').on('mouseleave', function () {
            $(this).addClass('hidden');
        });
        // tabs
        $('.tabselector').each(function (i) {
            let ts = this;
            $(ts)
                .find('a.tab')
                .on('click', function (e) {
                    e.preventDefault();
                    let tgt = $(this).attr('href');
                    // tabs
                    $(this)
                        .parent()
                        .find('a.tab')
                        .each(function (i) {
                            $(this).removeClass('active');
                        });
                    $(this).addClass('active');
                    // contents
                    $(ts)
                        .find('.tab-conts')
                        .each(function (i) {
                            if ($(this).prop('id') === tgt) $(this).removeClass('hidden');
                            else $(this).addClass('hidden');
                        });
                });
        });
        // flatnav
        $('.flatnav').each(function (i) {
            let o = this;
            $(o)
                .find('a')
                .each(function (i) {
                    let el = this;
                    $(el).on('click', function (e) {
                        e.preventDefault();
                        let id = $(el).attr('href');
                        // hide all
                        $(o)
                            .find('a')
                            .each(function (i) {
                                let id = $(this).attr('href');
                                $(this).removeClass('active');
                                $(id).addClass('hidden');
                            });
                        // current tab
                        $(el).addClass('active');
                        $(id).removeClass('hidden');
                        // callback if exists
                        let cb = $(el).attr('data-click-callback');
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
        this.logsys = new logArea($('#logsys'));
        this.logsnmp = new logArea($('#logsnmp'));
        // hashchange
        $(window).on('hashchange', e => {
            switch (window.location.hash) {
                case '':
                case '#home':
                    this.getDashData();
                    this.logsys.reload();
                    this.logsnmp.reload();
                    break;
                case '#conf':
                    this.getSurvey();
                    this.getCfg();
                    break;
            }
        });
        // stop all intervals
        const killIntervals = () => {
            for (let i in this.intls) {
                clearInterval(this.intls[i]);
            }
        };
        window.dispatchEvent(new Event('hashchange'));
        // SSID selector
        $('select[name=ssid]').on('change', function (e) {
            const list = $(e.target);
            const ssidKeyInput = $('input[name="ssidkey"]');
            const defaultValue = $(e.target).data('default-value');
            if (list.val() !== defaultValue) {
                ssidKeyInput.val('');
                ssidKeyInput.trigger('focus');
            } else {
                ssidKeyInput.val(ssidKeyInput.data('default-value'));
            }
        });
        // submit config
        $('input[name=configsys]').on('click', function (e) {
            e.preventDefault();
            self.setSysCfg();
        });
        $('input[name=configsnmp]').on('click', function (e) {
            e.preventDefault();
            self.setSNMPCfg();
        });
        $('input[name=configsec]').on('click', function (e) {
            e.preventDefault();
            self.setSecCfg();
        });
        $('input[name=firmware]').on('change', function (e) {
            e.preventDefault();
            if (this.files[0].name !== this.accept) {
                ohSnap($.t('index.js.updateWrongFirmware'), self.warn);
                this.value = '';
                return false;
            }
            killIntervals();
            self.sendUpdate(this);
        });
        $('input[name=filesystem]').on('change', function (e) {
            e.preventDefault();
            if (this.files[0].name !== this.accept) {
                ohSnap($.t('index.js.updateWrongFilesystem'), self.warn);
                this.value = '';
                return false;
            }
            killIntervals();
            self.sendUpdate(this);
        });
        $('#genserial').on('click', function (e) {
            e.preventDefault();
            self.doGenerateSerial();
        });
        // do refresh logs
        $('#log-aref-tg').on('change', function () {
            let v = $(this).prop('checked');
            self.setRefreshLogs(v);
            window.settings.set('logarf', v);
        });
        let refreshLogsSettingValue = window.settings.get('logarf', false);
        if (refreshLogsSettingValue === true) {
            this.setRefreshLogs(refreshLogsSettingValue);
            $('#log-aref-tg').attr('checked', true);
        } else {
            $('#log-aref-tg').removeAttr('checked');
        }
        // update dashboard every refreshdashintl
        this.intls['dd'] = setInterval(function () {
            // do not query if not the dashbrd
            (window.location.hash === '#home' || window.location.hash === '') && self.getDashData();
        }, this.refreshDashIntl);
        // reboot button
        $('#modal-rbt-alert button.submit').on('click', function (e) {
            self.doReboot('#cntr831', this);
        });
        //reset button
        $('#modal-rst-alert button.submit').on('click', function (e) {
            self.doReset('#cntr833', this);
        });
        // init charts
        this.initCharts();
        // chart refresh buttons
        $('#pwst-chart-reload').on('click', function () {
            self.pwstChartDataReload();
            ohSnap($.t('js.chartReloaded'), self.info);
        });
        $('#trecs-chart-reload').on('click', function () {
            self.tctChartDataReload();
            ohSnap($.t('js.chartReloaded'), self.info);
        });
        // components
        Tooltip();
        Dropdown();
        Modal();
        // page loader
        setTimeout(() => {
            $('#loader').addClass('hidden');
        }, 1000);
    }, // initPage
    toggleCooling: function () {
        $.ajax({
            url: $restURL.toggleCoolingUrl,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.isclng === true) {
                    ohSnap($.t('index.js.coolerIsOn'), this.info);
                } else if (r.isclng === false) {
                    ohSnap($.t('index.js.coolerIsOff'), this.info);
                } else ohSnap($.t('index.js.failedCoolerOnOff'), this.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // toggleCooling
    initCharts: function () {
        // @remind temerature charts
        this.charts['optmp'] = new tempChart('op-chart-tmp');
        // @remind ram chart
        this.charts['opram'] = new ramChart('op-chart-ram');
        // rt chart
        this.intls['opch'] = setInterval(() => {
            (window.location.hash === '#home' || window.location.hash === '') && this.opchChartDataReload();
        }, this.refreshtempChartIntl);
        this.opchChartDataReload();
        // @remind power state chart
        this.charts['pwrmx'] = new powerStateChart('pwr-chart');
        document.addEventListener('themeChange', e => {
            this.charts['pwrmx'].update(e.detail.theme);
            this.pwstChartRedraw();
        });
        // @remind trecs chart
        this.charts['trecs'] = new tempTimelineChart('trecs-chart');
        // filter change
        $('#dc-period').on('change', () => {
            this.tctChartRedraw();
        });
        // update now
        this.pwstChartDataReload();
        this.tctChartDataReload();
    }, // initCharts
    // @remind poll data for opcharts
    opchChartDataReload: function () {
        const self = this;
        $.ajax({
            url: $restURL.infoGraphUrl,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.length != 0) {
                    let tm = 0;
                    for (let v in r) {
                        tm = parseInt(v) * 1000;
                        self.charts['optmp'].update(
                            {
                                x: tm,
                                y: r[v].st
                            },
                            {
                                x: tm,
                                y: r[v].bt
                            }
                        );
                        self.charts['opram'].update(
                            {
                                x: tm,
                                y: r[v].r
                            },
                            {
                                x: tm,
                                y: r[v].r3
                            }
                        );
                    }
                } else console.log('no data for ops');
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // opchChartDataReload
    // Power status records
    pwstChartDataReload: function () {
        $.ajax({
            url: $restURL.dataChartUrl,
            dataType: 'text',
            type: 'POST',
            success: r => {
                if (r.length != 0) {
                    this.pwstChartData = r.split('\n');
                    this.pwstChartRedraw();
                } else {
                    // ohSnap($.t("js.chartHasNoData"), this.info);
                    console.log('no data for pwst');
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // pwstChartDataReload
    pwstChartRedraw: function () {
        if (this.pwstChartData.length != 0) {
            this.charts['pwrmx'].reset();
            let p = [],
                prg = [];
            let dateStart = 0;
            let dataset = [];
            let dayEvent = [];
            let currentYear = new Date().getFullYear();
            // prepare dataset it must be filled in with empty results for each of 31 days per month
            for (let m = 0; m < 12; m++) {
                let month = [];
                let daysTotal = daysInMonth(currentYear, m);
                for (let d = 0; d < 31; d++) {
                    if (d < daysTotal) {
                        let dt = new Date(currentYear, m, d + 1);
                        month.push({
                            x: d,
                            y: 0,
                            t: dt.toLocaleString([], {
                                month: 'long',
                                day: 'numeric'
                            }),
                            e: []
                        });
                    }
                }
                dataset.push(month);
            }
            // adding data to the day of the year
            const addDatasetEvent = (ts, eventData) => {
                let dt = new Date(ts);
                let month = dt.getMonth(); // 0 - 11
                let day = dt.getDate() - 1; // 1 - 31 (-1)
                dataset[month][day].y++;
                dataset[month][day].e.push(eventData);
            };
            $.each(this.pwstChartData, (ind, ln) => {
                // skip last empty row
                if (ln.length != 0) {
                    p = ln.split(';');
                    // timestamp
                    p[0] = parseInt(p[0]) * 1000;
                    // event ID: 3 - on battery, 2 - on-line
                    p[1] = parseInt(p[1]);
                    // 0 - power status change, 1 - battery status change
                    p[2] = parseInt(p[2]);
                    //
                    if (p[2] === 0) {
                        // power status events
                        if (dateStart !== 0) {
                            // which occurred during power status changes
                            prg.push([p[0], p[1]]);
                        }
                        if (p[1] === 3) {
                            dateStart = p[0];
                        } else if (p[1] === 2 && dateStart !== 0) {
                            dayEvent.push({
                                x: dateStart,
                                d: (p[0] - dateStart) / 1000, // in seconds
                                prg: prg
                            });
                            addDatasetEvent(dateStart, dayEvent);
                            dateStart = 0;
                            prg = [];
                            dayEvent = [];
                        }
                    } else if (p[2] === 1 && dateStart !== 0) {
                        // battery events
                        dayEvent.push({
                            x: p[0],
                            e: batteryStatusToString(p[1], false)
                        });
                    }
                }
            });
            // updating the chart
            this.charts['pwrmx'].dataset(dataset);
        }
    }, // pwstChartRedraw
    // Temperature records
    tctChartDataReload: function () {
        $.ajax({
            url: $restURL.tempChartUrl,
            dataType: 'text',
            type: 'POST',
            success: r => {
                if (r.length != 0) {
                    this.tctChartData = r.split('\n');
                    this.tctChartRedraw();
                } else {
                    // ohSnap($.t("js.chartHasNoData"), this.warn);
                    console.log('no temperature log records yet');
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },
    tctChartRedraw: function () {
        if (this.tctChartData.length != 0) {
            this.charts['trecs'].reset();
            // start with a period
            let sd = $('#dc-period').children('option').filter(':selected').val();
            // TODO: select parameter
            let p = [];
            let dt = 0,
                ts = new Date(),
                te = new Date();
            ts.setHours(0, 0, 0, 0);
            te.setHours(23, 59, 59, 59);
            // period
            switch (sd) {
                default:
                case '0': // today
                    ts = ts.getTime();
                    te = te.getTime();
                    break;
                case '1': // yesterday
                    ts = ts.getTime() - 86400000;
                    te = te.getTime() - 86400000;
                    break;
                case '2': // last 7 days
                    ts = ts.getTime() - 86400000 * 6;
                    te = te.getTime();
                    break;
                case '3': // last month (30 days)
                    ts = ts.getTime() - 86400000 * 29;
                    te = te.getTime();
                    break;
                case '4': // all time
                    ts = te = 0;
                    break;
            }
            // display data
            let datasetsSt = [];
            let datasetsBt = [];
            $.each(this.tctChartData, (ind, ln) => {
                // skip last empty row
                if (ln.length != 0) {
                    p = ln.split(';');
                    dt = parseInt(p[0]) * 1000;
                    if ((ts == 0 && te == 0) || (ts <= dt && dt <= te)) {
                        datasetsSt.push({
                            x: dt,
                            y: parseInt(p[1])
                        });
                        datasetsBt.push({
                            x: dt,
                            y: parseInt(p[2])
                        });
                        // i++;
                    }
                }
            });
            this.charts['trecs'].dataset(datasetsSt, datasetsBt);
        }
    }, // tctChartRedraw
    setRefreshLogs: function (s) {
        const self = this;
        if (s) {
            this.intls['logarf'] = setInterval(function () {
                // do not query if not the dashbrd
                if (window.location.hash === '#home' || window.location.hash === '') {
                    self.logsys.reload();
                    self.logsnmp.reload();
                }
            }, self.refreshLogsIntl);
        } else {
            clearInterval(this.intls.logarf);
        }
    },
    initSidebar: function () {
        $('.sidebar li.item > a').on('click', event => {
            event.preventDefault();
            let tab = $(event.currentTarget).attr('href');
            if (tab !== '#home' && tab !== '#conf') tab = '#home';
            $('div.pagecontainer').each(function (i) {
                $(this).removeClass('grid');
                $(this).addClass('hidden');
            });
            $(tab).removeClass('hidden');
            $(tab).addClass('grid');
            // main menu
            $('.sidebar li.item > a').each(function (i) {
                $(this).removeClass('active');
            });
            $(event.currentTarget).addClass('active');
            // set location.hash
            window.location.hash = tab;
        });
        // activate menu item by location hash
        $('.sidebar li.item > a').each(function (i) {
            if (window.location.hash === $(this).attr('href')) {
                $(this).addClass('active');
            }
        });
    }, // initSidebar
    doReset: function (countdownEl, controlEl) {
        $(controlEl).attr('disabled', 'disabled');
        $(controlEl).addClass('disabled');
        $.ajax({
            url: $restURL.resetUrl,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.length == 0 || r.done !== true || r.err !== undefined) {
                    ohSnap(
                        $.t('index.js.errErasingConfigErr') + (r.err !== undefined ? ' (' + r.err + ')' : ''),
                        this.err
                    );
                } else {
                    ohSnap($.t('index.js.infoErasingConfig'), this.info);
                    this.showCountdownAt(countdownEl, this.resetCountdown, () => {
                        window.location.reload();
                    });
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // doReset
    doGenerateSerial: function (controlEl) {
        $.ajax({
            url: $restURL.genSerial,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.length == 0 || r.serial === undefined || r.err !== undefined) {
                    ohSnap(
                        $.t('index.js.errWhileGenerateSerial') + (r.err !== undefined ? ' (' + r.err + ')' : ''),
                        this.err
                    );
                } else {
                    let serialContainer = $('div.device-serial > div:first-child');
                    serialContainer.html(r.serial);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // doGenerateSerial
    /**
     * Parse dashboard data
     */
    parseDashboardData: function () {
        let sc = $('.syscard');
        let bt = $('.battcard');
        let nw = $('.netwcard');
        let battstate = batteryStatusToString(this.dashboardData.battst);
        let uptime = secondsToHRts(this.dashboardData.uptm);
        let lchd = new Date(this.dashboardData.batchd);
        let outputstatus = outputStatusToSting(this.dashboardData.outst);
        let battdiast = batteryDiagStatusToString(this.dashboardData.battdiast);
        let battlifetime = secondsToHRts(this.dashboardData.ltime);
        let currentDTime = new Date(this.dashboardData.ctime * 1000);
        // card: system
        sc.find('.outload').html(this.dashboardData.outload + '%');
        sc.find('.systmp').html(this.dashboardData.systmp + '&#176;C');
        sc.find('.ram').html(this.dashboardData.ram + ' / ' + this.dashboardData.ram3);
        sc.find('.uptm').html(uptime);
        sc.find('.battst').html(battstate);
        sc.find('.outst').html(outputstatus);
        // adding text decoration
        if (this.dashboardData.outst != 2) sc.find('.outst').addClass('animate-pulse animate-delay-700');
        else sc.find('.outst').removeClass('animate-pulse animate-delay-700');
        // card: battery
        bt.find('.battcap').html(this.dashboardData.battcap + '%');
        bt.find('.batchd').html(lchd.toLocaleDateString());
        if (this.dashboardData.isclng) {
            $('#fan748').attr('class', 'fill-[#EAE1FF] animate-spin animate-duration-2000 mr-2');
        } else {
            $('#fan748').attr('class', 'hidden');
        }
        bt.find('.battmp').html(this.dashboardData.battmp + '&#176;C');
        bt.find('.involt').html(this.dashboardData.involt + 'V / ' + this.dashboardData.infreq + 'Hz');
        bt.find('.outvolt').html(this.dashboardData.outvolt + 'V / ' + this.dashboardData.outfreq + 'Hz');
        bt.find('.battdiast').html(battdiast);
        bt.find('.ltime').html(battlifetime);
        // CARD: NETWORK
        nw.find('.ip').html(this.dashboardData.ip);
        nw.find('.ap').html(this.dashboardData.ap);
        nw.find('.sm').html(this.dashboardData.sm);
        nw.find('.gw').html(this.dashboardData.gw);
        nw.find('.mac').html(this.dashboardData.mac);
        nw.find('.ctime').html(
            currentDTime.toLocaleString([], {
                year: 'numeric',
                month: 'numeric',
                day: 'numeric',
                hour: '2-digit',
                minute: '2-digit'
            })
        );
    }, // parseDashboardData
    getDashData: function () {
        $.ajax({
            url: $restURL.getDashDataUrl,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.err !== undefined || r.length == 0) {
                    ohSnap(
                        $.t('index.js.errNoDataDashbrd') + (r.err !== undefined ? ' (' + r.err + ')' : ''),
                        this.err
                    );
                } else {
                    this.dashboardData = r;
                    this.parseDashboardData();
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // getDashData
    // @remind getCfg
    getCfg: function () {
        $.ajax({
            url: $restURL.getCfgUrl,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap($.t('index.js.errNoConfigData') + (r.err !== undefined ? ' (' + r.err + ')' : ''), this.err);
                } else {
                    $('input[name="battmplt"]').val(r.battmplt);
                    $('input[name="battmput"]').val(r.battmput);
                    $('input[name="devtmplt"]').val(r.devtmplt);
                    $('input[name="devtmput"]').val(r.devtmput);
                    $('input[name="ntpsrv"]').val(r.ntpsrv);
                    $('input[name="ntpsrvfb"]').val(r.ntpsrvfb);
                    $('input[name="ntpsrvsitl"]').val(r.ntpsrvsitl);
                    // TIMEZONEs
                    let list = $('select[name=ntptmoff]');
                    list.find('option').each(function (i) {
                        if (parseInt($(this)[0].value) === r.ntptmoff) {
                            list[0].selectedIndex = $(this)[0].index;
                        }
                    });
                    // SSIDs (this selector has event listener, see page init)
                    list = $('select[name="ssid"]');
                    list.data('default-value', r.ssid);
                    const opt = list.find('option[value="' + r.ssid + '"]');
                    if (opt.length == 0) {
                        // the default AP has not been found so add it
                        let node = document.createElement('option');
                        node.value = r.ssid;
                        node.innerHTML = r.ssid;
                        list.append(node);
                    }
                    // in either case select the option from settings
                    list.val(r.ssid);
                    // WiFi passkey
                    let ssidKeyInput = $('input[name="ssidkey"]');
                    ssidKeyInput.val(r.ssidkey);
                    ssidKeyInput.data('default-value', r.ssidkey);
                    $('input[name="apkey"]').val(r.apkey);
                    $('input[name="configsys"]').removeAttr('disabled');
                    // SNMP
                    $('input[name="snmpport"]').val(r.snmpport);
                    $('input[name="snmptrapport"]').val(r.snmptraport);
                    $('input[name="snmploctn"]').val(r.snmploctn);
                    $('input[name="snmpcontct"]').val(r.snmpcontct);
                    $('input[name="snmpbatrpldt"]').val(r.snmpbatrpldt);
                    $('input[name="configsnmp"]').removeAttr('disabled');
                    $('div.device-serial > div:first-child').html(
                        r.snum !== '' ? r.snum : $.t('index.js.pressGenSerial')
                    );
                    // SECURITY
                    $('input[name="authtmout"]').val(r.authtmout);
                    $('input[name="snmpgckey"]').val(r.snmpgckey);
                    $('input[name="snmpsckey"]').val(r.snmpsckey);
                    $('input[name="adlogin"]').val(r.adlogin);
                    $('input[name="adpass"]').val(r.adpass);
                    $('input[name="configsec"]').removeAttr('disabled');
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },
    setSysCfg: function () {
        let form = $('form[name=confsys]');
        let formdata = form.serializeArray();
        $.ajax({
            url: $restURL.sysSetCfgUrl,
            dataType: 'json',
            type: 'POST',
            data: formdata,
            success: r => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap($.t('js.errEmptyResponse') + (r.err !== undefined ? ' (' + r.err + ')' : ''), this.err);
                } else {
                    // success
                    ohSnap($.t('index.js.doneConfigUpdated'), this.info);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },
    setSNMPCfg: function () {
        const form = $('form[name=confsnmp]');
        const formdata = form.serializeArray();
        $.ajax({
            url: $restURL.snmpSetCfgUrl,
            dataType: 'json',
            type: 'POST',
            data: formdata,
            success: r => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap($.t('js.errEmptyResponse') + (r.err !== undefined ? ' (' + r.err + ')' : ''), this.err);
                } else {
                    // success
                    ohSnap($.t('index.js.doneConfigUpdated'), this.info);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    },
    setSecCfg: function () {
        let form = $('form[name=confsec]');
        let formdata = form.serializeArray();
        $.ajax({
            url: $restURL.secSetCfgUrl,
            dataType: 'json',
            type: 'POST',
            data: formdata,
            success: r => {
                if (r.length == 0 || r.err !== undefined) {
                    ohSnap($.t('js.errEmptyResponse') + (r.err !== undefined ? ' (' + r.err + ')' : ''), this.err);
                }
                // TODO
                else ohSnap($.t('index.js.doneConfigUpdated'), this.info);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o);
            }
        });
    }, // setSecCfg
    sendUpdate: function (el) {
        const self = this;
        const formData = new FormData();
        document.getElementById('modal-loader').classList.add('show');
        formData.append(el.name, el.files[0]);
        const hash = md5(el.result).toString();

        $.ajax({
            url: $restURL.updateUrl,
            contentType: false,
            processData: false,
            type: 'POST',
            cache: false,
            data: formData,
            dataType: 'json',
            headers: {
                'X-MD5': hash
            },
            success: r => {
                ohSnap($.t('js.updateSuccessful'), self.info);
                self.showCountdownAt('#cntr835', self.updateCountdown, () => {
                    window.location.reload();
                })
            },
            error: (o, ts, e) => {
                ohSnap('Update error', self.warn);
                document.getElementById('modal-loader').classList.remove('show');
            },
            complete: () => {
                el.value = '';
            }
        });
    } // sendUpdate
});

//
$(function () {
    tinyUPS.init();
});
