from os.path import isdir, join

Import("env")
#print(env.Dump())
board = env.BoardConfig()

PROJ_DIR            = env.get("PROJECT_DIR") 
BUILD_DIR           = env.get("PROJECT_BUILD_DIR")
PIOENV              = env.get("PIOENV")

SRC_DIR             = "src"
FREERUN_TIMER_DIR   = join(SRC_DIR, "freerun_timer")
SHELL_DIR           = join(SRC_DIR, "shell")
SOUND_DIR           = join(SRC_DIR, "sound")
SOUND_APP_DIR       = join(SOUND_DIR, "app")
SOUND_MIDI_DIR      = join(SOUND_DIR, "midi")
SOUND_COMPONENT_DIR = join(SOUND_DIR, "components")
USBD_DIR            = join(SRC_DIR, "usbd")
USBD_APP_DIR        = join(USBD_DIR, "app")
PERIPH_DRIV_DIR     = join(SRC_DIR, "GD32VF103_Firmware_Library_V1.0.1", "Firmware", "GD32VF103_standard_peripheral")
USBFS_DRIV_DIR      = join(SRC_DIR, "GD32VF103_Firmware_Library_V1.0.1", "Firmware", "GD32VF103_usbfs_driver") 
RISCV_DRIV_DIR      = join(SRC_DIR, "GD32VF103_Firmware_Library_V1.0.1", "Firmware", "RISCV", "drivers")


env.Append(

    CPPPATH = [
        join(PROJ_DIR, SRC_DIR),
        join(PROJ_DIR, FREERUN_TIMER_DIR),
        join(PROJ_DIR, SHELL_DIR),
        join(PROJ_DIR, SOUND_DIR),
        join(PROJ_DIR, SOUND_APP_DIR),
        join(PROJ_DIR, SOUND_APP_DIR, "single_ymf825"),
        join(PROJ_DIR, SOUND_MIDI_DIR),
        join(PROJ_DIR, SOUND_COMPONENT_DIR),
        join(PROJ_DIR, SOUND_COMPONENT_DIR, "ymf825"),
        join(PROJ_DIR, USBD_DIR),
        join(PROJ_DIR, USBD_APP_DIR),
        join(PROJ_DIR, PERIPH_DRIV_DIR),
        join(PROJ_DIR, PERIPH_DRIV_DIR, "Include"),
        join(PROJ_DIR, USBFS_DRIV_DIR),
        join(PROJ_DIR, USBFS_DRIV_DIR, "Include"),
        join(PROJ_DIR, RISCV_DRIV_DIR),
    ]
)

env.Replace(

    ASFLAGS = ["-x", "assembler-with-cpp"],

    CCFLAGS=[
        "-Og",
        "-g",
        "-Wall", 
        "-march=%s" % board.get("build.march"),
        "-mabi=%s" % board.get("build.mabi"),
        #"-mcmodel=%s" % board.get("build.mcmodel"),
        "-fshort-wchar",
        "-ffunction-sections",
        "-fdata-sections",
        "-fstack-usage",
        "-msmall-data-limit=8",
    ],

    LINKFLAGS=[
        "-march=%s" % board.get("build.march"),
        "-mabi=%s" % board.get("build.mabi"),
        #"-mcmodel=%s" % board.get("build.mcmodel"),
        "-msmall-data-limit=8",
        "-nostartfiles",
        "-Wl,--gc-sections",
        #"-specs=nano.specs"
        #"-Wl,-Map=firmware.map",
    ],

    LIBS=[
        "c_nano",
        "m" # libm
    ],

    LDSCRIPT_PATH = [
        join(PROJ_DIR, "GD32VF103xB.lds") 
    ]
)


# copy CCFLAGS to ASFLAGS (-x assembler-with-cpp mode)
env.Append(ASFLAGS=env.get("CCFLAGS", [])[:])
