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

const isProduction = process.env.NODE_ENV == "production";

const config = {
    entry: {
        c: "./src/common.js",
        i: {
            import: "./src/index.js",
            dependOn: "c",
        },
        s: {
            import: "./src/setup.js",
            dependOn: "c",
        },
        l: {
            import: "./src/login.js",
            dependOn: "c",
        },
        e: {
            import: "./src/error.js",
            dependOn: "c",
        },
    },
    devtool: !isProduction ? "source-map" : false,
    output: {
        // Clean the output directory before emit.
        clean: false,
        path: path.join(__dirname, "./data/"),
        filename: "[name].js",
        // publicPath: "",
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
                extractComments: "all",
                test: /\.js(\?.*)?$/i,
                // terserOptions options will be passed to uglify-js
                // See: https://github.com/mishoo/UglifyJS#minify-options
                terserOptions: {
                    ie: true,
                    webkit: true,
                    v8: true,
                },
            }),
        ],
    },
    plugins: [
        new webpack.ProvidePlugin({
            $: "jquery",
            jQuery: "jquery",
        }),
        // see: https://github.com/jantimon/html-webpack-plugin
        new HtmlWebpackPlugin({
            title: "tinyUPS dashboard",
            filename: "i.htm",
            template: "src/index.html",
            minify: true,
            chunks: ["c", "i"],
        }),
        new HtmlWebpackPlugin({
            title: "tinyUPS setup",
            filename: "s.htm",
            template: "src/setup.html",
            minify: true,
            chunks: ["c", "s"],
        }),
        new HtmlWebpackPlugin({
            title: "tinyUPS sign-in",
            filename: "l.htm",
            template: "src/login.html",
            minify: true,
            chunks: ["c", "l"],
        }),
        new HtmlWebpackPlugin({
            title: "404 Not Found",
            filename: "e.htm",
            template: "src/error.html",
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
                root: "./data",
                test: [
                    {
                        folder: ".",
                        method: (absoluteItemPath) => {
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
                baseDir: ["data"],
            },
        }),
        // Add your plugins here
        // Learn more about plugins from https://webpack.js.org/configuration/plugins/
    ],
    resolve: {
        alias: {
            // core: path.join(__dirname, "core"),
            components: path.join(__dirname, "./components/"),
        },
        modules: [
            // Tell webpack what directories should be searched when resolving modules
            path.join(__dirname, "./node_modules/"),
        ],
    },
    module: {
        rules: [
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
                                    require("tailwindcss/nesting"),
                                    require("postcss-nesting"),
                                    require("tailwindcss"),
                                    require("autoprefixer"),
                                    // require('postcss-import'),
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
};

module.exports = () => {
    if (isProduction) {
        config.mode = "production";
    } else {
        config.mode = "development";
    }
    return config;
};
