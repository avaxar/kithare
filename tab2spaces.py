import glob


def tabs_to_spaces(file_ext):
    for filepath in glob.iglob(f"**/*.{file_ext}", recursive=True):
        with open(filepath, "r", encoding="utf-8") as f:
            file = f.read()

        newfile = file.replace("\t", " " * 4)

        with open(filepath, "w", encoding="utf-8") as f:
            f.write(newfile)


for ext in ["cpp", "hpp", "json", "py"]:
    tabs_to_spaces(ext)
