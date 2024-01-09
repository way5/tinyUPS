// Generated using webpack-cli https://github.com/webpack/webpack-cli

const path = require("path");
const webpack = require("webpack");
const BrowserSyncPlugin = require("browser-sync-webpack-plugin");
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const CompressionPlugin = require("compression-webpack-plugin");
const HtmlWebpackPlugin = require("html-webpack-plugin");
// See: https://www.npmjs.com/package/imagemin-webpack-plugin
// const ImageminPlugin = require('imagemin-webpack-plugin').default;
// const imageminMozjpeg = require('imagemin-mozjpeg');
const TerserWPPlugin = require("terser-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const RemovePlugin = require("remove-files-webpack-plugin");
// const HTMLInlineCSSWebpackPlugin = require("html-inline-css-webpack-plugin");
// const InlineChunkHtmlPlugin = require('inline-chunk-html-plugin');

const isProduction = process.env.NODE_ENV === "production";
const webDataDir = path.resolve(__dirname, "web/data");

const config = {
    entry: {
        "c": path.resolve(__dirname, "web/src/common.js"),
        "i": {
            import: path.resolve(__dirname, "web/src/index.js"),
            dependOn: "c",
        },
        "s": {
            import: path.resolve(__dirname, "web/src/setup.js"),
            dependOn: "c",
        },
        "l": {
            import: path.resolve(__dirname, "web/src/login.js"),
            dependOn: "c",
        },
        "e": {
            import: path.resolve(__dirname, "web/src/error.js"),
            dependOn: "c",
        },
    },
    devtool: !isProduction ? "source-map" : false,
    output: {
        // Clean the output directory before emit.
        clean: true,
        path: webDataDir,
        filename: "[name].js"
    },
    resolve: {
        // alias: {
        //     components: path.resolve(__dirname, "components/"),
        // },
        modules: [
            // Tell webpack what directories should be searched when resolving modules
            path.resolve(__dirname, "node_modules/"),
        ],
        // extensions: [
        //     '.js', '.css', '.scss', '...'
        // ]
    },
    plugins: [
        new webpack.ProvidePlugin({
            $: "jquery",
            jQuery: "jquery",
        }),
        // see: https://github.com/jantimon/html-webpack-plugin
        new HtmlWebpackPlugin({
            filename: "i.htm",
            template: "web/src/index.html",
            minify: true,
            chunks: ["c", "i"],
        }),
        new HtmlWebpackPlugin({
            filename: "s.htm",
            template: "web/src/setup.html",
            minify: true,
            chunks: ["c", "s"],
        }),
        new HtmlWebpackPlugin({
            filename: "l.htm",
            template: "web/src/login.html",
            minify: true,
            chunks: ["c", "l"],
        }),
        new HtmlWebpackPlugin({
            filename: "e.htm",
            template: "web/src/error.html",
            minify: true,
            chunks: ["c", "e"],
        }),
        // See: https://github.com/webpack-contrib/mini-css-extract-plugin
        new MiniCssExtractPlugin(),
        // See: https://webpack.js.org/plugins/compression-webpack-plugin/
        new CompressionPlugin({
            test: /\.(js|css|svg|ico|htm)$/i,
            algorithm: "gzip",
            compressionOptions: {
                level: 9,
            },
            minRatio: 1,
            threshold: 0,
            filename: isProduction ? "[base]" : "[base].gz",
            deleteOriginalAssets: isProduction ? true : false,
        }),
        // (isProduction ? new InlineChunkHtmlPlugin(HtmlWebpackPlugin, [/(i.ch|s.ch|l.ch|e.ch)/]) : null),
        // (isProduction ? new HTMLInlineCSSWebpackPlugin.default() : null),
        // See: https://github.com/Amaimersion/remove-files-webpack-plugin
        new RemovePlugin({
            before: {
                // parameters for "before normal compilation" stage.
            },
            watch: {
                // parameters for "before watch compilation" stage.
            },
            after: {
                // parameters for "after normal and watch compilation" stage.
                root: webDataDir,
                test: [
                    {
                        folder: ".",
                        method: (absoluteItemPath) => {
                            // return new RegExp(/\.(gz|map|ch.js|ch.css)$/, "m").test(
                            return new RegExp(/\.(gz|map)$/, "m").test(
                                absoluteItemPath
                            );
                        },
                        recursive: true,
                    },
                ],
                emulate: !isProduction,
                log: false,
                trash: false,
            },
        }),
        // new ImageminPlugin({
        //     test: /.(jpe?g|png|gif|svg)$/i,
        //     disable: !isProduction,
        //     pngquant: {
        //       quality: '95-100'
        //     },
        //     plugins: [
        //         imageminMozjpeg({
        //           quality: 100,
        //           progressive: true
        //         })
        //     ]
        //  }),
        new BrowserSyncPlugin({
            host: "localhost",
            port: 8880,
            server: {
                baseDir: [
                    webDataDir
                ],
            },
        }),
        // Add your plugins here
        // Learn more about plugins from https://webpack.js.org/configuration/plugins/
    ],
    module: {
        rules: [
            {
                test: /\.(jsx?|tsx?)$/i,
                exclude: [
                    /node_modules/,
                    /web\/data/
                ],
                use: [{
                    // supporting old browsers
                    loader: 'esbuild-loader',
                    options: {
                        target: 'es2015',
                    },
                }],
            },
            {
                test: /\.(jpe?g|png|gif|ico|svg)$/i,
                type: "asset/resource",
                generator: {
                    filename: "[name][ext]",
                },
            },
            {
                test: /\.html$/i,
                loader: "html-loader",
            },
            {
                test: /\.((c|sa|sc)ss)$/i,
                use: [
                    MiniCssExtractPlugin.loader,
                    "css-loader",
                    {
                        loader: "postcss-loader",
                        options: {
                            postcssOptions: {
                                plugins: [
                                    require("postcss-nesting"),
                                    require("tailwindcss/nesting"),
                                    require("tailwindcss"),
                                    require("autoprefixer")
                                ],
                            },
                        },
                    },
                ],
            },
            {
                test: /\.(eot|ttf|woff|woff2)$/i,
                type: "asset",
            },
            // Add your rules for custom modules here
            // Learn more about loaders from https://webpack.js.org/loaders/
        ],
    },
    optimization: {
        minimize: isProduction,
        minimizer: [
            // See: https://github.com/webpack-contrib/css-minimizer-webpack-plugin
            new CssMinimizerPlugin({
                minimizerOptions: {
                    preset: [
                        "default",
                        {
                            discardComments: {
                                removeAll: true
                            },
                        },
                    ],
                },
            }),
            new TerserWPPlugin({
                minify: TerserWPPlugin.uglifyJsMinify,
                extractComments: false,
                test: /\.js(\?.*)?$/i,
                // terserOptions options will be passed to uglify-js
                // See: https://github.com/mishoo/UglifyJS#minify-options
                terserOptions: {
                    ie: true,
                    webkit: true,
                    v8: true,
                    annotations: false
                },
            }),
        ],
    },
};

module.exports = () => {
    if (isProduction) {
        config.mode = "production";
    } else {
        config.mode = "development";
    }
    return config;
};
