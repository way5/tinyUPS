/**
 *
 * @param {*} s
 * @returns {*}
 */
const outputStatusToSting = (s) => {
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
 * @param {*} t
 * @returns {string}
 */
const secondsToHRts = (t) => {
    let value = 0;
    let ts = "";
    if (t > 31556952) {
        value = Math.floor(t / 31556952);
        t = t % 31556952;
        ts = value + " " + $.t("js.year", { count: value }) + " ";
    }
    if (t > 86400) {
        value = Math.floor(t / 86400);
        t = t % 86400;
        ts = ts + value + " " + $.t("js.day", { count: value }) + " ";
    }
    if (t > 3600) {
        value = Math.floor(t / 3600);
        t = t % 3600;
        ts = ts + value + " " + $.t("js.hr", { count: value }) + " ";
    }
    if (t > 60 && value == 0) {
        value = Math.floor(t / 60);
        t = t % 60;
        ts = ts + value + " " + $.t("js.min") + " ";
    }
    if (t > 0 && value == 0) {
        ts = ts + t + " " + $.t("js.sec");
    }
    return ts;
};

/**
 *
 * @param {*} s
 * @returns {*}
 */
const batteryDiagStatusToString = (s) => {
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
 * Returns the battery status name and/or changes dashboard indicator state
 * @param {*} t
 * @returns {*}
 */
const batteryStatusToString = (t, alterIndicators = true) => {
    let r = $.t("js.battStNormal");
    if (alterIndicators) {
        $(".indicators-wrapper").removeClass("battery-low-alert battery-faulty-alert battery-unknown battery-normal");
        $(".sidebar li:first-child").removeClass("battery-low-nav-alert battery-faulty-nav-alert");
    }
    switch (t) {
        default:
        case 1:     // Unknown
            alterIndicators && $(".indicators-wrapper").addClass("battery-unknown");
                // pt.attr("class", "blink-led fill-[#7a7a7a]");
            r = $.t("js.battStUnknown");
            break;
        case 2:     // Normal
            alterIndicators && $(".indicators-wrapper").addClass("battery-normal");
                // pt.attr("class", "blink-led fill-[#8df478]");
            break;
        case 3:     // Battery Low
            if (alterIndicators) {
                // pt.attr("class", "blink-led fill-[#ffda90]");
                // swith UI innto notification mode
                $(".indicators-wrapper").addClass("battery-low-alert");
                $(".sidebar li:first-child").addClass(
                    "battery-low-nav-alert"
                );
            }
            r = $.t("js.battStLow");
            break;
        case 4:     // Battery faulty
            if (alterIndicators) {
                // pt.attr("class", "blink-led fill-[#ff9898]");
                $(".indicators-wrapper").addClass("battery-faulty-alert");
                $(".sidebar li:first-child").addClass(
                    "battery-faulty-nav-alert"
                );
            }
            r = $.t("js.battStFaulty");
            break;
    }
    return r;
};

/**
 * Get nnumber of Days in a month
 * @param {*} month range: 0-11
 * @param {*} year
 * @returns
 */
function daysInMonth(year, month) {
    switch (month) {
        case 1: // February
            return (year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0)) ? 29 : 28;
        case 0: // January
        case 2: // March
        case 4: // May
        case 6: // July
        case 7: // August
        case 9: // October
        case 11: // December
            return 31;
        case 3: // April
        case 5: // June
        case 8: // September
        case 10: // November
            return 30;
        default:
            return -1; // Invalid month
    }
}

function parents(el, selector, filter) {
    let parentSelector = (selector === undefined) ? document : document.querySelector(selector);
    var parents = [];
    var pNode = el.parentNode;

    while (pNode !== parentSelector) {
        var element = pNode;

        if(filter === undefined){
            parents.push(element); // Push that parentSelector you wanted to stop at
        }else{
            element.classList.contains(filter) && parents.push(element);
        }
        pNode = element.parentNode;
    }

    return parents;
}


export {
    outputStatusToSting,
    secondsToHRts,
    batteryDiagStatusToString,
    batteryStatusToString,
    daysInMonth,
    parents
}