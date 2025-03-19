class Settings {
    constructor() {
        this.themeChangeEvent = new CustomEvent("themeChange", {
            detail: {
                theme: "",
            },
        });
        this.displayMode();
    }

    displayMode(toggle = false) {
        let theme = this.get("theme", "light");
        if (
            theme === "dark" ||
            (!("tinyUPS" in localStorage) &&
                window.matchMedia("(prefers-color-scheme: dark)").matches)
        ) {
            if (toggle) {
                theme = 'light';
                document.documentElement.classList.remove("dark");
            } else {
                theme = 'dark';
                document.documentElement.classList.add("dark");
            }
        } else {
            if (toggle) {
                theme = 'dark';
                document.documentElement.classList.add("dark");
            } else {
                theme = 'light';
                document.documentElement.classList.remove("dark");
            }
        }
        this.set("theme", theme);
        this.themeChangeEvent.detail.theme = theme;
        document.dispatchEvent(this.themeChangeEvent);
    }

    get(what, byDefault = null) {
        let sett = localStorage.getItem("tinyUPS");
        if (sett && sett !== undefined) {
            sett = JSON.parse(sett);
            if (sett[what] !== undefined) {
                return sett[what];
            } else {
                return byDefault;
            }
        }
    }

    set(what, value) {
        let sett = localStorage.getItem("tinyUPS");
        if (!sett || sett === undefined) {
            sett = {};
        } else {
            sett = JSON.parse(sett);
        }
        localStorage.setItem("tinyUPS", JSON.stringify({...sett, [what]: value }));
    }
}

window.settings = new Settings();