import os
import re
import zipfile

def get_version_from_header(header_path: str) -> int:
    with open(header_path, "r", encoding="utf-8") as f:
        text = f.read()
    match = re.search(r"constexpr\s+int\s+SOUNDIO_VERSION\s*=\s*(\d+)\s*;", text)
    if not match:
        raise ValueError("SOUNDIO_VERSION not found in SoundIO.h")
    return int(match.group(1))

def zip_src_folder(version: int, src_folder: str = "src", build_dir: str = "builds"):
    os.makedirs(build_dir, exist_ok=True)

    zip_filename = f"SoundIO_Release_{version}.zip"
    zip_path = os.path.join(build_dir, zip_filename)

    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, _, files in os.walk(src_folder):
            for file in files:
                file_path = os.path.join(root, file)

                relative_path = os.path.relpath(file_path, src_folder)
                arcname = os.path.join("SoundIO", relative_path)

                zipf.write(file_path, arcname)
    
    print(f"SoundIO built and packed as ./builds/{zip_filename}")

def main():
    header_path = os.path.join("src", "SoundIO.h")
    version = get_version_from_header(header_path)
    zip_src_folder(version)

if __name__ == "__main__":
    main()
