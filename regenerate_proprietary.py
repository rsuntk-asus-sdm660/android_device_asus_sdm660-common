#!/usr/bin/env python3

from pathlib import Path


THIS_DIR = Path(__file__).resolve().parent
ANDROID_ROOT = THIS_DIR.parents[2]
PROP_ROOT = ANDROID_ROOT / "vendor/asus/sdm660-common/proprietary"
OUT_FILE = THIS_DIR / "proprietary-files.txt"


def add_prefixes(prefixes: dict[str, str], paths: list[str], suffix: str) -> None:
    for p in paths:
        prefixes[p] = suffix


def so_stem(path: str) -> str:
    name = Path(path).name
    if name.endswith(".so"):
        return name[:-3]
    return name


def has_module_suffix(prefix: str) -> bool:
    return ";MODULE_SUFFIX=" in prefix


def append_module_suffix(prefix: str, suffix: str) -> str:
    if not prefix:
        return f";MODULE_SUFFIX={suffix}"

    if ";MODULE_SUFFIX=" in prefix:
        return prefix

    if "|" in prefix:
        left, right = prefix.split("|", 1)
        return f"{left};MODULE_SUFFIX={suffix}|{right}"

    return f"{prefix};MODULE_SUFFIX={suffix}"


def collect_files(root: Path) -> list[str]:
    return sorted(
        p.relative_to(root).as_posix()
        for p in root.rglob("*")
        if p.is_file()
    )


def decorate(path: str, prefixes: dict[str, str]) -> str:
    return path + prefixes.get(path, "")


# -----------------------------------------------------------------------------
# Prefixes/attributes preserved from the old proprietary-files.txt
# Applied ONLY if the file exists in the current real tree
# -----------------------------------------------------------------------------

PREFIXES: dict[str, str] = {}

# system_ext
PREFIXES["system_ext/lib64/libimscamera_jni.so"] = (
    ";SYMLINK=system_ext/priv-app/ims/lib/arm64/libimscamera_jni.so"
)
PREFIXES["system_ext/lib64/libimsmedia_jni.so"] = (
    ";SYMLINK=system_ext/priv-app/ims/lib/arm64/libimsmedia_jni.so"
)
PREFIXES["system_ext/lib64/fm_helium.so"] = (
    "|d64573b7a5b567bd9ee605d67b40357d55daf747"
)
PREFIXES["system_ext/lib64/libfm-hci.so"] = (
    "|13752c0da7302c0cc4ae4a3006a991f47e3bc509"
)

# vendor app
PREFIXES["vendor/app/CneApp/CneApp.apk"] = (
    ";REQUIRED=CneApp.libvndfwk_detect_jni.qti_symlink"
)

# vendor/lib
PREFIXES["vendor/lib/egl/libEGL_adreno.so"] = (
    ";SYMLINK=vendor/lib/libEGL_adreno.so"
)
PREFIXES["vendor/lib/egl/libGLESv2_adreno.so"] = (
    ";SYMLINK=vendor/lib/libGLESv2_adreno.so"
)
PREFIXES["vendor/lib/libmmosal.so"] = ";MODULE_SUFFIX=_vendor"

# vendor/lib64 egl
PREFIXES["vendor/lib64/egl/libEGL_adreno.so"] = (
    ";SYMLINK=vendor/lib64/libEGL_adreno.so"
)
PREFIXES["vendor/lib64/egl/libGLESv2_adreno.so"] = (
    ";SYMLINK=vendor/lib64/libGLESv2_adreno.so"
)

