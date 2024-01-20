Improved logging to discern cause of a non-fatal error in a user-submitted log




![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb.
If you enjoy my work, please consider my [completely optional tip jar](https://ko-fi.com/robb4).

## Bugfixes

- Fixed that log messages would display BlueprintGeneratedClass instead of the actual schematic name

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
- When opened through sources other than the HUB Terminal (ex. the Remote Research Module in PowerSuit), the HUB interface can be inoperable until you use the regular HUB Terminal again
