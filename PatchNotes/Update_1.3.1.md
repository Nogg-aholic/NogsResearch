Fix HUD wait times not updating due to U8 changes. Optimization.



![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb#6731

## Changed Stuff

- Moved the `CanProduce` implementation to C++ instead of blueprint, which improves performance

## Bugfixes

- Fixed that the "next research can begin in" timers in the HUD did not visually update

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
- When opened through sources other than the HUB Terminal (ex. the Remote Research Module in PowerSuit), the HUB interface can be inoperable until you use the regular HUB Terminal again
