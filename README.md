# SAGIT-PONGO

Linux ARM64 Pongo project for Xiaomi Mi 6 (sagit).

## Termux: установка зависимостей

Обновление пакетов:

    pkg update && pkg upgrade

Установка инструментов:

    pkg install clang lld make python binutils coreutils gzip tar git

## Fedora: установка зависимостей

    sudo dnf install clang lld make python3 binutils coreutils gzip tar git

## Команды сборки

### ARM64 Pongo ELF

    make pc_pongo

Сборка Pongo для ARM64 на ПК.

    make termux_pongo

Сборка Pongo для ARM64 в Termux.

### Полная сборка Android boot.img

    make pc_all

Полная сборка на ПК.

    make termux_all

Полная сборка в Termux.

### Очистка

    make clean

Удаление результатов предыдущей сборки.

## Результаты

ARM64 ELF:

    build/pongo-build/pongo_real

Initramfs:

    build/boot/ramdisk-pongo.cpio.gz

Android boot image:

    build/boot/pongo-linux.img

## Architecture

Target: AArch64 / ARM64
Device: Xiaomi Mi 6 (sagit)
Format: Static Linux ELF
