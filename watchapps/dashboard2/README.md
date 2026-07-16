# dashboard2

Attempt to learn Kotlin and use
[PebbleKit Android 2](https://github.com/pebble-dev/PebbleKitAndroid2) to
re-make
[Dashboard](https://apps.rebble.io/en_US/application/53ec8d840c3036447e000109),
starting with minimal features and adding more as they are deemed possible
without to much special device access or root. The focus will be on modularity,
maintainability, and reusability.

## Permissions

*Manifest Permissions*

* `android.permission.ACCESS_FINE_LOCATION` - Get Wi-Fi network info
* `android.permission.ACCESS_COARSE_LOCATION` - Get Wi-Fi network info
* `android.permission.ACCESS_WIFI_STATE` - Get Wi-Fi network info
* `android.permission.ACCESS_NETWORK_STATE` - Get network state

*Runtime Permissions*

These permissions are opt-in if you want to enable the feature they enable.

* None yet

## Roadmap

- [x] Basic comms with Android
- [x] Running in background
- [ ] MVP initial features:
  - [ ] Basic watch UI (info + tools)
  - [ ] Basic Android app UI
  - [x] Loading splash
  - [x] Watchapp minimum version check
  - [x] Hard-coded toggle order
  - [x] Phone name
  - [x] Phone battery level
  - [x] Phone free space
  - [-] Next alarm
  - [ ] Change ringer mode? (Requires notif policy one-time permissions popup)
  - [ ] Hard-coded Find Phone sound (and/or flash?)
- [ ] Post-MVP:
  - [ ] Customizable toggle order
  - [ ] Customizable find phone sound
  - [ ] Find Watch with vibration/sound/light?
  - [ ] WiFi network name? (Needs location permission - DOA due to Android...)
- [ ] Advanced (via [Shizuku](https://github.com/RikkaApps/Shizuku) or similar)
  - [ ] Toggle WiFi?
  - [ ] Toggle mobile data?
  - [ ] Lock phone?
  - [ ] Hotspot?
  - [ ] Toggle AutoSync (background app updates)
- [ ] New things!?
  - [ ] Change WiFi network?
  - [ ] Next calendar event (needs calendar permission)
  - [ ] Battery status / temperature / plugged?
  - [ ] Free RAM (not useful as Linux uses it)
  - [ ] Is WiFi vs is mobile data (may not require permission)
  - [ ] Custom webhook POST URL
- [ ] Summon phone features as Activities?
  - [ ] NFC payment
