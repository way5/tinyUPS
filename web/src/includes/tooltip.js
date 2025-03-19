import { createPopper } from "@popperjs/core";

const $class_top = `[&[data-tooltip-placement^='top']]:after:top-full [&[data-tooltip-placement^='top']]:after:left-1/2 [&[data-tooltip-placement^='top']]:after:-translate-x-1/2 [&[data-tooltip-placement^='top']]:after:border-t-gray-800 [&[data-tooltip-placement^='top']]:after:border-t-6 [&[data-tooltip-placement^='top']]:after:border-r-6 [&[data-tooltip-placement^='top']]:after:border-b-0 [&[data-tooltip-placement^='top']]:after:border-l-6`;
const $class_right = `[&[data-tooltip-placement^='right']]:after:right-full [&[data-tooltip-placement^='right']]:after:top-1/2 [&[data-tooltip-placement^='right']]:after:-translate-y-1/2 [&[data-tooltip-placement^='right']]:after:border-r-gray-800 [&[data-tooltip-placement^='right']]:after:border-t-6 [&[data-tooltip-placement^='right']]:after:border-r-6 [&[data-tooltip-placement^='right']]:after:border-b-6 [&[data-tooltip-placement^='right']]:after:border-l-0`;
const $class_bottom = `[&[data-tooltip-placement^='bottom']]:after:bottom-full [&[data-tooltip-placement^='bottom']]:after:left-1/2 [&[data-tooltip-placement^='bottom']]:after:-translate-x-1/2 [&[data-tooltip-placement^='bottom']]:after:border-b-gray-800 [&[data-tooltip-placement^='bottom']]:after:border-t-0 [&[data-tooltip-placement^='bottom']]:after:border-r-6 [&[data-tooltip-placement^='bottom']]:after:border-b-6 [&[data-tooltip-placement^='bottom']]:after:border-l-6`;
const $class_left = `[&[data-tooltip-placement^='left']]:after:left-full [&[data-tooltip-placement^='left']]:after:top-1/2 [&[data-tooltip-placement^='left']]:after:-translate-y-1/2 [&[data-tooltip-placement^='left']]:after:border-l-gray-800 [&[data-tooltip-placement^='left']]:after:border-t-6 [&[data-tooltip-placement^='left']]:after:border-r-0 [&[data-tooltip-placement^='left']]:after:border-b-6 [&[data-tooltip-placement^='left']]:after:border-l-6`;
/**
 * Tooltip elements
 * @param {*} selector
 */
export default function Tooltip(selector = null) {
    if (!selector) selector = "[data-tooltip]";
    let el = document.querySelectorAll(selector);
    //
    [...el].forEach(function (item) {
        const title = item.dataset.tooltip ? $.t(item.dataset.tooltip) : false;
        if (title) {
            const id = "id" + Math.random().toString(16).slice(2);
            item.setAttribute("data-target", id);
            const markup = (c) => {
                return `<div tabindex="0" id="${id}" class="tooltip ${c}">${title}</div>`;
            };
            // show
            item.addEventListener("mouseenter", function (e) {
                e.preventDefault();
                const offset = item.dataset.offset
                    ? item.dataset.offset
                    : [0, 10];
                const placement = item.dataset.tooltipPlacement
                    ? item.dataset.tooltipPlacement
                    : "right";

                let ttpClass = $class_top;
                if (placement === "right") {
                    ttpClass = $class_right;
                } else if (placement === "bottom") {
                    ttpClass = $class_bottom;
                } else if (placement === "left") {
                    ttpClass = $class_left;
                }

                document.body.insertAdjacentHTML("beforeend", markup(ttpClass));
                const tooltip = document.getElementById(item.dataset.target);
                createPopper(item, tooltip, {
                    placement: placement,
                    modifiers: [
                        {
                            name: "offset",
                            options: {
                                offset: offset,
                            },
                        },
                        {
                            name: "flip",
                            options: {
                                fallbackPlacements: ["top", "bottom"],
                            },
                        },
                    ],
                });
            });
            // hide
            item.addEventListener("mouseleave", function (e) {
                e.preventDefault();
                const tooltip = document.getElementById(item.dataset.target);
                tooltip && tooltip.remove();
            });
        }
    });
}
