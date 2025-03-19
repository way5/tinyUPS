export default function Modal(selector) {
    if (!selector) selector = "[data-modal-toggle]";
    let el = document.querySelectorAll(selector);
    //
    [...el].forEach(function (item) {
        let dialog = document.querySelector(item.dataset.modalToggle) ?? document.getElementById(item.dataset.modalToggle);
        if(dialog) {
            let close = dialog?.querySelectorAll('.modal-close');
            let getAll = document.querySelectorAll('.modal');
            item.addEventListener('click', function (e) {
                dialog.classList.add('show');
                // document.body.classList.add('overflow-hidden');
                getAll.forEach(function (getItem) {
                    getItem !== dialog && getItem.classList.remove('show');
                });
            });
            close?.forEach(function (item) {
                item.addEventListener('click', function (e) {
                    e.preventDefault();
                    dialog.classList.remove('show');
                    // document.body.classList.remove('overflow-hidden');
                });
            });
        } else
            console.warn('(!) no such modal with selector: ' + item.dataset.modalToggle);
    });
}
