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

* `android.permission.WRITE_SYNC_SETTINGS` - Toggle AutoSync
* `android.permission.READ_SYNC_SETTINGS`

*Runtime Permissions*

These permissions are opt-in if you want to enable the feature they enable.

* None yet

## Roadmap

- [x] Basic comms with Android
- [x] Running in background
- [ ] MVP initial features:
  - [ ] Basic watch UI
  - [ ] Basic Android app UI
  - [x] Loading splash
  - [x] Watchapp minimum version check
  - [x] Hard-coded toggle order
  - [x] Phone name
  - [x] Phone battery level
  - [x] Phone free space
  - [ ] Toggle AutoSync (background app updates)
  - [ ] Change ringer mode? (Requires notif policy one-time permissions popup)
  - [ ] Hard-coded Find Phone sound (and/or flash?)
- [ ] Post-MVP:
  - [ ] Customizable toggle order
  - [ ] Customizable find phone sound
  - [ ] Find Watch with vibration?
  - [ ] Feature diagnostic page
  - [ ] WiFi network name? (Needs location permission)
- [ ] Advanced (via [Shizuku](https://github.com/RikkaApps/Shizuku) or similar)
  - [ ] Toggle WiFi?
  - [ ] Toggle mobile data?
  - [ ] Lock phone?
  - [ ] Hotspot?
- [ ] New things!?
  - [ ] Change WiFi network?
  - [ ] Next calendar event (needs calendar permission)
  - [ ] Battery status / temperature / plugged?
  - [ ] Free RAM
  - [ ] Is WiFi vs is mobile data (may not require permission)
  - [ ] Next alarm?
  - [ ] Custom webhook POST URL
- [ ] Summon phone features as Activities?
  - [ ] NFC payment
