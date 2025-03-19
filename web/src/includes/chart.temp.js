// See: https://apexcharts.com/docs
import ApexCharts from 'apexcharts';
// themes
import { lineChartLightColors, lineChartDarkColors } from './chart.theme';

/**
 * Common options
 */
const tempChartOptions = theme => {
    let chartTheme = theme === 'light' ? lineChartLightColors : lineChartDarkColors;
    return {
        series: [
            {
                name: $.t('index.js.opchDeviceTemp'),
                type: 'line',
                data: []
            },
            {
                name: $.t('index.js.opchBatteryTemp'),
                type: 'line',
                data: []
            }
        ],
        colors: ['rgba(245, 109, 4, 1)', 'rgba(225, 3, 40, 1)'],
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
                text: $.t('index.js.opchTitleTemp'),
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
            strokeColors: ['rgba(225, 3, 40, 0.4)', 'rgba(245, 108, 4, 0.4)']
        }
    };
};

export default class tempChart {
    constructor(selector) {
        // history length is about 2:50 min, see xaxis range
        this.maxLength = 1000;
        this.dataSt = [];
        this.dataBt = [];
        this.chart = new ApexCharts(
            document.getElementById(selector),
            tempChartOptions(window.settings.get('theme', 'light'))
        );
        this.chart.render();
        // theme change
        document.addEventListener('themeChange', e => {
            this.chart.updateOptions(tempChartOptions(e.detail.theme));
        });
    }

    update(st, bt) {
        this.dataSt.push(st);
        if (this.dataSt.length > this.maxLength) {
            this.dataSt.slice(1, this.maxLength + 1);
        }
        this.dataBt.push(bt);
        if (this.dataBt.length > this.maxLength) {
            this.dataBt.slice(1, this.maxLength + 1);
        }

        this.chart.updateSeries([
            {
                data: this.dataSt
            },
            {
                data: this.dataBt
            }
        ]);
    }
}
