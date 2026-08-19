# dashboard2

Attempt to learn Kotlin and use
[PebbleKit Android 2](https://github.com/pebble-dev/PebbleKitAndroid2) to
re-make
[Dashboard](https://apps.rebble.io/en_US/application/53ec8d840c3036447e000109),
starting with minimal features and adding more as they are deemed possible
without to much special device access or root. The focus will be on modularity,
maintainability, and reusability.

## Feature requests

Please see [this issue](https://github.com/C-D-Lewis/pebble-dev/issues/18) for
status of feature requests. If you think you can help implement a new feature
(new info or toggle), PRs are welcome!

## Permissions

These permissions are listed in the manifest and explained below. If you choose
not to grant them, the corresponding feature will just not work.

| Permission                                      | Feature           | Details                                               |
|-------------------------------------------------|-------------------|-------------------------------------------------------|
| `android.permission.ACCESS_NOTIFICATION_POLICY` | Cycle ringer mode | allow prompting access to notification policy / modes |

## Roadmap

- [x] Basic comms with Android
- [x] Running in background
- [ ] MVP initial features:
  - [x] Basic watch UI (info + toggles)
  - [x] Basic Android app UI
  - [x] Loading splash
  - [x] Watchapp minimum version check
  - [x] Hard-coded toggle order
  - [x] Phone name
  - [x] Phone battery level
  - [x] Phone free space
  - [x] Hard-coded Find Phone sound (and/or flash?)
  - [x] Next alarm
  - [x] Is battery charging?
  - [x] Change ringer mode? (Requires notif policy one-time permissions popup)
  - [ ] Check all layouts
- [ ] Post-MVP:
  - [ ] Customizable toggle order
  - [ ] Customizable find phone sound
  - [ ] Find Watch with vibration/sound/light?
  - [ ] WiFi network name? (Needs location permission - DOA due to Android...)
  - [ ] Next calendar event (needs calendar permission)
  - [ ] Custom webhook POST URL
- [ ] Advanced (via [Shizuku](https://github.com/RikkaApps/Shizuku) or similar)
  - [ ] Toggle WiFi?
  - [ ] Toggle mobile data?
  - [ ] Lock phone?
  - [ ] Hotspot?
  - [ ] Toggle AutoSync (background app updated not allowed DOA)
  - [ ] Change WiFi network?
- [ ] New things!?
  - [ ] Free RAM (not useful as Linux uses it)
  - [ ] Is WiFi vs is mobile data (may not require permission)
- [ ] Summon phone features as Activities?
  - [ ] NFC payment
