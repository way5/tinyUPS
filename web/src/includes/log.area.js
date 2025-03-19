import { $baseURL } from './url';

/**
 * Do element scroll to bottom
 * @param {*} o
 */
const containerScrollBottom = (o) => {
    o.scroll({ top: o.scrollHeight, behavior: "smooth" });
};
/**
 * Log area objects
 * @author sk
 *
 * @class logArea
 * @typedef {logArea}
 */
export default class logArea {
    constructor(o) {
        this.obj = o;
        this.url = $(o).attr("data-url");
        this.name = $(o).attr("data-name");
    }

    reload() {
        $.ajax({
            url: $baseURL + this.url,
            dataType: "text",
            type: "POST",
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
window.logArea = logArea;
window.scrollBottom = containerScrollBottom;