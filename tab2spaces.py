import glob


def tabs_to_spaces(file_ext):
    for filepath in glob.iglob(f"**/*.{file_ext}", recursive=True):
        with open(filepath, "r", encoding="utf8") as f:
            file = f.read()

        newfile = file.replace("\t", " " * 4)

        with open(filepath, "w", encoding="utf8") as f:
            f.write(newfile)

tabs_to_spaces("cpp")
tabs_to_spaces("hpp")
tabs_to_spaces("json")
