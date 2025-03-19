import './login.scss';
import { tinyUPS, ohSnap, ohSnapX, $restURL } from './common.js';
// components
import Modal from './includes/modal.js';

$.extend(tinyUPS, {
    /**
     * Setup page
     */
    initPage: function () {
        Modal();
    },
    doLogin: function () {
        let form = $('form[name=login]');
        let formdata = form.serializeArray();
        $.ajax({
            url: $restURL.loginUrl,
            dataType: 'json',
            type: 'POST',
            data: formdata,
            success: r => {
                if (r.length != 0 && r.login !== 'err') {
                    // auth results
                    if (r.login === 'repeat') {
                        $('#modal-alert').addClass('show');
                        // modalAlert.show();
                    } else {
                        // console.log("logged in");
                        window.location.reload();
                    }
                } else {
                    if (r.length != 0) ohSnap(r.err, this.warn);
                    else ohSnap($.t('js.errLoginFailed'), this.err);
                }
            },
            error: (o, ts, e) => {
                this.handleErrorResponse(o, ts, e);
            }
        });
    } // doLogin
});

$(function () {
    tinyUPS.init();
    $('form[name=login]').on('submit', function (e) {
        e.preventDefault();
        tinyUPS.doLogin();
    });
});
