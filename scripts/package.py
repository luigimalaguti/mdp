import shutil
import sys
from pathlib import Path

PACKAGES_PATH = Path("packages")
BOILERPLATE_PATH = PACKAGES_PATH / Path("boilerplate")


EXCLUDE = {
}


def should_exclude(path: Path) -> bool:
    return path.name in EXCLUDE


def copy_boilerplate(destination: Path) -> None:
    if not BOILERPLATE_PATH.exists():
        raise FileNotFoundError(f"Boilerplate not found: {BOILERPLATE_PATH}")

    if destination.exists():
        raise FileExistsError(f"Destination directory already exists: {destination}")

    for item in BOILERPLATE_PATH.rglob("*"):
        if any(part in EXCLUDE for part in item.parts):
            continue

        relative_path = item.relative_to(BOILERPLATE_PATH)
        target_path = destination / relative_path

        if item.is_dir():
            target_path.mkdir(parents=True, exist_ok=True)
        else:
            target_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(item, target_path)


def main(project_name: str) -> None:
    destination = PACKAGES_PATH / project_name

    copy_boilerplate(destination)
    print(f"✔ Boilerplate copied in: {destination}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python package.py <project_name>")
        sys.exit(1)

    main(sys.argv[1])
