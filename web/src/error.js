import "./error.scss";
import { tinyUPS } from "./common.js";

$.extend(tinyUPS, {
    initPage: function() {},
});

$(function () {
    tinyUPS.init();
});