import shutil
from pathlib import Path

def deploy():
    src = Path("assets/tv_db")
    dest = Path("applications/tv_scanner/assets/tv_db")
    dest.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src, dest, dirs_exist_ok=True)

if __name__ == "__main__":
    deploy()