# vendor/lib64 explicit MODULE_SUFFIX=_vendor from old list
add_prefixes(PREFIXES, [
    "vendor/lib64/com.qualcomm.qti.dpm.api@1.0.so",
    "vendor/lib64/com.qualcomm.qti.imscmservice@1.0.so",
    "vendor/lib64/com.qualcomm.qti.imscmservice@2.0.so",
    "vendor/lib64/com.qualcomm.qti.imscmservice@2.1.so",
    "vendor/lib64/com.qualcomm.qti.imscmservice@2.2.so",
    "vendor/lib64/com.qualcomm.qti.uceservice@2.0.so",
    "vendor/lib64/com.qualcomm.qti.uceservice@2.1.so",
    "vendor/lib64/com.qualcomm.qti.uceservice@2.2.so",
    "vendor/lib64/com.qualcomm.qti.uceservice@2.3.so",
    "vendor/lib64/libmmosal.so",
    "vendor/lib64/vendor.qti.data.factory@2.0.so",
    "vendor/lib64/vendor.qti.data.factory@2.1.so",
    "vendor/lib64/vendor.qti.data.factory@2.2.so",
    "vendor/lib64/vendor.qti.data.factory@2.3.so",
    "vendor/lib64/vendor.qti.data.mwqem@1.0.so",
    "vendor/lib64/vendor.qti.data.slm@1.0.so",
    "vendor/lib64/vendor.qti.hardware.alarm@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.cne.internal.api@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.cne.internal.constants@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.cne.internal.server@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.connection@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.connection@1.1.so",
    "vendor/lib64/vendor.qti.hardware.data.dynamicdds@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.dynamicdds@1.1.so",
    "vendor/lib64/vendor.qti.hardware.data.flow@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.flow@1.1.so",
    "vendor/lib64/vendor.qti.hardware.data.iwlan@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.latency@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.lce@1.0.so",
    "vendor/lib64/vendor.qti.hardware.data.qmi@1.0.so",
    "vendor/lib64/vendor.qti.hardware.fm@1.0.so",
    "vendor/lib64/vendor.qti.hardware.mwqemadapter@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.am@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.1.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.2.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.3.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.4.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.5.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.6.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.7.so",
    "vendor/lib64/vendor.qti.hardware.radio.ims@1.8.so",
    "vendor/lib64/vendor.qti.hardware.radio.internal.deviceinfo@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.lpa@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.qcrilhook@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.1.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.2.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.3.so",
    "vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.4.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim@1.1.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim@1.2.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.0.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.1.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.2.so",
    "vendor/lib64/vendor.qti.hardware.radio.uim_remote_server@1.0.so",
    "vendor/lib64/vendor.qti.hardware.slmadapter@1.0.so",
    "vendor/lib64/vendor.qti.ims.callcapability@1.0.so",
    "vendor/lib64/vendor.qti.ims.callinfo@1.0.so",
    "vendor/lib64/vendor.qti.ims.factory@1.0.so",
    "vendor/lib64/vendor.qti.ims.factory@1.1.so",
    "vendor/lib64/vendor.qti.ims.rcsconfig@1.0.so",
    "vendor/lib64/vendor.qti.ims.rcsconfig@1.1.so",
    "vendor/lib64/vendor.qti.ims.rcsconfig@2.0.so",
    "vendor/lib64/vendor.qti.ims.rcsconfig@2.1.so",
    "vendor/lib64/vendor.qti.imsrtpservice@3.0.so",
    "vendor/lib64/vendor.qti.latency@2.0.so",
    "vendor/lib64/vendor.qti.latency@2.1.so",
    "vendor/lib64/vendor.qti.latency@2.2.so",
], ";MODULE_SUFFIX=_vendor")


# -----------------------------------------------------------------------------
# Grouping order
# -----------------------------------------------------------------------------

