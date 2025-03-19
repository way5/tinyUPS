// See: https://apexcharts.com/docs
import ApexCharts from 'apexcharts';
// themes
import { lineChartLightColors, lineChartDarkColors } from './chart.theme';

/**
 * Common options
 */
const ramChartOptions = theme => {
    let chartTheme = theme === 'light' ? lineChartLightColors : lineChartDarkColors;
    return {
        series: [
            {
                name: $.t('index.js.opchTotalRam'),
                type: 'line',
                data: []
            },
            {
                name: $.t('index.js.opchRAMmaxFreeBlk'),
                type: 'line',
                data: []
            }
        ],
        colors: ['rgba(0, 135, 22, 1)', 'rgba(1, 168, 29, 1)'],
        chart: {
            height: 250,
            offsetY: -20,
            type: 'line',
            animations: {
                enabled: true,
                easing: 'linear',
                dynamicAnimation: {
                    speed: 1000
                }
            },
            dropShadow: {
                enabled: true,
                enabledSeries: [0],
                top: -2,
                left: 2,
                blur: 5,
                opacity: 0.06
            },
            toolbar: {
                show: false
            },
            zoom: {
                enabled: true,
                autoScaleYaxis: true
            }
        },
        // fill: {
        //     type: 'gradient',
        //     gradient: {
        //         type: 'vertical',
        //         shadeIntensity: 0,
        //         opacityFrom: 0.3,
        //         opacityTo: 0.0,
        //         stops: [0, 100]
        //     }
        // },
        dataLabels: {
            enabled: false
        },
        stroke: {
            curve: 'smooth',
            width: 3
        },
        // title: {
        //     text: "Dynamic Updating Chart",
        //     align: "left",
        // },
        grid: {
            xaxis: {
                lines: {
                    show: true
                }
            },
            yaxis: {
                lines: {
                    show: true
                }
            },
            borderColor: chartTheme.gridBorderColor,
            row: {
                colors: chartTheme.gridRowColors
                // opacity: 0.5,
            }
        },
        markers: {
            size: 0,
            strokeWidth: 3,
            strokeOpacity: 1,
            fillOpacity: 1,
            hover: {
                size: 6
            }
        },
        xaxis: {
            type: 'datetime',
            range: 300000,
            labels: {
                datetimeUTC: false,
                style: {
                    colors: chartTheme.xaxis.labelColors
                }
            },
            axisBorder: {
                color: chartTheme.xaxis.axisBorderColor
            },
            axisTicks: {
                color: chartTheme.xaxis.axisTicksColor
            },
            title: {
                style: {
                    color: chartTheme.xaxis.titleColor
                }
            },
            crosshairs: {
                stroke: {
                    color: chartTheme.xaxis.crosshairsStrokeColor
                },
                fill: {
                    color: chartTheme.xaxis.crosshairsFillColor
                }
            }
        },
        yaxis: {
            // min: 5,
            // max: 40,
            title: {
                text: $.t('index.js.opchTitleRAM'),
                style: {
                    color: chartTheme.yaxis.titleColor
                }
            },
            labels: {
                style: {
                    colors: chartTheme.yaxis.labelColors
                }
            },
            axisBorder: {
                show: true,
                color: chartTheme.yaxis.axisBorderColor
            },
            axisTicks: {
                show: true,
                color: chartTheme.yaxis.axisTicksColor
            },
            crosshairs: {
                stroke: {
                    color: chartTheme.yaxis.crosshairsStrokeColor
                },
                fill: {
                    color: chartTheme.yaxis.crosshairsFillColor
                }
            }
        },
        legend: {
            show: true,
            position: 'bottom',
            horizontalAlign: 'center',
            floating: true,
            offsetY: 10,
            // offsetX: -5,
            labels: {
                colors: chartTheme.labelColors
            }
        },
        tooltip: {
            // enabled: true,
            theme: theme,
            // fillSeriesColor: false,
            x: {
                show: false,
                format: 'MMM dd, HH:mm:ss'
            }
        },
        markers: {
            colors: undefined,
            strokeColors: ['rgba(1, 168, 29, 1)', 'rgba(0, 135, 22, 1)']
        }
    };
};

export default class ramChart {
    constructor(selector) {
        // history length is about 2:50 min, see xaxis range
        this.maxLength = 1000;
        this.dataR = [];
        this.dataR3 = [];
        this.chart = new ApexCharts(
            document.getElementById(selector),
            ramChartOptions(window.settings.get('theme', 'light'))
        );
        this.chart.render();
        // theme change
        document.addEventListener('themeChange', e => {
            this.chart.updateOptions(ramChartOptions(e.detail.theme));
        });
    }

    update(r, r3) {
        this.dataR.push(r);
        if (this.dataR.length > this.maxLength) {
            this.dataR.slice(1, this.maxLength + 1);
        }
        this.dataR3.push(r3);
        if (this.dataR3.length > this.maxLength) {
            this.dataR3.slice(1, this.maxLength + 1);
        }

        this.chart.updateSeries([
            {
                data: this.dataR
            },
            {
                data: this.dataR3
            }
        ]);
    }
}
