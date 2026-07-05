# dashboard2

Attempt to learn Kotlin and use
[PebbleKit Android 2](https://github.com/pebble-dev/PebbleKitAndroid2) to
re-make
[Dashboard](https://apps.rebble.io/en_US/application/53ec8d840c3036447e000109),
starting with minimal features and adding more as they are deemed possible
without to much special device access or root. The focus will be on modularity,
maintainability, and reusability.

## TODO

- [x] Basic comms with Android
- [x] Running in background
- [ ] MVP initial features:
  - [ ] Basic watch UI
  - [ ] Basic Android app UI
  - [x] Hard-coded toggle order
  - [x] Phone name
  - [x] Phone battery level
  - [x] Phone free space
  - [ ] Watchapp minimum version check
  - [ ] Hard-coded Find Phone sound (and/or flash?)
  - [ ] Change ringer mode? (Requires one-time permissions popup)
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
- [ ] New things!?
  - [ ] Change WiFi network?
  - [ ] Next calendar event (needs calendar permission)
