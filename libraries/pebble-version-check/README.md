# pebble-version-check

Simple version checking mechanism for Pebble apps.

## How to use

1. Create a `VersionCheckCallback` implementation to receive the result of the
   version check:

        static void version_check_callback(bool success) {
          
        }

2. Send a version check (AppMessage callbacks are overridden at this time):

        static char s_version = "1.0.0";
        version_check(s_version, version_check_callback);

3. In your Android companion app, pass the received `PebbleDictionary` to
   `check` in `VersionCheck.java`, along with the local version to check against.

        VersionCheck.check(getApplicationContext(), Build.WATCHAP_UUID, dict, Build.COMPATIBLE_WATCHAPP_VERSION);

4. The result will be propagated back to your `VersionCheckCallback`, whereupon
   you should update the UI with the result (i.e: tell the user the version does
   not match the companion app).