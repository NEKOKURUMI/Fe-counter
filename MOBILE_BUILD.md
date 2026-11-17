# Build FE Counter with NO PC — using GitHub Actions on your phone

This lets GitHub's servers compile the mod for you. You just need a
free GitHub account and a browser (works fine on mobile Chrome/Safari).

## Step 1 — Create a GitHub account

Go to https://github.com/signup in your phone browser and make a free
account, if you don't have one already.

## Step 2 — Create a new repository

1. Tap the **+** icon (top right) → **New repository**
2. Name it something like `fe-counter`
3. Set it to **Public** (required for free Actions minutes)
4. Tap **Create repository**

## Step 3 — Upload the files

On your new repo's page:

1. Tap **Add file** → **Upload files**
2. Upload ALL the files from this zip, keeping the folder structure:
   - `mod.json`
   - `CMakeLists.txt`
   - `src/FECounter.cpp`
   - `.github/workflows/build.yml`

   Mobile browsers sometimes don't let you upload folders directly —
   if that happens, upload each file one at a time, and when GitHub
   asks for the file name/path, type the full path like
   `src/FECounter.cpp` or `.github/workflows/build.yml` so it lands
   in the right folder.

3. Scroll down, tap **Commit changes**

## Step 4 — Let it build

1. Go to the **Actions** tab of your repo (top menu)
2. You should see a workflow run called "Build Geode Mod" already
   running (it triggers automatically on upload)
3. Tap it, wait for the green checkmarks (takes a few minutes —
   compiling C++ isn't instant even in the cloud)

## Step 5 — Download your .geode file

1. Once the run finishes (green check), tap into it
2. Scroll down to **Artifacts**
3. You'll see entries like `FECounter-Android64`,
   `FECounter-Android32`, and `FECounter-combined`
4. Tap `FECounter-combined` to download — this is a `.zip` containing
   your `.geode` file (GitHub always wraps artifacts in a zip, so
   you'll need to unzip it once — most phone file managers can do
   this, or apps like "Files" on Android can unzip directly)

## Step 6 — Install on your phone

1. Make sure you have **Geode installed** on your Android device
   (see https://geode-sdk.org/install if not — it has an Android
   installer)
2. Move the `.geode` file into your GD mods folder, or use Geode's
   in-app "install from file" option if available
3. Restart Geometry Dash, enable "FE Counter" in the Geode mod list

## If the build fails (red X instead of green check)

Tap into the failed run → tap the failed job → read the error log.
Common issues:
- Missing file (double check all 4 files uploaded to the right paths)
- A typo in `mod.json` (it must be valid JSON)

Copy the error text and send it to me — I'll fix the source and you
just re-upload the corrected file to trigger a new build.
