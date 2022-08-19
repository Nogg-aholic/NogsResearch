# Nog's Automated Research

Queue up HUB and MAM research, and automatically submit items towards them via new buildings! Reduce the time it takes to research via Science Power.

![Early Access version support: full](https://i.imgur.com/1TXo5em.png)
![Experimental version support: full](https://i.imgur.com/kvJ4ZoR.png)
![Singleplayer compatibility: full](https://i.imgur.com/S8roc0Y.png)
![Multiplayer compatibility: partial](https://i.imgur.com/GJh3Lcb.png)

![HubUI](https://i.imgur.com/Sa8xpW8.jpeg)

![BuildgunUI](https://i.imgur.com/2ceAyT2.jpeg)

Please report bugs on [on Discord](https://discord.gg/HT4w3qEGMQ) or [on GitHub](https://github.com/Nogg-aholic/NogsResearch/issues).

_This mod is currently being maintained by Robb#6731. Contact him if you want up to date source code._

## Features

- Queue up HUB Milestones and MAM nodes for automated research
- Automatically contribute items toward queued research via belts and pipes with Researcher buildings
- Construct Research Accelerators to reduce the wait time between researches
- Submit liquids and gasses directly as a research material with Fluid Researchers

![ResearchAcceleratorArrayCoolWheelSpokeThing](https://i.imgur.com/btFKmZt.jpg)

_Example Research Accelerator Setup_

## Getting Started

The HUB and MAM queueing functionality are available immediately upon installing the mod.
You can queue milestones or nodes by dragging them into the queue, and they can be reordered the same way.

You can unlock your first Researcher in Tier 0 of the HUB, which will allow automatic submission of items.

Fluid researchers are unlocked in Tier 3 of the HUB, which allows directly submitting Liquids and Gases toward research.

## Videos

Queue and Science Power demonstration

<video controls="" width="720" height="405">
  <source src="https://cdn.discordapp.com/attachments/623891487683510323/1009984077916557362/FactoryGame-Win64-Shipping_0Kqgil1aOS.mp4" autoplay="false" controls="true" type="video/mp4">
</video>

(Outdated!) Spotlight videos by LK Aice:

[NogsResearch Mod ✪ Satisfactory ✪ English](https://youtu.be/Ahy-6HYhBPs)

[NogsResearch Mod ✪ Satisfactory ✪ Deutsch](https://youtu.be/qciJZE3jvvs)

## Multiplayer Compatibility Notes

Unfortunately, multiplayer compatibility still needs more work, but the mod **will work without crashing in multiplayer**.

To summarize, clients can't easily manually submit items toward queued researches, but they can still view and edit the queue, as well as construct and operate the research buildings.

- Clients must interact with a researcher building in the world before they can remotely access its inventory from the HUB
- Clients don't get accurate info on if researchers are out of power
- Clients can't manually interact with the MAM queue inventory

## Known Bugs

- **See the Multiplayer Compatibility Notes section**
- The Researcher storage slots have a custom size of 500 - when taking out items directly your personal inventory, you should use Take All or shift click, because dragging will void items.
- Fluid Researchers will not graphically update their displayed fluid count (in response to being taken by the research system) until fluid flow changes or they are interacted with.
- It's possible to get duplicate entries in the queues by reordering items. They should correct themselves as researches complete.
- Schematic Dependency Tree viewing in the HUB is inconsistent, but vanilla and modded schematics very rarely use this feature regardless.

## Credits

Machine models modified from the [Science Lab asset pack](https://www.unrealengine.com/marketplace/en-US/product/science-laboratory) & Epic for making this a free Asset for a limited period of time.

Original Mod Author: Nog#6605

Robb - Testing & Balance feedback, UI overhaul, multiplayer support, Update 5 and 6 support

Schematic Icon by [Lorc](http://lorcblog.blogspot.com/ "lorcblog") from [game-icons.net](https://game-icons.net "game-icons")

Mod Icon by Deantendo#4265

<a target="_blank" href="https://icons8.com/icon/mISgYgMK6Z2X/hand-drag">Hand Drag</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
