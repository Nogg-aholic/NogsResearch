Update 6 support. UI overhaul. Research time reduction via Science Points. This update brought to you by Robb#6731






![Image](https://i.imgur.com/btFKmZt.jpg)

## Changes

* Update 6 support
* **You will have to reconstruct your built researchers - consider taking items out of them before you update!**
* New HUD element informs you of how long it is until your next HUB and MAM queue items can start
* Science Points System
  * Science Points reduce the amount of time you have to wait before the next research can start
  * You can see the time reduction in the HUB
  * Using the base-game HUB Terminal submission method still requires you to wait the full time period - more incentive to set up a robust researcher array.
* Now requires [MAM Enhancer](https://ficsit.app/mod/MAMTips) instead of being optional
  * The MAM queueing functionality requires it to be installed to work, which wasn't conveyed by anything in the past
* Improved HUB UI
  * Progress bars showing research progress for both HUB and MAM queues
  * Direct display of to what extent Science Points are reducing the wait time
  * Helpful tooltips and infotext for when and why queues/cost lists are empty
  * Info tooltips and visuals for how to submit different kinds of items (liquids/gasses must use a Fluid Researcher!)
  * Total Cost calculations in the HUB now list what Milestones and Nodes require that item
  * Improved Researcher summary
    * Displays science point contribution, power status, x/y/z location
    * Click on researchers in the HUB UI to ping their location in-world
* Milestone rebalance
  * Renamed the researcher unlock milestones to better match their purpose
  * Rebalanced cost of most milestones
    * Some of the later ones require fluid submitted by Fluid Researchers
    * You might have to unlock the research accelerators again
  * Changed the milestone dependencies so fewer of this mod's milestones are hidden based on prerequisites
    * Should make it easier to see that the Fluid Researcher exists, for example
* Increased Fluid Researcher Mk2 capacity to 20L (from 10L)
* Corrected order of researchers in the Build Menu
* Made the inside of the Research Accelerator Mk1 spin!
* All research machines can now be placed at gradual angles (like player storage boxes)
* Moved the power input points on some of the research machines
* Most research machines can now daisy-chain power connections
* The Research Accelerator Mk1 and Mk2 now have 5 and 7 storage slots respectively
  * This helps them serve as storage for uncommonly submitted research items.
* Evened out the points on the Science Power curve so they are round numbers (ex. 450 instead of 451.13401842), this doesn't really affect much, curve shape is otherwise the same.
* Changing the MAM queue must do something with the items left behind in the inventory. Previously, the mod would void them. Now, it tries to drop them by the HUB terminal. 
* Probably more I forgot to list here

## Bugfixes

* Fixed item duplication bug with researchers supplying items to the MAM buffer
* Should no longer leave a bunch of empty space inside the HUB UI. You can use a mod config setting to adjust this if there's still a gap.
* No longer visually shifts down the Launch button in the HUB UI
* Fix cost widgets defaulting to displaying "Alternate: Adhered Iron Plate" as their tooltip
* Researchers with items in them should no longer crash the game when dismantled
* Researcher tanks no longer display as full on water when first built
* Now on version `1.0.0` so other mods can list this as a dependency without the pain caused by `^` special behavior for major versions less than one

## Known Bugs

* **See the Multiplayer Compatibility Notes section of the modpage**
* It's possible to get duplicate entries in the queues by reordering items. They should correct themselves as researches complete.
* The Researcher storage slots have a custom size of 500 - when taking out items directly your personal inventory, you should use Take All or shift click, because dragging will void items.
* Fluid Researchers will not graphically update their displayed fluid count (in response to being taken by the research system) until fluid flow changes or they are interacted with.
* Schematic Dependency Tree viewing in the HUB (bottom left corner) is inconsistent, but vanilla and modded schematics very rarely use this feature regardless.

## Internals

* A ton of internal C++ code cleanup and commenting, some blueprint code cleanup.
* Changed the location of many assets to improve organization.
* Chat command `nogsresearch_curve` to select between a normal, disabled, and for-debugging Science Points to Time Reduction curve
* Chat command `nogsresearch_debugDumpBuffer` to manually dump the items from the MAM buffer onto the ground by the HUB
