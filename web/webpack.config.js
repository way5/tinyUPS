// Generated using webpack-cli https://github.com/webpack/webpack-cli
const path = require('path');
const BrowserSyncPlugin = require('browser-sync-webpack-plugin');
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const CompressionPlugin = require("compression-webpack-plugin");
const HtmlWebpackPlugin = require("html-webpack-plugin");
// See: https://www.npmjs.com/package/imagemin-webpack-plugin
// const ImageminPlugin = require('imagemin-webpack-plugin').default;
// const imageminMozjpeg = require('imagemin-mozjpeg');
const UglifyJsPlugin = require("uglifyjs-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const RemovePlugin = require('remove-files-webpack-plugin');

const isProduction = process.env.NODE_ENV == "production";

const config = {
    entry: {
        app: "./src/script.js",
    },
    output: {
        // Clean the output directory before emit.
        clean: false,
        path: path.resolve(__dirname, "data"),
        filename: "[name].js",
        publicPath: "",
    },
    plugins: [
        // see: https://github.com/jantimon/html-webpack-plugin
        new HtmlWebpackPlugin({
            title: "tinyUPS dashboard",
            filename: "i.htm",
            template: "src/index.html",
            minify: true,
        }),
        new HtmlWebpackPlugin({
            title: "tinyUPS setup",
            filename: "s.htm",
            template: "src/setup.html",
            minify: true,
        }),
        new HtmlWebpackPlugin({
            title: "tinyUPS sign-in",
            filename: "l.htm",
            template: "src/signin.html",
            minify: true,
        }),
        new HtmlWebpackPlugin({
            title: "404 Not Found",
            filename: "e.htm",
            template: "src/error.html",
            minify: true,
        }),
        // See: https://github.com/webpack-contrib/mini-css-extract-plugin
        new MiniCssExtractPlugin(),
        // See: https://webpack.js.org/plugins/compression-webpack-plugin/
        new CompressionPlugin({
            test: /\.(js|css|svg|ico|htm)(\?.*)?$/i,
            algorithm: "gzip",
            compressionOptions: { level: 9 },
            filename: (isProduction ? "[base]" : "[base].gz"),
            deleteOriginalAssets: (isProduction ? true : false)
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
                root: './data',
                test: [
                    {
                        folder: '.',
                        method: (absoluteItemPath) => {
                            return new RegExp(/\.gz$/, 'm').test(absoluteItemPath);
                        },
                        recursive: true
                    }
                ],
                emulate: !isProduction,
                log: false,
                trash: false,
            }
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
            host: 'localhost',
            port: 8880,
            server: { 
                baseDir: ['data'] 
            },
        }),
        // Add your plugins here
        // Learn more about plugins from https://webpack.js.org/configuration/plugins/
    ],
    resolve: {
        alias: {
            // core: path.join(__dirname, "core"),
        },
        modules: [
            // Tell webpack what directories should be searched when resolving modules
            path.resolve("./node_modules"),
        ],
    },
    module: {
        rules: [
            {
                test: /\.(jpe?g|png|gif|ico|svg)$/i,
                type: "asset/resource",
                generator: {
                    filename: "[name][ext]"
                }
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
                                    require('tailwindcss/nesting'),
                                    require('postcss-nesting'),
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
    optimization: {
        minimize: isProduction,
        minimizer: [
            // See: https://github.com/webpack-contrib/css-minimizer-webpack-plugin
            new CssMinimizerPlugin({
                minimizerOptions: {
                    preset: [
                        "default",
                        {
                            discardComments: { removeAll: true },
                        },
                    ],
                },
            }),
            new UglifyJsPlugin({
                test: /\.js(\?.*)?$/i,
                cache: false,
                // Extract comments into a filename: ...
                extractComments: false,
                uglifyOptions: {
                    mangle: {
                        // Pass true to mangle names visible in scopes where eval or with are used.
                        // eval: false,
                        // Pass true to mangle names declared in the top level scope.
                        toplevel: true,
                    },
                    toplevel: true,
                    output: {
                        comments: false,
                        beautify: false,
                        // preamble: "/* uglified */"
                    },
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
