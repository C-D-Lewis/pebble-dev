# pebble-dev

Repository with Pebble watchfaces, watchapps, and libraries - old and new.

Projects here are mostly too old for the latest Pebble SDK.

## Buildable projects

### Watchfaces

* `brackets`
* `dual-gauge`
* `split-horizen-se` (crashes, InverterLayerCompat.c)
* `starfield-smooth`
* `startfield-demo`
* `time-dots-appstore`

### Watchapps

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

Caused by invalid `name` in `package.json`

#### `This project is very outdated, and cannot be handled by this SDK.`

Missing `wscript`. Running `pebble convert-project` helped.


## Todo

* Finish importing all previous Pebble projects
* Fixup old projects
