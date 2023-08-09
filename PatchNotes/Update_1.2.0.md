Update 8 support. Better handling for when the NoPower cheat changes state.



![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb#6731



## IMPORTANT

After updating, you may need to load your save, save it, and then load that save again, if your Researchers aren't submitting items correctly.

## New Stuff

- Your selected value for "hide purchased schematics" in the HUB is remembered between opening/closing the HUB
  - Thanks Dovahkin for the suggestion

## Changed Stuff

- Modified how researchers are kept track of by the mod, meaning it should not require a save reload when the No Power cheat is enabled or disabled.
- The research subsystem is now a ModSubsystem, which simplifies the code but is the cause of possibly needing to reload your save once (see 'Important' heading) to migrate the old one correctly
- Hide Purchased Schematics button is now below the current queue item so it doesn't overlap the schematic list

## Bugfixes

- Probably fixed some edge cases of researchers not contributing their items correctly

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
