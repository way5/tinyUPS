/**
 * ? Change the languages that are going to be built into UI
 * ? the very first language here is also a fallback language
 */
const i18nlang = ["en", "es", "ru"];
// See: https://www.i18next.com/overview
import i18next from "i18next";
// See: https://github.com/i18next/jquery-i18next
const $i18next = require("jquery-i18next");
// See: https://github.com/i18next/i18next-browser-languageDetector
import LanguageDetector from "i18next-browser-languagedetector";
//
const i18nResources = () => {
    let a = {};
    for (let l in i18nlang) {
        a[i18nlang[l]] = require("../lang/" + i18nlang[l] + ".json");
    }
    return a;
};
//
i18next.use(LanguageDetector).init({
    debug: false,
    fallbackLng: i18nlang[0],
    useDataAttrOptions: true,
    resources: i18nResources(),
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