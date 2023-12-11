Multiplayer bugfixes, improved visualization of "waiting on return of hub ship" status




![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb.
If you enjoy my work, please consider my [completely optional tip jar](https://ko-fi.com/robb4).

## Changed Stuff

- Clarified default options in the mod's configuration options
- Your setting for 'Hide Purchased Milestones' is now stored in the mod's config, meaning it persists across save/load
- When waiting for the HUB ship to return, the "Current Queue Item" slot (previously empty) will now say "(Waiting for Ship)"

## Bugfixes

- Fixed 'Hide Purchased Milestones' option not working for multiplayer clients
- Cleaned up some debugging log calls

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
- When opened through sources other than the HUB Terminal (ex. the Remote Research Module in PowerSuit), the HUB interface can be inoperable until you use the regular HUB Terminal again
