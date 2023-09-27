import "./setup.scss";
import { tinyUPS, ohSnap, ohSnapX } from "./common.js";

$.extend(tinyUPS, {
    setupUrl: "/setup",
    setupPageDataUrl: "/setup-init",
    /**
     * Setup page
     */
    initPage: function () {
        let self = this;
        // reset button
        $("button.reset-solo").on("click", (e) => {
            self.doReboot("button.reset-solo", "button.reset-solo");
            $("button.reset-solo").attr("disabled", "disabled");
        });
        // only for POSTS
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.setupPageDataUrl,
            dataType: "json",
            type: 'POST',
            success: (r) => {
                // create node
                if (r.length != 0) {
                    $(".ap-mac-address").html("MAC: " + r.mac);
                } else {
                    ohSnap($.t("setup.js.errNoInitReceived"), self.err);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
    /**
     * Send setup form
     */
    doSetup: function () {
        let self = this;
        let form = $("form[name=setup]");
        // let modalDone = new Modal(document.getElementById("modal"));
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
            ohSnap($.t("setup.js.infLoginAlert"), self.info);
            return;
        }
        let passformat = new RegExp(
            "^(((?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])))(?=.{6,})"
        );
        if (!passformat.test(form.find("[name=pass]").val())) {
            ohSnap($.t("setup.js.infPassAlert"), self.info);
            return;
        }
        // testing parameters
        // modalTest.show();
        // only for POSTS
        // let formdata = new FormData(form[0]);
        let formdata = form.serializeArray();
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.setupUrl,
            // url: "http://testURL/?test=22",
            dataType: "json",
            type: 'POST',
            data: formdata,
            success: (r) => {
                if (r.setup === "fail") {
                    // modalTest.hide();
                    switch (r.err) {
                        case "sta":
                            ohSnap($.t("setup.js.infNoWiFiConnect"), self.info);
                            return;
                        case "data":
                            ohSnap($.t("setup.js.infWrongParams"), self.info);
                            return;
                        default:
                            break;
                    }
                } else if (r.setup === "done") {
                    // modalDone.show();
                    $("#modal").show();
                    return;
                }
                ohSnap($.t("setup.js.errServerResponse"), self.err);
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    },
});

// document ready
$(function () {
    tinyUPS.init();
    tinyUPS.getSurvey();
    $("form[name=setup]").on("submit", function (e) {
        e.preventDefault();
        tinyUPS.doSetup();
    });
});
