# pebble-dev

Repository with my Pebble watchfaces, watchapps, and libraries - old and new.
Working status is shown in the lists below.

Some PBWs can be found in `/pbw` for sideloading.

- [watchfaces](#watchfaces)
- [watchapps](#watchapps)
- [libraries](#libraries)
- [other](#other)
- [Debugging](#debugging)


## `watchfaces`

> Platforms: `A` = Aplite, `B` = Basalt, `C` = Chalk, `D` = Diorite, `E` = Emery

| Name                  | Screenshot                                                 | Builds              | Icon | PBW                                |
|-----------------------|------------------------------------------------------------|---------------------|------|------------------------------------|
| `beam-up`             | ![](watchfaces/beam-up/screenshots/aplite.png)             | `A` `B` `C` `D`     | ✅    | [pbw](pbw/beam-up.pbw)             |
| `brackets`            | ![](watchfaces/brackets/screenshots/aplite.png)            | `A` `B` `C` `D`     | ✅    | [pbw](pbw/brackets.pbw)            |
| `cards`               | ![](watchfaces/cards/screenshots/diorite.png)              | `A` `B` `D`         |      | [pbw](pbw/cards.pbw)               |
| `cmd-time-typed`      | ![](watchfaces/cmd-time-typed/screenshots/diorite.png)     | `A` `B` `C` `D` `E` | ✅    | [pbw](pbw/cmd-time-typed.pbw)      |
| `cmd-time`            | ![](watchfaces/cmd-time/screenshots/diorite.png)           | `A` `B` `C` `D` `E` | ✅    | [pbw](pbw/cmd-time.pbw)            |
| `dayring`             | ![](watchfaces/dayring/screenshots/basalt.png)             | `A` `B` `C` `D`     |      | [pbw](pbw/dayring.pbw)             |
| `deep-rock`           | ![](watchfaces/deep-rock/screenshots/basalt.png)           | `A` `B` `D`         |      | [pbw](pbw/deep-rock.pbw)           |
| `dual-gauge`          | ![](watchfaces/dual-gauge/screenshots/basalt.png)          | `A` `B` `C` `D`     |      | [pbw](pbw/dual-gauge.pbw)          |
| `eclipse`             | ![](watchfaces/eclipse/screenshots/aplite.png)             | `A` `B` `D`         |      | [pbw](pbw/eclipse.pbw)             |
| `eventful`            | ![](watchfaces/eventful/screenshots/aplite.png)            | `A` `B` `C` `D`     |      | [pbw](pbw/eventful.pbw)            |
| `events`              | ![](watchfaces/events/screenshots/aplite.png)              | `A` `B` `D` `E`     | ✅    | [pbw](pbw/events.pbw)              |
| `hollywatch`          | ![](watchfaces/hollywatch/screenshots/basalt.png)          | `A` `B` `C` `D`     |      | [pbw](pbw/hollywatch.pbw)          |
| `index`               | ![](watchfaces/index/screenshots/aplite.png)               | `A` `B` `C` `D`     |      | [pbw](pbw/index.pbw)               |
| `isotime-appstore`    | ![](watchfaces/isotime-appstore/screenshots/yellow.png)    | `A` `B` `C` `D` `E` | ✅    | [pbw](pbw/isotime.pbw)             |
| `kitty-watchface`     | ![](watchfaces/kitty-watchface/screenshots/aplite.png)     | `A` `B` `D`         |      | [pbw](pbw/kitty-watchface.pbw)     |
| `morndas`             | ![](watchfaces/morndas/screenshots/basalt.png)             | `A` `B` `D`         |      | [pbw](pbw/morndas.pbw)             |
| `past-present-future` | ![](watchfaces/past-present-future/screenshots/aplite.png) | `A` `B` `D`         |      | [pbw](pbw/past-present-future.pbw) |
| `potential-divider`   | ![](watchfaces/potential-divider/screenshots/aplite.png)   | `A` `B` `D`         | ✅    | [pbw](pbw/potential-divider.pbw)   |
| `pseudotime`          | ![](watchfaces/pseudotime/screenshots/basalt.png)          | `A` `B` `C` `D`     |      | [pbw](pbw/pseudotime.pbw)          |
| `split-horizon`       | ![](watchfaces/split-horizon/screenshots/diorite.png)      | `A` `B` `D`         | ✅    | [pbw](pbw/split-horizon.pbw)       |
| `starfield-demo`      | ![](watchfaces/starfield-demo/screenshots/aplite.png)      | `A` `B` `C` `D`     |      | [pbw](pbw/starfield-demo.pbw)      |
| `starfield-smooth`    | ![](watchfaces/starfield-smooth/screenshots/aplite.png)    | `A` `B` `C`         | ✅    | [pbw](pbw/starfield-smooth.pbw)    |
| `thin`                | ![](watchfaces/thin/screenshots/basalt.png)                | `A` `B` `C` `D` `E` | ✅    | [pbw](pbw/thin.pbw)                |
| `time-dots-appstore`  | ![](watchfaces/time-dots-appstore/screenshots/basalt1.png) | `A` `B` `C` `D`     |      | [pbw](pbw/time-dots-appstore.pbw)  |
| `void-statues`        | ![](watchfaces/void-statues/screenshots/basalt.png)        | `A` `B` `C` `D` `E` |      | [pbw](pbw/void-statues.pbw)        |
| `eclipse-solar`       |                                                            | ❌ (WIP)             |      |                                    |


## `watchapps`

| Name             | Screenshot                                              | Builds                | Icon | PBW                           |
|------------------|---------------------------------------------------------|-----------------------|------|-------------------------------|
| `block-world`    | ![](watchapps/block-world/screenshots/basalt.png)       | `B` `C`               |      | [pbw](pbw/block-world.pbw)    |
| `dashboard`      | ![](watchapps/dashboard/assets/screenshots/basalt1.png) | `A` `B` `C` `D`       | ✅    | [pbw](pbw/dashboard.pbw)      |
| `news-headlines` | ![](watchapps/news-headlines/screenshots/basalt.png)    | `A` `B` `C` `D`       | ✅    | [pbw](pbw/news-headlines.pbw) |
| `tube-status`    | ![](watchapps/tube-status/screenshots/basalt.png)       | `A` `B` `C` `D` `E`   | ✅    | [pbw](pbw/tube-status.pbw)    |
| `pge-examples`   |                                                         | ❌ (needs pge package) |      |                               |

## `libraries`

| Name                      | Status | Test App |
|---------------------------|--------|----------|
| `InverterLayerCompat`     | ✅      | ✅        |
| `notif-layer`             | ✅      | ✅        |
| `pebble-isometric`        | ✅      | ✅        |
| `pebble-packet`           | ✅      | ✅        |
| `pebble-universal-fb`     | ✅      | ✅        |
| `pge`                     | ✅      | ✅        |
| `pebble-pge-simple`       |        |          |
| `pebble-simple-request`   |        |          |
| `pebble-timeline-js-node` |        |          |
| `pebble-timeline-js`      |        |          |

## `other`

Defunct, incomplete, or PoC things live here.

## Debugging

Here are some errors encountered in old projects and the fixes I found:

#### `Missing node_modules directory`

Caused by invalid `name` in `package.json`.

Caused by `version` that was missing patch component.

#### `This project is very outdated, and cannot be handled by this SDK.`

Missing `wscript`. Running `pebble convert-project` helped.

#### `An invalid value of '2' was found in the 'sdkVersion' field of the project's package.json. The latest supported value for this field is '3'.`

(Even when it's not true) - manually update `appinfo.json` to the more recent
`package.json` format from another working project, field by field.

#### waf-looking errors or resource indice errors

Copy `wscript` from a working project.

#### Missing images

As of SDK 3, some image resource types like `png` were deprecated. `bitmap`
should be used instead

## Todo

- [x] Fix build in GitHub Actions
- [x] Finish importing all previous Pebble projects
- [x] Merge all Split Horizon editions into config.
- [x] Migrate all custom config to Clay.
- [ ] Menu icons for all current watchfaces and watchapps
- [ ] Verify correct tick unit used to save power.
