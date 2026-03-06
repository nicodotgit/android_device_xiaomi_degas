# Android device tree for Xiaomi 14T (degas)

> TWRP device tree targeting Android 14/16 (vendor_boot ramdisk, Virtual A/B)

## Device Specifications

| Feature | Specification |
|---------|--------------|
| **Codename** | degas |
| **Model** | Xiaomi 14T (2406APNFAG) |
| **SoC** | MediaTek Dimensity 8300-Ultra (MT6897) |
| **RAM** | 12 GB LPDDR5 |
| **Storage** | 256 / 512 GB UFS 3.1 |
| **Display** | 6.67" AMOLED, 2712 × 1220, 144 Hz |
| **Battery** | 5000 mAh, 67W wired |
| **Android** | Ships with Android 14 (MIUI / HyperOS) |
| **TEE** | MiTEE (MediaTek Trusted Execution Environment) |
| **Encryption** | f2fs `aes-256-xts`:`aes-256-cts`:v2+inlinecrypt |

## Tree Structure

```
android_device_xiaomi_degas/
├── BoardConfig.mk                  # Board-level build config (MT6897)
├── device.mk                       # Product packages, A/B OTA config
├── twrp_degas.mk                   # Top-level product makefile
├── Android.mk / AndroidProducts.mk # Build system hooks
├── system.prop                     # Default system properties
├── board-info.txt                  # Board verification
│
├── init/
│   ├── Android.bp                  # libinit_degas static library
│   └── init_degas.cpp              # Vendor init: brand/model/marketname overrides
│
├── bootctrl/                       # MTK Boot Control HAL v1.2
│   ├── Android.bp
│   ├── BootControl.cpp / .h
│   ├── boot_control_definition.h
│   ├── boot_region_control.cpp / _private.h
│   ├── ufs-mtk-ioctl.h
│   └── ufs-mtk-ioctl-private.h
│
├── mtk_plpath_utils/               # Preloader DM-device path utility
│   ├── Android.bp
│   └── mtk_plpath_utils.cpp
│
├── prebuilt/
│   └── dtb                         # Device Tree Blob extracted from vendor_boot
│
├── security/
│   └── miui_releasekey             # MIUI OTA release key (X.509 PEM)
│
└── recovery/root/
    ├── init.recovery.mt6897.rc     # TEE/MiTEE init, persist mount, TEE perms
    ├── init.recovery.usb.rc        # USB gadget (ADB/MTP/RNDIS)
    ├── first_stage_ramdisk/
    │   ├── fstab.mt6897            # First-stage fstab for MT6897
    │   └── fstab.emmc              # Alias (same content, UFS device)
    ├── system/etc/
    │   ├── recovery.fstab          # TWRP partition mount table
    │   └── persist.fstab           # /mnt/vendor/persist mount
    ├── lib/modules/
    │   ├── *.ko                    # 241 kernel modules from vendor_dlkm
    │   ├── modules.load            # Full vendor_dlkm load order
    │   ├── modules.load.recovery   # Recovery-only subset (TEE, USB, display, haptics)
    │   ├── modules.dep
    │   ├── modules.alias
    │   └── modules.softdep
    └── vendor/
        ├── bin/tee-supplicant
        ├── bin/hw/
        │   ├── android.hardware.gatekeeper-service.mitee
        │   ├── android.hardware.security.keymint@3.0-service.mitee
        │   └── vendor.xiaomi.hardware.vibratorfeature.service  # from /odm
        ├── lib64/                  # 9 vendor shared libraries (TEE, sensors, vibrator)
        ├── mitee/ta/               # 15 MiTEE Trusted Applications
        ├── firmware/               # 82 firmware blobs (haptics, touch, BT/WiFi, ISP)
        └── etc/
            ├── init/               # 5 RC files (TEE services, vibrator)
            ├── vintf/              # compatibility_matrix.xml, manifest.xml
            └── ueventd.rc
```

## Building TWRP

### Prerequisites

- TWRP 3.7+ build environment (AOSP `twrp-14.1` manifest)
- Android build tools (Python 3, Ninja, ccache recommended)
- 16+ GB RAM, 250+ GB free disk space

### Sync

Add to your local manifest (`.repo/local_manifests/roomservice.xml`):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<manifest>
  <project name="android_device_xiaomi_degas"
           path="device/xiaomi/degas"
           remote="github"
           revision="twrp-14.1" />
</manifest>
```

Then:

```bash
repo sync device/xiaomi/degas
```

### Build

```bash
. build/envsetup.sh
lunch twrp_degas-eng
mka vendorbootimage
```

The output `vendor_boot.img` will be in `out/target/product/degas/`.

### Flash

```bash
# Boot into fastboot mode (Vol-Down + Power)
fastboot flash vendor_boot out/target/product/degas/vendor_boot.img
fastboot reboot
```

> **Note:** This tree uses `BOARD_USES_RECOVERY_AS_BOOT := false` and installs TWRP
> into the `vendor_boot` ramdisk slot, consistent with the MTK VAB layout on degas.

## Key Technical Notes

### Partition Layout
- Virtual A/B (VAB) with 54 `AB_OTA_PARTITIONS`
- Dynamic super partition (`sda20` on UFS, `sda` bus at `112b0000.ufshci`)
- Boot LU A/B switching via UFS ioctl on `/dev/block/sdc`

### Encryption
- Metadata: `/dev/block/by-name/metadata` → `/metadata` (f2fs)
- Userdata: `fileencryption=aes-256-xts:aes-256-cts:v2+inlinecrypt_optimized`
- Key directory: `/metadata/vold/metadata_encryption`
- Key mint HAL: `android.hardware.security.keymint@3.0-service.mitee`

### TEE (MiTEE)
- 15 Trusted Applications in `vendor/mitee/ta/`
- `tee-supplicant` bridges in-kernel OP-TEE with user-space
- `mitee.ko` loaded from vendor_dlkm
- `gz_trusty_mod.ko` et al. required for GZ (Hypervisor) TEE side

### Haptics
- AW8697 (aw8697_haptic.bin) — driven by `haptic.ko`
- SI Haptic — driven by `si_haptic.ko`
- Vibrator HAL: `vendor.xiaomi.hardware.vibratorfeature.service` (from `/odm/bin/hw/` on device)

### Reference Tree
Structural reference: [JonesqPacMan/android_device_xiaomi_rothko_twrp](https://github.com/JonesqPacMan/android_device_xiaomi_rothko_twrp) (branch `twrp-14.1_a16`)
Note: rothko is MT6989 (Dimensity 9300); degas is MT6897 (Dimensity 8300-Ultra) — different SoC family.

## Credits

- [JonesqPacMan](https://github.com/JonesqPacMan) – rothko tree (structural reference)
- [TeamWin](https://github.com/TeamWin) – TWRP
- [LineageOS](https://github.com/LineageOS) – MTK boot control HAL base
