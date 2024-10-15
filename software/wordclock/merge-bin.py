#!/usr/bin/python3

# Adds PlatformIO post-processing to merge all the ESP flash images into a single image.
# Source: DavidSchinazi https://github.com/platformio/platform-espressif32/issues/1078#issuecomment-2219671743
import os

Import("env", "projenv")

board_config = env.BoardConfig()
firmware_bin = "${BUILD_DIR}/${PROGNAME}.bin"
merged_bin = os.path.join(env.get('BUILD_DIR'), env.get('PROGNAME') + "_merged.bin")


def merge_bin_action(source, target, env):
    flash_images = [
        *map(lambda path: f'"{path}"', env.Flatten(env.get("FLASH_EXTRA_IMAGES", []))),
        "$ESP32_APP_OFFSET",
        '"' + source[0].get_abspath() + '"',
    ]
    merge_cmd = " ".join(
        [
            '"$PYTHONEXE"',
            '"$OBJCOPY"',
            "--chip",
            board_config.get("build.mcu", "esp32"),
            "merge_bin",
            "-o",
            "\"" + merged_bin + "\"",
            "--flash_mode",
            board_config.get("build.flash_mode", "dio"),
            "--flash_freq",
            "${__get_board_f_flash(__env__)}",
            "--flash_size",
            board_config.get("upload.flash_size", "4MB"),
            *flash_images,
        ]
    )
    env.Execute(merge_cmd)


env.AddCustomTarget(
    name="mergebin",
    dependencies=firmware_bin,
    actions=merge_bin_action,
    title="Merge binary",
    description="Build combined image",
    always_build=True,
)