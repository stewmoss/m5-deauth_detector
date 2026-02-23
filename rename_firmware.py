Import("env")
import shutil
import os

# Read version from platformio.ini [env] custom_fw_version option
version = env.GetProjectOption("custom_fw_version", "dev")

# Inject as a C preprocessor macro so firmware code can reference FIRMWARE_VERSION
env.Append(CPPDEFINES=[("FIRMWARE_VERSION", env.StringifyMacro(version))])

def copy_firmware(source, target, env):
    """Post-build action: copy firmware.bin to deauthdetector-{version}.bin"""
    build_dir = env.subst("$BUILD_DIR")
    src = os.path.join(build_dir, "firmware.bin")
    dest = os.path.join(build_dir, f"..\..\..\deauthdetector-{version}.bin")

    if os.path.isfile(src):
        shutil.copy2(src, dest)
        print(f"\n*** Output: {dest}\n")
    else:
        print(f"\n*** copy_firmware: source not found: {src}\n")

env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)
