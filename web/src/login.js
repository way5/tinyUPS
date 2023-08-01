import "./login.scss";
import { tinyUPS } from "./common.js";

$.extend(tinyUPS, {
    loginUrl: "/login",
    initPage: function() {},
    doLogin: function () {
        let self = this;
        let form = $("form[name=login]");
        // let formdata = new FormData(form[0]);
        let formdata = form.serializeArray();
        // Flowbite bug! Doesn't work like so
        // let modalAlert = new Modal(document.getElementById("modal-alert"), {
        //     backdrop: 'dynamic',
        //     closable: true
        // });
        $.ajax({
            url:
                window.location.protocol +
                "//" +
                window.location.hostname +
                this.loginUrl,
            // url: "http://local.ims:8888/?test=23",
            dataType: "json",
            type: "POST",
            data: formdata,
            success: function(r) {
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
                            window.location.reload();
                        }
                    }
                } else {
                    self.err("Login failed");
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            },
        });
    }, // doLogin
});

$(function () {
    tinyUPS.init();
    $("form[name=login]").on("submit", function (e) {
        e.preventDefault();
        tinyUPS.doLogin();
    });
});
