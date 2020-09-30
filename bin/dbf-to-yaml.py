import os

inputPath = "GameIndex.dbf"
outputPath = "GameIndex.yaml"

if os.path.exists(outputPath):
    os.remove(outputPath)

divider = "---------------------------------------------"
gameListLine = "-- Game List"

# NOTE - can't use a nice YAML library, because we want to preserve comments and build up the yaml generatively
# just run the resulting file through a formatter!
# New Syntax:
# game:
#   serial:
#     name: Name
#     region: Region
#     compat: num
#     roundModes:
#       eeRoundMode: num
#       vuRoundMode: num
#     clampModes:
#       eeClampMode: num
#       vuClampMode: num
#     gameFixes:
#       - someGameFix
#     speedHacks:
#       - someSpeedHack
#     memcardFilters:
#       - SomeSerial,
#       - AnotherOne
#     patches:
#       - crc: "possibly empty"
#         content: |-
#           "// sure is cool"
#           "comment=Wow, what a cool patch"
#       - crc: "possibly empty"
#         content: |-
#           "// sure is cool"
#           "comment=Wow, what a cool patch"

# TODO - put an example at the top of the file

currentGame = {
    "roundModes": [],
    "clampModes": [],
    "gameFixes": [],
    "speedHacks": [],
    "memcardFilters": [],
    "memcardFiltersComment": "",
    "patches": [],
    "danglingComments": [],
}


def isRoundModeSetting(line):
    line = line.lower()
    roundModes = ["eeRoundMode", "vuRoundMode"]
    for mode in roundModes:
        if line.startswith(mode.lower()):
            return True
    return False


def isClampModeSetting(line):
    line = line.lower()
    clampModes = ["eeClampMode", "vuClampMode"]
    for mode in clampModes:
        if line.startswith(mode.lower()):
            return True
    return False


def isGameFix(line):
    line = line.lower()
    gameFixes = [
        "VuAddSubHack",
        "FpuCompareHack",
        "FpuMulHack",
        "FpuNegDivHack",
        "XgKickHack",
        "IPUWaitHack",
        "EETimingHack",
        "SkipMPEGHack",
        "OPHFLagHack",
        "DMABusyHack",
        "VIFFIFOHack",
        "VIF1StallHack",
        "GIFFIFOHack",
        "FMVinSoftwareHack",
        "ScarfaceIbitHack",
        "CrashTagTeamRacingIbit",
        "VU0KickstartHack",
    ]
    for fix in gameFixes:
        if line.startswith(fix.lower()):
            return True
    return False


def isSpeedHack(line):
    line = line.lower()
    speedHacks = ["mvuFlagSpeedHack"]
    for hack in speedHacks:
        if line.startswith(hack.lower()):
            return True
    return False


def peek_line(f):
    pos = f.tell()
    line = f.readline()
    f.seek(pos)
    return line


