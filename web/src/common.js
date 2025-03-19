import './common.scss';
//
import './includes/i18n';
// See: https://github.com/way5/ohSnap-tailwind
import 'ohsnap-tailwind/ohsnap.scss';
import { ohSnap, ohSnapX } from 'ohsnap-tailwind';
//
import { $baseURL, $restURL } from './includes/url';
//
import './includes/settings';

const ohSnapConfig = {
    info: {
        title: $.t('js.alertInfo'),
        styles: {
            bg: 'bg-green-500 dark:bg-green-600',
            border: 'border-green-700',
            icon: 'ohsnap-info bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center'
        },
        duration: 7000,
        container: 'body',
        fadein: 500,
        fadeout: 500
    },
    warn: {
        title: $.t('js.alertWarn'),
        styles: {
            bg: 'bg-yellow-500 dark:bg-yellow-600',
            border: 'border-yellow-700',
            icon: 'ohsnap-warn bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center'
        },
        duration: 7000,
        container: 'body',
        fadein: 500,
        fadeout: 500
    },
    err: {
        title: $.t('js.alertErr'),
        styles: {
            bg: 'bg-red-500 dark:bg-red-700',
            border: 'border-red-700',
            icon: 'ohsnap-err bg-white dark:bg-white bg-[length:28px_28px] bg-no-repeat bg-center'
        },
        duration: 7000,
        container: 'body',
        fadein: 500,
        fadeout: 500
    }
};

var tinyUPS = {
    rebootCountdownIntl: 10000,
    err: ohSnapConfig.err,
    warn: ohSnapConfig.warn,
    info: ohSnapConfig.info,
    uiVer: null,
    fwVer: null,
    /**
     * Doc-scope initializer. Called manually
     */
    init: function () {
        // this.displayMode();
        $('#dmode').on('click', e => {
            e.preventDefault();
            e.stopPropagation();
            // this.displayMode(true);
            $(e.target);
            window.settings.displayMode(true);
        });
        // password unhide
        $('button.showkey').on('click', function (e) {
            e.preventDefault();
            let s = $(this).parent().find('input[type]');
            s.prop('type', s.prop('type') === 'text' ? 'password' : 'text');
            if (s.prop('type') === 'text') {
                $(this).addClass('active');
            } else {
                $(this).removeClass('active');
            }
        });
        // survey
        $('#dosurvey').on('click', e => {
            e.preventDefault();
            this.getSurvey();
        });
        var pkg = require('../../package.json');
        this.uiVer = pkg.version;
        pkg = null;
        var pkg = require('../../configure.json');
        this.fwVer = pkg.version;
        pkg = null;
        // footer
        $('a[data-i18n="[title]gotoGithubLink"]').append(
            new Date().getFullYear() + ' (fw: ' + this.fwVer + ' / ui: ' + this.uiVer + ')'
        );
        this.initPage();
        // i18n
        $('html').localize();
    },
    handleErrorResponse: function (o, ts, e) {
        if (o.status === 401) {
            window.location.reload();
        } else {
            console.error(ts + ': ' + o.responseText);
            ohSnap($.t('js.errCommunication'), this.err);
        }
    },
    getSurvey: function () {
        const self = this;
        $.ajax({
            url: $restURL.surveyUrl,
            type: 'POST',
            dataType: 'json',
            success: r => {
                // returned as an object, so the painful version of length
                // create node
                if (r.delay !== undefined) {
                    let delay = parseInt(r.delay);
                    console.log('scan delay detected: ', delay);
                    ohSnap(
                        delay > 0 ? $.t('js.scanRepeatIn', { ds: delay / 1000.0 }) : $.t('js.scanInProgress'),
                        self.info
                    );
                    setTimeout(() => {
                        self.getSurvey();
                    }, delay);
                } else if (r.length != 0) {
                    const list = $('select[name="ssid"]');
                    // get currently selected option value
                    let currentSSID = list.val();
                    let currentSSIDtext = '';
                    let selectedOption = list[0].options[list[0].options.selectedIndex];
                    if (selectedOption) {
                        currentSSIDtext = selectedOption.text;
                    }
                    self.scanResult = r;
                    list.html('');
                    //
                    const addListNode = (val, text) => {
                        let node = document.createElement('option');
                        node.value = val;
                        node.innerHTML = text;
                        list.append(node);
                    };
                    // option text feeder
                    const optionText = r => {
                        let enc = 'TKIP';
                        switch (r.e) {
                            case 4:
                                enc = 'CCMP';
                                break;
                            case 5:
                                enc = 'WEP';
                                break;
                            case 7:
                                enc = 'NONE';
                                break;
                            case 8:
                                enc = 'AUTO';
                                break;
                        }
                        return r.s + ' (ENC: ' + enc + ' RSSI: ' + r.r + ')';
                    };
                    // append the default option
                    if (currentSSID !== null && currentSSID !== undefined) {
                        addListNode(currentSSID, currentSSIDtext);
                    }
                    // parse
                    for (let i = 0; i < r.length; i++) {
                        if (r[i].s === '' || r[i].s === null) continue;
                        // if the default option exists in results
                        if (r[i].s !== currentSSID) {
                            addListNode(r[i].s, optionText(r[i]));
                        } else {
                            // if found same SSID as the currentSSID, then update text
                            let k = list[0].options.selectedIndex;
                            list[0].options[k].text = optionText(r[i]);
                        }
                    }
                    ohSnap($.t('js.scanComplete'), self.info);
                } else ohSnap($.t('js.errNoWiFiDetect'), self.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    }, // getSurvey
    showCountdownAt: function (whereTo, duration, thenDo = null, step = 1000) {
        const el = $(whereTo);
        if(el.length !== 0) {
            let countdown = duration / step;
            $(el).html(countdown);
            setInterval(() => {
                countdown -= 1;
                if (countdown >= 0) {
                    $(el).html(countdown);
                    if (countdown === 0) {
                        (thenDo !== null && typeof thenDo === 'function') && thenDo();
                    }
                }
            }, step);
        } else
            console.warn(`no elements like: ${whereTo} for countdown couter`);
    },  // showCountdownAt
    doReboot: function (countdownEl, controlEl = null) {
        const self = this;
        if (controlEl !== null) {
            $(controlEl).attr('disabled', 'disabled');
            $(controlEl).addClass('disabled');
        }
        $.ajax({
            url: $restURL.rebootURL,
            dataType: 'json',
            type: 'POST',
            success: r => {
                if (r.length != 0) {
                    if (r.done === true) {
                        ohSnap($.t('js.deviceIsRebooting'), self.info);
                        self.showCountdownAt(countdownEl, self.rebootCountdownIntl, () => {
                            window.location.reload();
                        });
                    }
                    // else
                    // console.log('wrong answer: ', r);
                } else ohSnap($.t('js.errDoReboot'), self.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    } // doReboot
};

window.$ = jQuery;
window.tinyUPS = tinyUPS;

export { tinyUPS, ohSnap, ohSnapX, $restURL, $baseURL };
