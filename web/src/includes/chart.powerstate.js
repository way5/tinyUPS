// See: https://apexcharts.com/docs
import ApexCharts from "apexcharts";
// themes
import { heatmapChartLightColors, heatmapChartDarkColors } from "./chart.theme";
// Helpers
import { secondsToHRts, outputStatusToSting } from "./helpers.js";

export default class powerStateChart {
    constructor(selector) {
        this.data = [];
        this.chart = new ApexCharts(
            document.getElementById(selector),
            this.powerStateChartOptions(window.settings.get("theme", "light"))
        );
        this.chart.render();
        //* theme change must be called from parent since method
        //* that does redraw it is there, see: this.update
        // document.addEventListener("themeChange", (e) => {
        //     this.chart.updateOptions(
        //         this.powerStateChartOptions(e.detail.theme)
        //     );
        //     this.reset();
        // });
    }

    update(theme) {
        this.chart.updateOptions(
            this.powerStateChartOptions(theme)
        );
        this.reset();
    }

    dataset(d = []) {
        this.data = d;
        this.chart.updateSeries([
            {
                data: d[0],
            },
            {
                data: d[1],
            },
            {
                data: d[2],
            },
            {
                data: d[3],
            },
            {
                data: d[4],
            },
            {
                data: d[5],
            },
            {
                data: d[6],
            },
            {
                data: d[7],
            },
            {
                data: d[8],
            },
            {
                data: d[9],
            },
            {
                data: d[10],
            },
            {
                data: d[11],
            },
        ]);
    }

    reset() {
        this.chart.resetSeries();
    }

    powerStateChartOptions(theme) {
        const self = this;
        let chartTheme =
            theme === "light"
                ? heatmapChartLightColors
                : heatmapChartDarkColors;
        return {
            series: [
                {
                    name: $.t("months.jan"),
                    data: [],
                },
                {
                    name: $.t("months.feb"),
                    data: [],
                },
                {
                    name: $.t("months.mar"),
                    data: [],
                },
                {
                    name: $.t("months.apr"),
                    data: [],
                },
                {
                    name: $.t("months.may"),
                    data: [],
                },
                {
                    name: $.t("months.jun"),
                    data: [],
                },
                {
                    name: $.t("months.jul"),
                    data: [],
                },
                {
                    name: $.t("months.aug"),
                    data: [],
                },
                {
                    name: $.t("months.sep"),
                    data: [],
                },
                {
                    name: $.t("months.oct"),
                    data: [],
                },
                {
                    name: $.t("months.nov"),
                    data: [],
                },
                {
                    name: $.t("months.dec"),
                    data: [],
                },
            ],
            chart: {
                height: 300,
                // offsetY: -25,
                type: "heatmap",
                toolbar: {
                    show: false,
                },
                animations: {
                    enabled: true,
                    easing: "linear",
                    dynamicAnimation: {
                        speed: 1000,
                    },
                },
                zoom: {
                    enabled: false,
                },
            },
            plotOptions: {
                heatmap: {
                    enableShades: true,
                    shadeIntensity: 0.5,
                    radius: 7,
                    useFillColorAsStroke: false,
                    // distributed: true,
                    colorScale: {
                        ranges: [
                            {
                                from: -1,
                                to: 0,
                                name: $.t("index.js.severityNone"),
                                color: "rgb(10, 117, 0)",
                            },
                            {
                                from: 1,
                                to: 1,
                                name: $.t("index.js.severityLow"),
                                color: "rgb(197, 255, 38)",
                            },
                            {
                                from: 2,
                                to: 3,
                                name: $.t("index.js.severityMedium"),
                                color: "rgb(255, 169, 41)",
                            },
                            {
                                from: 4,
                                to: 9,
                                name: $.t("index.js.severityHigh"),
                                color: "rgb(255, 54, 36)",
                            },
                            {
                                from: 10,
                                to: 100,
                                name: $.t("index.js.severityExtreme"),
                                color: "rgb(88, 0, 0)",
                            },
                        ],
                    },
                },
            },
            dataLabels: {
                enabled: false,
            },
            stroke: {
                width: 1,
            },
            grid: {
                // position: 'front',
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
            xaxis: {
                type: "numeric",
                tickAmount: 31,
                // tickPlacement: "between",
                decimalsInFloat: 0,
                labels: {
                    style: {
                        colors: chartTheme.xaxis.labelColors,
                    },
                    formatter: function(value, timestamp, opts) {
                        // adjust day numbers since the data array starts with 0
                        return parseInt(value)+1;
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
            },
            yaxis: {
                tickAmount: 12,
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
            },
            legend: {
                show: true,
                position: "bottom",
                horizontalAlign: "center",
                floating: true,
                offsetY: -280,
                // offsetX: -5,
                labels: {
                    colors: chartTheme.labelColors,
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
                custom: function ({ series, seriesIndex, dataPointIndex, w }) {
                    let ttp = '<h5>' + self.data[seriesIndex][dataPointIndex].t + '</h5><p>' + $.t('index.js.severityNone') + '</p>';
                    let events = self.data[seriesIndex][dataPointIndex].e;
                    events.forEach((e) => {
                        let startTime = new Date(e[0].x);
                        ttp = '<h5>' + self.data[seriesIndex][dataPointIndex].t + '</h5>';
                        // power status events
                        if (e[0] && e[0].length !== 0) {
                            let duration = secondsToHRts(e[0].d);
                            ttp += '<p>' + $.t("index.js.powerOutageDetected") + ': ' + startTime.toLocaleTimeString() +
                                    '</p><p>   - ' +  $.t("index.js.powerOutageDuration") + ': ' + duration + '</p>';
                            if(e[0].prg.length !== 0) {
                                ttp += "<p>   - " + $.t("index.js.powerOutageUPSPrg") + ": </p><ul>";
                                $.each(e[0].prg, (i, v) => {
                                    let tm = new Date(v[0]);
                                    ttp += "<li>" + outputStatusToSting(v[1]) + " (" + tm.toLocaleTimeString() + ")</li>";
                                });
                                ttp += '</ul>';
                            }
                            ttp += '<p>' + $.t("index.js.powerOutageEnded") + ': ' + (new Date(e[0].x + (e[0].d * 1000)).toLocaleTimeString()) + '</p>';
                        }
                        // battery events
                        if (e[1] && e[1].length !== 0) {
                            ttp += '<p>' + $.t("index.js.batteryStatusChanged") + ' <b>' + e[1].e + '</b> ' +
                                $.t("index.js.batteryStatusSince") + ': ' + startTime.toLocaleTimeString() + '</p>';
                        }
                    });
                    return (
                        '<div class="power-outage-tooltip">' + ttp + "</div>"
                    );
                },
            },
            // markers: {
            //     colors: undefined,
            //     strokeColors: ["rgba(225, 3, 40, 0.4)", "rgba(245, 108, 4, 0.4)"],
            // },
        };
    }
}
