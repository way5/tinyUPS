import { createPopper } from '@popperjs/core';
/**
 * Dropdown elements
 * @param {*} selector
 */
export default function Dropdown(selector) {
    if (!selector) selector = "[data-dropdown]";
    let el = document.querySelectorAll(selector);
    //
    [...el].forEach(function (item) {
        item.addEventListener('click', function (e) {
            e.preventDefault();

            const offset = item.dataset.dropdownOffset
                ? [parseInt(item.dataset.dropdownOffset.split(',')[0]), parseInt(item.dataset.dropdownOffset.split(',')[1])]
                : [0, 0];
            let placement = item.dataset.dropdownPlacement ? item.dataset.dropdownPlacement : 'bottom-start';

            const dropdowns = $(item).siblings('.dropdown-menu');
            [...dropdowns].forEach((container) => {
                createPopper(item, container, {
                    placement: placement,
                    // strategy: 'fixed',
                    modifiers: [
                        {
                            name: 'offset',
                            options: {
                                offset: offset
                            }
                        },
                        {
                            name: 'preventOverflow',
                            options: {
                                padding: 8,
                                altAxis: true,
                                boundary: '#pagecontent'
                            }
                        }
                    ]
                });
            });
            item.classList.contains('show') ? item.classList.remove('show') : item.classList.add('show');
        });
        // hide
        document.addEventListener('mouseup', function (e) {
            e.preventDefault();
            if (
                item !== e.target
                    && $(e.target).parents('.clickable').length <= 0
                        && !e.target.classList.contains('clickable')
            ) {
                item.classList.remove('show');
            }
        });
    });
}
