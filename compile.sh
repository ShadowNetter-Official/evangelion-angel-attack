
echo "=== COMPILE BEGIN ==="
#!/usr/bin/env bash
# Simple 3DS build script (DevKitPro)
# Edit the variables at the top to add sources / libraries.

set -euo pipefail

# ------------------------------------------------------------------
# Configuration (edit these)
# ------------------------------------------------------------------
TARGET="evangelion_angel_attack"                 # output name (produces mygame.3dsx / mygame.elf)
BUILD="build"                   # object / intermediate directory
SOURCES=("source")              # directories containing .c / .cpp / .s
INCLUDES=("include")            # extra header directories
DATA=("data")                   # optional binary data directories (*.bin etc.)
GRAPHICS=("gfx")                # optional .t3s directories (tex3ds)

# App metadata (for .smdh)
APP_TITLE="Evangelion: Angel Attack"
APP_DESCRIPTION="A Neon Genesis Evangelion fan game"
APP_AUTHOR="ShadowNetter"
ICON=""                         # leave empty to auto-detect icon.png / <TARGET>.png

# Compiler flags
ARCH="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft"
CFLAGS="-g -Wall -O2 -mword-relocations -ffunction-sections ${ARCH} -D__3DS__"
CXXFLAGS="${CFLAGS} -fno-rtti -fno-exceptions -std=gnu++11"
ASFLAGS="-g ${ARCH}"
LDFLAGS="-specs=3dsx.specs -g ${ARCH} -Wl,-Map,${TARGET}.map"

# Libraries – add more here (order matters)
#LIBS="-lcitro2d -lcitro3d -lctru -lm"
LIBS="-lSDL_mixer -lSDL -lmikmod -lvorbisidec -lmad -logg -lcitro2d -lcitro3d -lctru -lm"
# Example: LIBS="-lctru -lm -lcitro2d -lcitro3d -lpng -lz"

# Library search paths (top-level dirs that contain include/ and lib/)
#LIBDIRS=("${CTRULIB:-$DEVKITPRO/libctru}")
LIBDIRS=(
    "${CTRULIB:-$DEVKITPRO/libctru}"
    "$DEVKITPRO/portlibs/3ds"
)
# Example: LIBDIRS=("$CTRULIB" "$DEVKITPRO/portlibs/3ds")

# Optional RomFS
ROMFS="romfs"

# ------------------------------------------------------------------
# Sanity checks
# ------------------------------------------------------------------
if [[ -z "${DEVKITARM:-}" ]]; then
    echo "Error: DEVKITARM is not set. Run:"
    echo "  export DEVKITARM=/opt/devkitpro/devkitARM   # or wherever yours is"
    exit 1
fi

if [[ -z "${DEVKITPRO:-}" ]]; then
    export DEVKITPRO="$(dirname "$DEVKITARM")"
fi

# Pull in the official 3DS toolchain definitions
source "$DEVKITARM/3ds_rules" 2>/dev/null || {
    # Fallback if 3ds_rules is not source-able (older setups)
    export PATH="$DEVKITARM/bin:$PATH"
    CC="arm-none-eabi-gcc"
    CXX="arm-none-eabi-g++"
    AS="arm-none-eabi-as"
    LD="arm-none-eabi-g++"
    OBJCOPY="arm-none-eabi-objcopy"
}

# ------------------------------------------------------------------
# Derived paths / flags
# ------------------------------------------------------------------
mkdir -p "$BUILD"

INCLUDE_FLAGS=""
for d in "${INCLUDES[@]}"; do
    INCLUDE_FLAGS+=" -I$(pwd)/$d"
done
for d in "${LIBDIRS[@]}"; do
    INCLUDE_FLAGS+=" -I$d/include"
done
INCLUDE_FLAGS+=" -I$(pwd)/$BUILD"

LIBPATH_FLAGS=""
for d in "${LIBDIRS[@]}"; do
    LIBPATH_FLAGS+=" -L$d/lib"
done

# ------------------------------------------------------------------
# Collect source files
# ------------------------------------------------------------------
C_SOURCES=()
CPP_SOURCES=()
S_SOURCES=()