GROUPS: list[tuple[str, list[str]]] = [
    ("# System - Framework", ["system/framework/"]),

    ("# System Ext - Apps", ["system_ext/app/"]),
    ("# System Ext - Priv Apps", ["system_ext/priv-app/"]),
    ("# System Ext - Permissions", ["system_ext/etc/permissions/"]),
    ("# System Ext - Sysconfig", ["system_ext/etc/sysconfig/"]),
    ("# System Ext - ETC", ["system_ext/etc/"]),
    ("# System Ext - Framework", ["system_ext/framework/"]),
    ("# System Ext - Lib", ["system_ext/lib/"]),
    ("# System Ext - Lib64", ["system_ext/lib64/"]),

    ("# Vendor - Apps", ["vendor/app/"]),
    ("# Vendor - HAL Binaries", ["vendor/bin/hw/"]),
    ("# Vendor - Binaries", ["vendor/bin/"]),
    ("# Vendor - CNE Configs", ["vendor/etc/cne/"]),
    ("# Vendor - Data Configs", ["vendor/etc/data/"]),
    ("# Vendor - Default Permissions", ["vendor/etc/default-permissions/"]),
    ("# Vendor - Init Scripts", ["vendor/etc/init/"]),
    ("# Vendor - Seccomp Policies", ["vendor/etc/seccomp_policy/"]),
    ("# Vendor - ETC", ["vendor/etc/"]),
    ("# Vendor - EGL Libraries (32-bit)", ["vendor/lib/egl/"]),
    ("# Vendor - HAL Libraries (32-bit)", ["vendor/lib/hw/"]),
    ("# Vendor - ADSP Libraries (32-bit)", ["vendor/lib/rfsa/adsp/"]),
    ("# Vendor - SoundFX Libraries (32-bit)", ["vendor/lib/soundfx/"]),
    ("# Vendor - Libraries (32-bit)", ["vendor/lib/"]),
    ("# Vendor - EGL Libraries (64-bit)", ["vendor/lib64/egl/"]),
    ("# Vendor - HAL Libraries (64-bit)", ["vendor/lib64/hw/"]),
    ("# Vendor - SoundFX Libraries (64-bit)", ["vendor/lib64/soundfx/"]),
    ("# Vendor - Libraries (64-bit)", ["vendor/lib64/"]),
    ("# Vendor - Radio Database Upgrades", ["vendor/radio/qcril_database/upgrade/"]),
    ("# Vendor - Radio Database", ["vendor/radio/qcril_database/"]),
]


def apply_auto_vendor_suffixes(files: list[str], prefixes: dict[str, str]) -> None:
    """
    If a vendor .so has the same stem as a .so in system/system_ext,
    append ;MODULE_SUFFIX=_vendor to avoid Soong duplicate module names.
    """
    system_like_stems: set[str] = set()

    for f in files:
        if not f.endswith(".so"):
            continue
        if f.startswith(("system/lib/", "system/lib64/", "system_ext/lib/", "system_ext/lib64/")):
            system_like_stems.add(so_stem(f))

    for f in files:
        if not f.endswith(".so"):
            continue
        if not f.startswith(("vendor/lib/", "vendor/lib64/")):
            continue

        stem = so_stem(f)
        if stem in system_like_stems:
            current = prefixes.get(f, "")
            if not has_module_suffix(current):
                prefixes[f] = append_module_suffix(current, "_vendor")


def main() -> None:
    if not PROP_ROOT.exists():
        raise SystemExit(f"Missing proprietary dir: {PROP_ROOT}")

    files = collect_files(PROP_ROOT)

    # Apply automatic MODULE_SUFFIX=_vendor for duplicate vendor libs
    apply_auto_vendor_suffixes(files, PREFIXES)

    assigned: set[str] = set()
    output: list[str] = [
        "# Vendor blobs for sdm660-common",
        "# Autogenerated strictly from vendor/asus/sdm660-common/proprietary",
        "# Preserved attributes come only from the old proprietary-files list",
        "",
    ]

    for header, prefixes in GROUPS:
        group_files = [
            f for f in files
            if f not in assigned and any(f.startswith(pref) for pref in prefixes)
        ]

        if not group_files:
            continue

        output.append(header)
        for f in group_files:
            output.append(decorate(f, PREFIXES))
            assigned.add(f)
        output.append("")

    remaining = [f for f in files if f not in assigned]
    if remaining:
        output.append("# Ungrouped")
        for f in remaining:
            output.append(decorate(f, PREFIXES))
        output.append("")

    OUT_FILE.write_text("\n".join(output).rstrip() + "\n", encoding="utf-8")

    print(f"Wrote: {OUT_FILE}")
    print(f"Files: {len(files)}")
    if remaining:
        print(f"Ungrouped: {len(remaining)}")


if __name__ == "__main__":
    main()
