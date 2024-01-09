Import("env")
import os
import shutil

root = os.getcwd()

def before_buildfs(source, target, env):
    env.Execute("yarn build:prod")

env.AddPreAction("$BUILD_DIR/fatfs.bin", before_buildfs)

def after_buildfs(source, target, env):
    ## source is web/data directory
    path = target[0].get_abspath()
    shutil.copyfile(path, root + "/filesystem.bin")

env.AddPostAction("$BUILD_DIR/fatfs.bin", after_buildfs)

def after_build(source, target, env):
    # print(source[0].get_path())
    path = source[0].get_abspath()
    # print(path)
    shutil.copyfile(os.path.splitext(path)[0] + ".bin", root + "/firmware.bin")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)

def after_cleanup(source, target, env):
    print("Clean up root directory...")
    os.remove(root + "/filesystem.bin")
    os.remove(root + "/firmware.bin")

env.AddPostAction("clean", after_cleanup)
env.AddPostAction("fullclean", after_cleanup)