gamesProcessed = 0
with open(inputPath, "r") as fInput, open(outputPath, "a+") as output:
    line = fInput.readline()
    gameListStarted = False
    currentIndentAmount = 0  # spaces
    while line:
        # First Step - There is a huge header section in the GameIndex file, we need to change this to
        # valid YAML comments, it isn't part of the game database portion.
        if line.startswith(gameListLine):
            gameListStarted = True
            currentIndentAmount += 2
            output.write("# {}\n".format(line.rstrip()))
            # Consume next line early lines ahead
            line = fInput.readline()
            output.write("# {}\n".format(line.rstrip()))

        # Actually process games
        if gameListStarted:
            if line.startswith("-----"):
                # Write out all the stored up lists
                if len(currentGame["roundModes"]) > 0:
                    output.write("{}roundModes:\n".format(" " * currentIndentAmount))
                    currentIndentAmount += 2
                    for mode in currentGame["roundModes"]:
                        output.write(
                            '{}{}"\n'.format(" " * currentIndentAmount, mode.strip())
                        )
                    currentIndentAmount -= 2
                if len(currentGame["clampModes"]) > 0:
                    output.write("{}clampModes:\n".format(" " * currentIndentAmount))
                    currentIndentAmount += 2
                    for mode in currentGame["clampModes"]:
                        output.write(
                            '{}{}"\n'.format(" " * currentIndentAmount, mode.strip())
                        )
                    currentIndentAmount -= 2
                if len(currentGame["gameFixes"]) > 0:
                    output.write("{}gameFixes:\n".format(" " * currentIndentAmount))
                    currentIndentAmount += 2
                    for fix in currentGame["gameFixes"]:
                        output.write(
                            "{}- {}\n".format(" " * currentIndentAmount, fix.strip())
                        )
                    currentIndentAmount -= 2
                if len(currentGame["speedHacks"]) > 0:
                    output.write("{}speedHacks:\n".format(" " * currentIndentAmount))
                    currentIndentAmount += 2
                    for hack in currentGame["speedHacks"]:
                        output.write(
                            "{}- {}\n".format(" " * currentIndentAmount, hack.strip())
                        )
                    currentIndentAmount -= 2
                if len(currentGame["memcardFilters"]) > 0:
                    output.write(
                        "{}memcardFilters:{}\n".format(
                            " " * currentIndentAmount,
                            currentGame["memcardFiltersComment"],
                        )
                    )
                    currentIndentAmount += 2
                    for memFilter in currentGame["memcardFilters"]:
                        output.write(
                            '{}- "{}"\n'.format(
                                " " * currentIndentAmount, memFilter.strip()
                            )
                        )
                    currentIndentAmount -= 2
                if len(currentGame["patches"]) > 0:
                    output.write("{}patches:\n".format(" " * currentIndentAmount))
                    currentIndentAmount += 2
                    for patch in currentGame["patches"]:
                        if len(patch["crc"]) > 0:
                            output.write(
                                '{}- crc: "{}"\n'.format(
                                    " " * currentIndentAmount, patch["crc"]
                                )
                            )
                            output.write(
                                "{}  content: |-\n".format(" " * currentIndentAmount)
                            )
                        else:
                            output.write(
                                "{}- content: |-\n".format(" " * currentIndentAmount)
                            )
                        currentIndentAmount += 4
                        for patchLine in patch["content"]:
                            output.write(
                                '{}"{}"\n'.format(" " * currentIndentAmount, patchLine)
                            )
                        currentIndentAmount -= 4
                    currentIndentAmount -= 2
                if len(currentGame["danglingComments"]) > 0:
                    output.write(
                        "{}# Comments from old GameIndex.dbf for this Game\n".format(
                            " " * currentIndentAmount,
                        )
                    )
                    for comment in currentGame["danglingComments"]:
                        output.write(
                            "{}# {}\n".format(" " * currentIndentAmount, comment)
                        )

                gamesProcessed += 1
                currentIndentAmount -= 2
                currentGame = {
                    "roundModes": [],
                    "clampModes": [],
                    "gameFixes": [],
                    "speedHacks": [],
                    "memcardFilters": [],
                    "memcardFiltersComment": "",
                    "patches": [],
                    "danglingComments": [],
                }
                print("Processed {} Games".format(gamesProcessed))
            else:
                # To save my sanity, store the non-1-liner settings until the end, as they technically can be given in any order
                # we can't backtrack lines!
                # !! CLOSE YOUR EYES !!
                if isRoundModeSetting(line):
                    lineParts = line.split("=")
                    settingName = lineParts[0].strip()
                    setting = lineParts[1].strip()
                    comment = ""
                    if "//" in setting:
                        comment = " # " + setting.split("//")[1]
                        setting = setting.split("//")[0]
                    elif "--" in setting:
                        comment = " # " + setting.split("--")[1]
                        setting = setting.split("--")[0]
                    currentGame["roundModes"].append(
                        "{}: {}{}".format(settingName, setting, comment)
                    )
                elif isClampModeSetting(line):
                    lineParts = line.split("=")
                    settingName = lineParts[0].strip()
                    setting = lineParts[1].strip()
                    comment = ""
                    if "//" in setting:
                        comment = " # " + setting.split("//")[1]
                        setting = setting.split("//")[0]
                    elif "--" in setting:
                        comment = " # " + setting.split("--")[1]
                        setting = setting.split("--")[0]
                    currentGame["clampModes"].append(
                        "{}: {}{}".format(settingName, setting, comment)
                    )
                elif isGameFix(line):
                    lineParts = line.split("=")
                    settingName = lineParts[0].strip()
                    setting = lineParts[1].strip()
                    comment = ""
                    if "//" in setting:
                        comment = " # " + setting.split("//")[1]
                        setting = setting.split("//")[0]
                    elif "--" in setting:
                        comment = " # " + setting.split("--")[1]
                        setting = setting.split("--")[0]
                    currentGame["gameFixes"].append(
                        '"{}"{}'.format(settingName, comment)
                    )
                elif isSpeedHack(line):
                    lineParts = line.split("=")
                    settingName = lineParts[0].strip()
                    setting = lineParts[1].strip()
                    comment = ""
                    if "//" in setting:
                        comment = " # " + setting.split("//")[1]
                        setting = setting.split("//")[0]
                    elif "--" in setting:
                        comment = " # " + setting.split("--")[1]
                        setting = setting.split("--")[0]
                    currentGame["speedHacks"].append(
                        '"{}"{}'.format(settingName, comment)
                    )
                elif line.lower().startswith("//") or line.lower().startswith("--"):
                    # The current file has comments preceeding only the memcard filters specifically, we I'll handle that edge-case
                    if peek_line(fInput).lower().startswith("memcardfilter"):
                        currentGame["memcardFiltersComment"] = " # " + (
                            line.replace("//", "").replace("--", "").rstrip()
                        )
                    # the rest are generic comments about the game, or disabled options which aren't worth handling that edge-case
                    else:
                        currentGame["danglingComments"].append(
                            line.replace("//", "").replace("--", "").rstrip()
                        )
                elif line.lower().startswith("serial"):
                    serial = line.split("=")[1].strip()
                    comment = ""
                    if "//" in serial:
                        comment = " # " + serial.split("//")[1]
                        serial = serial.split("//")[0]
                    elif "--" in serial:
                        comment = " # " + serial.split("--")[1]
                        serial = serial.split("--")[0]
                    output.write(
                        "{}{}:\n".format(
                            " " * currentIndentAmount,
                            serial.replace('"', "'").rstrip(),
                            comment,
                        )
                    )
                    currentIndentAmount += 2
                # !! I MEAN IT, THIS IS DISGUSTING !!
                elif line.lower().startswith("name"):
                    name = line.split("=")[1].strip()
                    comment = ""
                    if "//" in name:
                        comment = " # " + name.split("//")[1]
                        name = name.split("//")[0]
                    elif "--" in name:
                        comment = " # " + name.split("--")[1]
                        name = name.split("--")[0]
                    output.write(
                        '{}name: "{}"{}\n'.format(
                            " " * currentIndentAmount,
                            name.replace('"', "'").rstrip(),
                            comment,
                        )
                    )
                elif line.lower().startswith("region"):
                    region = line.split("=")[1].strip()
                    comment = ""
                    if "//" in region:
                        comment = " # " + region.split("//")[1]
                        region = region.split("//")[0]
                    elif "--" in region:
                        comment = " # " + region.split("--")[1]
                        region = region.split("--")[0]
                    output.write(
                        '{}region: "{}"\n'.format(
                            " " * currentIndentAmount,
                            region.replace('"', "'").rstrip(),
                            comment,
                        )
                    )
                elif line.lower().startswith("compat"):
                    compat = line.split("=")[1].strip()
                    comment = ""
                    if "//" in compat:
                        comment = " # " + compat.split("//")[1]
                        compat = compat.split("//")[0]
                    elif "--" in compat:
                        comment = " # " + compat.split("--")[1]
                        compat = compat.split("--")[0]
                    output.write(
                        "{}compat: {}\n".format(
                            " " * currentIndentAmount,
                            compat.replace('"', "'").rstrip(),
                            comment,
                        )
                    )
                elif line.lower().startswith("memcardfilter"):
                    filters = line.split("=")[1].strip()
                    comment = ""
                    if "//" in filters:
                        comment = " # " + filters.split("//")[1]
                        filters = filters.split("//")[0]
                    elif "--" in filters:
                        comment = " # " + filters.split("--")[1]
                        filters = filters.split("--")[0]
                    currentGame["memcardFilters"] = filters.split("/")
                    if len(currentGame["memcardFiltersComment"]) > 0:
                        currentGame["memcardFiltersComment"] = (
                            currentGame["memcardFiltersComment"] + " " + comment
                        ).rstrip()
                    else:
                        currentGame["memcardFiltersComment"] = comment
                # Patch Handling
                elif line.lower().startswith("[patches"):
                    patch = {"crc": "", "content": []}
                    # Patches can or cannot have a CRC embeded within the tag
                    if "=" in line.lower():
                        patch["crc"] = line.split("=")[1].strip().replace("]", "")
                    # Since they are guaranteed to be in groups, we can parse until the end of the patch
                    line = fInput.readline()
                    while not line.lower().startswith("[/patches"):
                        patch["content"].append(line.replace('"', "'").strip())
                        line = fInput.readline()
                    currentGame["patches"].append(patch)

        # !! You can open your eyes now !!
        else:
            if len(line.rstrip()) != 0:
                output.write("# {}\n".format(line.rstrip()))
            else:
                output.write("\n".format(line.rstrip()))

        # Next Line!
        line = fInput.readline()
