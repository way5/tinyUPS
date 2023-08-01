/** @type {import('tailwindcss').Config} */
const colors = require('tailwindcss/colors');

module.exports = {
    mode: 'jit',
    content: [
        "./src/**/*.{html,js,css,ico}",
        "./node_modules/flowbite/**/*.js"
    ],
    darkMode: "class",      // (false|media|class)
    theme: {
        screens: {
            '2xs': { 'max': '322px' },
            'xs': { 'min': '323px', 'max': '639px' },
            'smdn': { 'max': '767px' },
            'sm': { 'min': '640px', 'max': '767px' },
            'md': { 'min': '768px', 'max': '1023px' },
            'mddn': { 'max': '768px' },
            'mdup': { 'min': '768px' },
            'lg': { 'min': '1024px', 'max': '1279px' },
            'xl': { 'min': '1280px', 'max': '1535px' },
            '2xl': { 'min': '1536px' },
        },
        extend: {
            fontFamily: {
                'narrow': ['Pragati Narrow', 'Arial Narrow', 'sans-serif'],
                'roboto': ['Roboto Condensed', 'Tahoma', 'sans-serif'],
                'exo': ['Exo 2', 'Lucida Grande', 'Helvetica Neue', 'Arial'],
            },
            fontSize: {
                // body: ["1rem", "1.6875rem"],
                // "h1":   ["2.5rem", "3rem"],
                // "h2":   ["2rem", "2.5rem"],
                // "h3":   ["1.75rem", "2.3125rem"],
                // "h4":   ["1.5rem", "1.9375rem"],
                // "h5":   ["1.25rem", "1.625rem"],
                // "h6":   ["1rem", "1.3125rem"],
                'xs': '.625rem',    // 10px
                'sm': '.75rem',     // 12px
                'tiny': '.875rem',  // 14px
                'base': '1rem',     // 16px
                'lg': '1.375rem',   // 22px
                'xl': '1.75rem',    // 28px
                '2xl': '2.25rem',   // 36px
                '3xl': '2.625rem',  // 42px
                '4xl': '3rem',      // 48px
                '5xl': '3.25rem',   // 52px
                '6xl': '3.625rem',  // 58px
                '7xl': '3.875rem',  // 62px
            },
            borderRadius: {
                'none': '0',
                'sm': '5px',
                'md': '10px',
                'lg': '15px',
                'xl': '20px',
                '2xl': '30px',
                default: '5px',
            },
            boxShadow: {
                'sm': "0 1px 2px 0 rgb(0 0 0 / 0.05)",
                'md': "0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1)",
                'lg': "0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1)",
                'xl': "0 20px 25px -5px rgb(0 0 0 / 0.1), 0 8px 10px -6px rgb(0 0 0 / 0.1)",
                '2xl': "0 25px 50px -12px rgb(0 0 0 / 0.25)",
                default: "0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1)",
                inner: "inset 0 2px 4px 0 rgb(0 0 0 / 0.05)",
                none: "0 0 rgb(0, 0 / 0, 0)"
            },
            backgroundImage: {},
            colors: {
                inherit: "inherit",
                current: "currentColor",
                transparent: "transparent",
                white: "#ffffff",
                black: "#000000",
                'red': {
                    '50': '#fff1f4',
                    '100': '#ffe3e8',
                    '200': '#ffcbd8',
                    '300': '#ffa1b8',
                    '400': '#ff6d94',
                    '500': '#fa3972',
                    '600': '#e91f64',
                    '700': '#c40c4f',
                    '800': '#a40d49',
                    '900': '#8c0f45',
                    '950': '#4e0321',
                },
                'yellow': {
                    '50': '#fffbeb',
                    '100': '#fef3c7',
                    '200': '#fde58a',
                    '300': '#fbd24e',
                    '400': '#fabe25',
                    '500': '#f49d0c',
                    '600': '#d87607',
                    '700': '#bc560a',
                    '800': '#923f0e',
                    '900': '#78340f',
                    '950': '#451a03',
                },
                'light-green': {
                    '50': '#ecfdf3',
                    '100': '#d1fae1',
                    '200': '#a7f3c9',
                    '300': '#6ee7ac',
                    '400': '#34d38b',
                    '500': '#0fa968',
                    '600': '#05965c',
                    '700': '#04784c',
                    '800': '#065f3e',
                    '900': '#064e34',
                    '950': '#022c1e',
                },
                'green': {
                    '50': '#f6faf3',
                    '100': '#e9f5e3',
                    '200': '#d3eac8',
                    '300': '#afd89d',
                    '400': '#82bd69',
                    '500': '#61a146',
                    '600': '#4c8435',
                    '700': '#3d692c',
                    '800': '#345427',
                    '900': '#2b4522',
                    '950': '#13250e',
                },
                'violet': {
                    '50': '#f3f1ff',
                    '100': '#ebe5ff',
                    '200': '#d9ceff',
                    '300': '#bea6ff',
                    '400': '#9f75ff',
                    '500': '#843dff',
                    '600': '#7916ff',
                    '700': '#6b04fd',
                    '800': '#5a03d5',
                    '900': '#4b05ad',
                    '950': '#2c0076',
                },
                'brown': {
                    '50': '#fbf6f5',
                    '100': '#f6ecea',
                    '200': '#f0dcd8',
                    '300': '#e4c3bd',
                    '400': '#d3a096',
                    '500': '#ba7264',
                    '600': '#aa6558',
                    '700': '#8e5347',
                    '800': '#77463d',
                    '900': '#643f38',
                    '950': '#351e1a',
                },
                'gray': {
                    '50': '#f8f8f8',
                    '100': '#f0f0f0',
                    '200': '#e4e4e4',
                    '300': '#d1d1d1',
                    '400': '#b4b4b4',
                    '500': '#9a9a9a',
                    '600': '#8a8a8a',
                    '700': '#6a6a6a',
                    '800': '#5a5a5a',
                    '900': '#4e4e4e',
                    '950': '#282828',
                },
                'base': {
                    '50': '#f5f4f9',
                    '100': '#dfdaed',
                    '200': '#bfb5da',
                    '300': '#9888c0',
                    '400': '#745fa2',
                    '450': '#664e91',
                    '500': '#5c4686',
                    '550': '#543d80',
                    '600': '#49366b',
                    '700': '#3e2e57',
                    '800': '#342847',
                    '900': '#2e253c',
                    '950': '#0a070e',
                },
            }
        },
    },
    plugins: [
        // require("daisyui"),
        // * https://github.com/themesberg/flowbite
        require('flowbite/plugin'),
        require("@tailwindcss/forms"),
        // require("@tailwindcss/aspect-ratio"),
        require("@tailwindcss/typography"),
        // require("@tailwindcss/line-clamp"),
    ],
};