for dir in "${SOURCES[@]}"; do
    [[ -d "$dir" ]] || continue
    while IFS= read -r -d '' f; do C_SOURCES+=("$f"); done < <(find "$dir" -maxdepth 1 -name '*.c' -print0)
    while IFS= read -r -d '' f; do CPP_SOURCES+=("$f"); done < <(find "$dir" -maxdepth 1 -name '*.cpp' -print0)
    while IFS= read -r -d '' f; do S_SOURCES+=("$f"); done < <(find "$dir" -maxdepth 1 -name '*.s' -print0)
done

# Decide linker
if [[ ${#CPP_SOURCES[@]} -gt 0 ]]; then
    LD="$CXX"
else
    LD="$CC"
fi

# ------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------
OBJECTS=()

compile() {
    local src="$1"
    local obj="$BUILD/$(basename "${src%.*}").o"
    local dep="$BUILD/$(basename "${src%.*}").d"

    OBJECTS+=("$obj")

    if [[ "$src" == *.c ]]; then
        echo "CC  $src"
        $CC $CFLAGS $INCLUDE_FLAGS -MMD -MP -MF "$dep" -c "$src" -o "$obj"
    elif [[ "$src" == *.cpp ]]; then
        echo "CXX $src"
        $CXX $CXXFLAGS $INCLUDE_FLAGS -MMD -MP -MF "$dep" -c "$src" -o "$obj"
    elif [[ "$src" == *.s ]]; then
        echo "AS  $src"
        $AS $ASFLAGS -c "$src" -o "$obj"
    fi
}

for src in "${C_SOURCES[@]}" "${CPP_SOURCES[@]}" "${S_SOURCES[@]}"; do
    compile "$src"
done

# ------------------------------------------------------------------
# Optional: binary data → .o (bin2o style)
# ------------------------------------------------------------------
for dir in "${DATA[@]}"; do
    [[ -d "$dir" ]] || continue
    while IFS= read -r -d '' bin; do
        name=$(basename "$bin")
        obj="$BUILD/${name}.o"
        header="$BUILD/${name//./_}.h"
        echo "BIN $bin → $obj"
        # bin2o is provided by DevKitPro
        bin2o "$bin" "$obj" "$header"
        OBJECTS+=("$obj")
    done < <(find "$dir" -maxdepth 1 -type f -print0)
done

# ------------------------------------------------------------------
# Link
# ------------------------------------------------------------------
echo "LD  $TARGET.elf"
$LD $LDFLAGS $LIBPATH_FLAGS -o "$TARGET.elf" "${OBJECTS[@]}" $LIBS

# ------------------------------------------------------------------
# Create .smdh (optional but usual)
# ------------------------------------------------------------------
SMDH_FLAGS=()
if [[ -z "${NO_SMDH:-}" ]]; then
    if [[ -n "$ICON" && -f "$ICON" ]]; then
        APP_ICON="$ICON"
    elif [[ -f "${TARGET}.png" ]]; then
        APP_ICON="${TARGET}.png"
    elif [[ -f "icon.png" ]]; then
        APP_ICON="icon.png"
    else
        APP_ICON=""
    fi

    if command -v smdhtool >/dev/null 2>&1; then
        echo "SMDH $TARGET.smdh"
        smdhtool --create "$APP_TITLE" "$APP_DESCRIPTION" "$APP_AUTHOR" \
                 "${APP_ICON:-$DEVKITPRO/libctru/default_icon.png}" "$TARGET.smdh"
        SMDH_FLAGS+=(--smdh="$(pwd)/$TARGET.smdh")
    fi
fi

# ------------------------------------------------------------------
# Create .3dsx
# ------------------------------------------------------------------
THREE_DSX_FLAGS=("${SMDH_FLAGS[@]}")
if [[ -n "${ROMFS:-}" && -d "$ROMFS" ]]; then
    THREE_DSX_FLAGS+=(--romfs="$(pwd)/$ROMFS")
fi

echo "3DSX $TARGET.3dsx"
3dsxtool "$TARGET.elf" "$TARGET.3dsx" "${THREE_DSX_FLAGS[@]}"

echo
echo "Build finished: $TARGET.3dsx"
echo "=== COMPILE END ==="
