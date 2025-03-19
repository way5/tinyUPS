// See: https://apexcharts.com/docs
import ApexCharts from "apexcharts";
// themes
import { lineChartLightColors, lineChartDarkColors } from "./chart.theme";

/**
 * Common options
 */
const tempTimelineChartOptions = (theme) => {
    let zMinTct = new Date();
    zMinTct.setHours(0, 0, 0, 0);
    let zMaxTct = new Date();
    zMaxTct.setHours(23, 59, 59, 99);
    let chartTheme = theme === "light" ? lineChartLightColors : lineChartDarkColors;
    return  {
        series: [
            {
                name: $.t("index.js.opchDeviceTemp"),
                data: [],
            },
            {
                name: $.t("index.js.opchBatteryTemp"),
                data: [],
            },
        ],
        colors: ["rgba(245, 109, 4, 1)", "rgba(225, 3, 40, 1)"],
        chart: {
            height: 300,
            // offsetY: -20,
            type: "line",
            animations: {
                enabled: true,
                easing: "linear",
                dynamicAnimation: {
                    speed: 1000,
                },
            },
            dropShadow: {
                enabled: true,
                enabledSeries: [0],
                top: -2,
                left: 2,
                blur: 5,
                opacity: 0.06,
            },
            toolbar: {
                show: false,
            },
            zoom: {
                enabled: true,
                autoScaleYaxis: true,
            },
        },
        dataLabels: {
            enabled: false,
        },
        stroke: {
            curve: "smooth",
            width: 3,
        },
        // title: {
        //     text: "Dynamic Updating Chart",
        //     align: "left",
        // },
        grid: {
            xaxis: {
                lines: {
                    show: true,
                },
            },
            yaxis: {
                lines: {
                    show: true,
                },
            },
            borderColor: chartTheme.gridBorderColor,
            row: {
                colors: chartTheme.gridRowColors,
                // opacity: 0.5,
            },
        },
        markers: {
            size: 0,
            strokeWidth: 3,
            strokeOpacity: 1,
            fillOpacity: 1,
            hover: {
                size: 6,
            },
        },
        xaxis: {
            type: "datetime",
            min: zMinTct.getTime() - 11037600000,
            max: zMaxTct.getTime() + 11037600000,
            minRange: 3600000,
            labels: {
                datetimeUTC: false,
                style: {
                    colors: chartTheme.xaxis.labelColors,
                },
            },
            axisBorder: {
                color: chartTheme.xaxis.axisBorderColor,
            },
            axisTicks: {
                color: chartTheme.xaxis.axisTicksColor,
            },
            title: {
                style: {
                    color: chartTheme.xaxis.titleColor,
                },
            },
            crosshairs: {
                stroke: {
                    color: chartTheme.xaxis.crosshairsStrokeColor,
                },
                fill: {
                    color: chartTheme.xaxis.crosshairsFillColor,
                },
            },
        },
        yaxis: {
            // min: 5,
            // max: 40,
            title: {
                text: $.t("index.js.opchTitleTemp"),
                style: {
                    color: chartTheme.yaxis.titleColor,
                },
            },
            labels: {
                style: {
                    colors: chartTheme.yaxis.labelColors,
                },
            },
            axisBorder: {
                show: true,
                color: chartTheme.yaxis.axisBorderColor,
            },
            axisTicks: {
                show: true,
                color: chartTheme.yaxis.axisTicksColor,
            },
            crosshairs: {
                stroke: {
                    color: chartTheme.yaxis.crosshairsStrokeColor,
                },
                fill: {
                    color: chartTheme.yaxis.crosshairsFillColor,
                },
            },
        },
        legend: {
            show: true,
            position: "bottom",
            horizontalAlign: "center",
            floating: true,
            offsetY: -280,
            // offsetX: -5,
            labels: {
                colors: chartTheme.labelColors
            },
        },
        tooltip: {
            // enabled: true,
            theme: theme,
            // fillSeriesColor: false,
            x: {
                show: false,
                format: "MMM dd, HH:mm:ss",
            },
        },
        markers: {
            colors: undefined,
            strokeColors: ["rgba(225, 3, 40, 0.4)", "rgba(245, 108, 4, 0.4)"],
        },
    };
};

export default class tempTimelineChart {
    constructor(selector) {
        this.chart = new ApexCharts(
            document.getElementById(selector),
            tempTimelineChartOptions(window.settings.get("theme", "light"))
        );
        this.chart.render();
        // theme change
        document.addEventListener("themeChange", (e) => {
            this.chart.updateOptions(tempTimelineChartOptions(e.detail.theme));
        });
    }

    dataset(st = [], bt = []) {
        this.chart.updateSeries([
            {
                data: st,
            },
            {
                data: bt,
            },
        ]);
    }

    reset() {
        this.chart.resetSeries();
    }
}
