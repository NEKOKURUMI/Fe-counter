# FE Counter — Setup & Build Guide

This is a **Geode mod** written in C++. It needs to be compiled on your
machine — I can't compile it for you in this chat, since it requires
the actual Geode SDK, a C++ compiler, and links against Geometry Dash's
own game code.

## 1. Check if you already have what you need

Open a terminal (Command Prompt / PowerShell on Windows) and run:

```
geode --version
```

- If it prints a version number → you have the Geode CLI. Skip to step 3.
- If it says "command not found" → you need to install it (step 2).

## 2. Install the Geode SDK + CLI (if you don't have it)

1. Go to https://geode-sdk.org/install and download the installer for
   your OS (Windows is the standard/most supported platform for GD
   modding).
2. Run the installer — it installs:
   - Geode Loader (injects into GD itself)
   - The Geode CLI (`geode` command, used to build/package mods)
3. **Also required on Windows:** Visual Studio 2022 with the
   "Desktop development with C++" workload. Install via
   https://visualstudio.microsoft.com/downloads/ if you don't have it.
   This provides the actual C++ compiler (MSVC) that CMake will use.
4. Restart your terminal after installing, then re-run `geode --version`
   to confirm it works.

## 3. Set the GEODE_SDK environment variable

The CMakeLists.txt in this project needs to know where the SDK lives.
The installer usually sets this automatically, but to check:

**Windows (PowerShell):**
```
echo $env:GEODE_SDK
```

If empty, find where Geode was installed (commonly
`C:\Geode\sdk` or wherever you chose) and set it:
```
setx GEODE_SDK "C:\path\to\geode\sdk"
```
(Restart your terminal after this.)

## 4. Build the mod

From inside this project folder:

```
geode build
```

This runs CMake + compiles it, and outputs a `.geode` file (the
packaged mod) — usually under `build/`.

## 5. Install it into your game

Easiest way:
```
geode install build/FECounter.geode
```

Or just drag the `.geode` file into your GD mods folder (Geode will
tell you where that is, or check via the in-game Geode mod menu).

## 6. Launch GD and test

- Open the Geode mod list in-game, make sure "FE Counter" is enabled.
- Load a level and play — you should see:
  - Top-right: live MS/Hz readout
  - Top-left: attempt counts bucketed by percent range (1, 2, 3, 4,
    5-6, 7-8, 9-12, 13-15), incrementing each time you die in that
    range, resetting when you re-enter the level.

## If the build fails

Common issues:
- **"Unable to find Geode SDK"** → `GEODE_SDK` isn't set correctly (step 3).
- **MSVC/compiler errors** → Visual Studio's C++ workload isn't installed, or
  you're not running from a "Developer" terminal. Try "Developer PowerShell
  for VS 2022" from the Start menu instead of a plain terminal.
- **Binding/class not found errors** (e.g. `getCurrentPercent` missing) →
  your installed Geode SDK bindings version may not match; run
  `geode sdk update` to pull the latest.

Paste me the exact error text if you hit something and I'll help you
fix it.
