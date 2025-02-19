# pebble-dev

Repository with Pebble watchfaces, watchapps, and libraries - old and new.

Projects here are mostly too old for the latest Pebble SDK.

## Watchfaces

* ⚠️ `split-horizon-se` (crashes, InverterLayerCompat.c)
* ✅ `brackets`
* ✅ `dual-gauge`
* ✅ `eclipse`
* ✅ `index`
* ✅ `kitty-watchface`
* ✅ `split-horizon-pe`
* ✅ `starfield-smooth`
* ✅ `startfield-demo`
* ✅ `time-dots-appstore`
* ❌ `cards` (custom fonts)
* ❌ `cmd-time-typed` (old inverter layer)
* ❌ `cmd-time` (old inverter layer)
* ❌ `isotime-appstore` (pebble-pge-simple reference)
* ❌ `split-horizon-me` (old inverter layer)
* ❔ `divider-large-2`
* ❔ `divider-small-2`
* ❔ `eventful`
* ❔ `events`
* ❔ `past-present-future-extended`
* ❔ `past-present-future`
* ❔ `seven-segments-date`
* ❔ `seven-segments`

## Watchapps

* ⚠️ `tube-status` (data sent, no reaction)
* ✅ `block-world`
* ✅ `modular-app-base`
* ✅ `news-headlines`
* ❌ `pge-examples` (needs ref to pge package)

## Other

Defunct, incomplete, or PoC things live here.

## Other repositories

Some projects still live in their own repositories:

* ✅ [thin](https://github.com/C-D-Lewis/thin)
* ✅ [dashboard](https://github.com/C-D-Lewis/dashboard)
* ✅ [beam-up](https://github.com/C-D-Lewis/beam-up)
* ✅ [pge](https://github.com/C-D-Lewis/pge)

## Develop in Docker

If you have a Docker image locally called `pebble-sdk`, the script
`docker-dev.sh` can be used to run it and allow building and running at least on
an emulator.

Such an image can be built with
[bboehmke/docker-pebble-dev](https://github.com/bboehmke/docker-pebble-dev)

```
docker build -t pebble-sdk .
```

Then run it pointing to a Pebble project directory:

```
./scripts/docker-dev.sh watchfaces/starfield-smooth
```

### Debugging

Here are some errors encountered in old projects and the fixes I found:

#### `Missing node_modules directory`

Caused by invalid `name` in `package.json`.

Caused by `version` that was missing patch component.

#### `This project is very outdated, and cannot be handled by this SDK.`

Missing `wscript`. Running `pebble convert-project` helped.

#### `An invalid value of '2' was found in the 'sdkVersion' field of the project's package.json. The latest supported value for this field is '3'.`

(Even when it's not true) - manually update `appinfo.json` to the more recent `package.json` format from another working project, field by field.

#### waf-looking errors or resource indice errors

Copy `wscript` from a working project.

#### Missing images

As of SDK 3, some image resource types like `png` were deprecated.

## Todo

* Finish importing all previous Pebble projects
* Fixup old projects
