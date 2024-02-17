---
title: "Birdhouse"
subtitle: "OSC to MIDI Plugin manual"
author: Mads Kjeldgaard 
linkcolor: blue
urlcolor: blue
toc: true
papersize: a4
documentclass: extarticle # Needed for changing font size
logo: "packaging/icon.png"
fontsize: 14pt
geometry:
- top=30mm
- left=30mm
- heightrounded
---

![birdhouse logo](packaging/icon.png){ width=50% }

# About

The Birdhouse OSC to MIDI plugin is a simple plugin that listens for OSC messages, processes their data and sends outputs it as MIDI to allow using it in a DAW or other plugin host environment. Each instance of Birdhouse is able to process a stream of OSC messages to a MIDI event type, with a visualization of the stream and the ability to mute/unmute the output data. 

# Overview

## Channels

[![](https://mermaid.ink/img/pako:eNoljsEKwjAMhl-l5FRBX2AHQTcPgmOgsIv1ELroil072nQiw3e30_8UvnwJ_wzadwQF3K1_6R4Di9NZOZGzuzaXsqYY8UE3sdls97JG1r1gL0bkfpVRKVu0iUTUaGkBrWxNTGhNpLD6_ykzrmR9rI6i9G6iEI13i3uQTeIxsfjtaCLHvxNYw0BhQNPlXvNCFHBPAyko8thheCpQ7pM9TOwvb6eh4JBoDWnskKky-Ag4QHFHG-nzBSB2SUg?type=png)](https://mermaid.live/edit#pako:eNoljsEKwjAMhl-l5FRBX2AHQTcPgmOgsIv1ELroil072nQiw3e30_8UvnwJ_wzadwQF3K1_6R4Di9NZOZGzuzaXsqYY8UE3sdls97JG1r1gL0bkfpVRKVu0iUTUaGkBrWxNTGhNpLD6_ykzrmR9rI6i9G6iEI13i3uQTeIxsfjtaCLHvxNYw0BhQNPlXvNCFHBPAyko8thheCpQ7pM9TOwvb6eh4JBoDWnskKky-Ag4QHFHG-nzBSB2SUg)

_An overview of the proessing of a single channel_

Birdhouse consists of 8 channels. A channel in this context is a pipeline that receives an OSC message at a given address, processes it, and sends it out as MIDI.

Each channel has a set of parameters that can be set to control the behavior of the channel.

All channels are independent from eachother, but listen to the same port as set in the global settings.

## Global
Each instance of Birdhouse has a port parameter that may be set. This is the port that Birdhouse listens for OSC messages on for all channels.

# Parameters

## Global parameters

- **Port**: The port to listen for OSC messages on.

## Channel parameters

- **Path**: The OSC path to listen for. The channel will only match messages with this path.
- **InMin**: The minimum value of the incoming OSC message. This is used to scale the incoming value to the MIDI range.
- **InMax**: The maximum value of the incoming OSC message. This is used to scale the incoming value to the MIDI range.
- **OutMin**: The minimum value of the outgoing MIDI message. This is used to scale the incoming value to the MIDI range.
- **OutMax**: The maximum value of the outgoing MIDI message. This is used to scale the incoming value to the MIDI range.
- **MIDIChan**: The output MIDI channel. 
- **MIDINum**: The output MIDI number (note number for notees, control number for control change).
- **MsgType**: The type of the message. This can be either `CC` for control change or `NOTE` for note on/off.
- **Mute**: Mutes the channel. When muted, the channel will not send any MIDI messages. This is useful when mapping it inside of your plugin host.

# Usage

## Sending OSC messages to Birdhouse

Birdhouse listens for OSC messages on a global (per instance) port and parses incoming messages according to the `Path` parameter of each channel.

These messages are expected to be:
- Either float or integer values (they are cast to floats internally for conversion purposes)
- Only one value per message. Any values beyond the first are ignored.


## Event types

A channel will behave quite differently depending on the `MsgType` parameter.

If `MsgType` is set to `CC`, the channel will send out a control change message which is simply a scaling of the incoming OSC message to the MIDI range.

If, on the other hand, `MsgType` is set to `NOTE`, the channel will send out a note on message when the incoming OSC message is above it's `InMin` value, and a note off message when it is below or at that threshold. The value itself is interpreted as the velocity of the note.


## Setup

Birdhouse is essentially a MIDI effect plugin. This means that it deos not produce sound in itself, and for this reason, some plugin hosts and DAW's have different setup requirements.

Below is a non-extensive list of how to set up Birdhouse in some popular DAW's and plugin hosts.

If you see a plugin host on here that is not covered, please feel free to open up an issue on the [github repository](https://github.com/madskjeldgaard/Birdhouse/issues/new) with specific instructions like the ones below.

NOTE: It is generally recommended to use the CLAP version of the plugin, if your host supports it, since the CLAP plugin protocol is more suited for MIDI effects.

### Receiving OSC in Reaper

![reaper routing](manual/reaperrouting.png) 

1. Create two tracks: One for Birdhouse, one for an instrument you want to control with birdhouse.
2. Add the Birdhouse plugin to the first track
3. Record enable the track (otherwise, the plugin will not receive any MIDI when you switch windows on your computer)
4. Add an instrument to the second track
5. Click and drag the "ROUTE" button to the second track to route the MIDI from Birdhouse to the instrument

### Receiving OSC in BitWig

![bitwig routing](manual/bitwigsetup.png)

#### Note on/off in a device

1. Create an instrument track
2. Add an instrument (for example "Polymer")
3. Add BirdHouse:
    - Click the NoteFX button to add BirdHouse as a MIDI effect inside of the device
    - Or: Click the `+` before the device and insert BirdHouse BEFORE the instrument (you may have to click "Show all sources" in the search bar to allow MIDI effects to be added)


#### Control change in a device

![bitwig midi cc routing](manual/bitwigcc.png) 

If you want to use a Birdhouse channel as a MIDI CC source, follow the steps above, and then:

1. Inside of Birdhouse within the channel you want to use, choose the `CC` message type.
2. Go back to the device in Bitwig you want to control and add a `MIDI` modulator.
    - Click the new MIDI modulator and press `Learn CC`
    - Send a value to the Birdhouse channel you want to control
    - Verify that it got picked up by the MIDI modulator
    - Map the midi modulator to something within the device

# Extra

## Sending OSC to Birdhouse from SuperCollider

A small example of sending OSC to Birdhouse from SuperCollider:

```supercollider
(
// Must match the port in the Birdhouse instance you want to target
~birdhousePort = 9999;

n = NetAddr.new("127.0.0.1", ~birdhousePort);
)

// Note ON
n.sendMsg("/1/value", 0.9);

// Note OFF
n.sendMsg("/1/value", 0.0);

// Send some random messages
r{
    loop{ 
        n.sendMsg("/1/value", rrand(0.0, 0.9)); 
        0.25.wait; 
        n.sendMsg("/1/value", 0.0); 
        0.25.wait
    }
}.play
```
