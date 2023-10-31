import pathlib

# print(f">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MPYDIR: {MPYDIR}")
# include("$(MPY_DIR)/extmod/uasyncio")

include("./modules/uasyncio")

modules = list(pathlib.Path("./modules").glob("*.py"))

if any(modules):
    for m in modules:
        print(f"INCLUDING {m} as a module.")
        path, file = m.parts
        print(f"Including {m.stem} as a module.")
        freeze_as_mpy(path, file)
