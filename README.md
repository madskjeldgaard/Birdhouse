[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/X8X6RXV10)

# BirdHouse

## By MadsKjeldgaard

An OSC to MIDI bridge

Generated using [Cookiejuce](github.com/madskjeldgaard/Cookiejuce).

## Building

Configure:
```bash
cmake -S . -B build
```

Build:
```bash
cmake --build build
```

The plugins are now copied to your system.

Run standalone (on MacOS):
```bash
./build/BirdHouse_artefacts/Standalone/BirdHouse.app/Contents/MacOS/BirdHouse
```

## Usage


### Setting up

#### Basic Reaper usage
- Create two tracks: One for Birdhouse, one for an instrument you want to control with birdhouse.
- Add the Birdhouse plugin to the first track
- Record enable the track (otherwise, the plugin will not receive any MIDI when you switch windows on your computer)
- Add an instrument to the second track

- Click and drag the "ROUTE" button to the second track to route the MIDI from Birdhouse to the instrument

![reaper routing](manual/reaperrouting.png) 

