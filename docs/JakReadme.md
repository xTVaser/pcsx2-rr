Allows you to access Debug Mode in most of the publicly available Jak and Daxter builds.

This is a modified PCSX2 build that more closely emulates a PS2 TOOL (devkit), allowing the games' Debug Mode to load. It uses pnach files to patch the game (located in cheats folder). This build will work for any game / setup that requires a devkit-like setup. It is not specific to just 1 game. Support has recently been expanded to PAL/NTSC-J versions of the Jak and Daxter Trilogy, as well as several prototypes and demos.

Usage:

    Cheats must be enabled for Debug Mode to work. You can enable cheats here: PCSX2 -> Game Settings -> Enable Cheats

    For retail and demo builds, just select the ISO and boot. You can do it with or without fast boot, but doing without is recommended - the game will be set to Japanese otherwise.
    For internal/preview/review builds, you must unpack the ISO of the build. Select the ISO, and then choose Run ELF:
        For Jak II, you must choose FIREWIRE.IRX from the DRIVERS folder of the build.
        For Jak 3, you must choose DISKINFO.BIN from the root directory of the build.
    There will be significant visual glitches in these builds until you enable manual game-fixes:
        Go to PCSX2 -> Emulation Settings -> Game Fixes
        Check "Enable manual game fixes" and then check "VU0 Kickstart to avoid sync problems with VU1".

Troubleshooting:

    If a build doesn't boot into Debug Mode, then make sure that you followed the Usage section properly.
    Graphical issues? Double check that "VU0 Kickstart to avoid sync problems with VU1" is enabled in PCSX2 -> Emulation Settings -> Game Fixes
    When loading the game, you should see logs similar to the following:

Found Cheats file: '9184AAF1.pnach'
comment: Enables Developer/Debug Mode - Credit to water111 for discovering / documenting the required ELF edits
Loaded 5 Cheats from '9184AAF1.pnach' at 'C:\Users\xtvas\Repositories\pcsx2\bin\cheats'
Overall 5 Cheats loaded
