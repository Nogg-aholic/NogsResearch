Fixed alignment of the HUB "milestone rewards" grid




![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb#6731

## Bugfixes

- Fixed alignment of the HUB "milestone rewards" grid (it's no longer stuck on the left)
  - SML used to make the rewards grid scrollable and centered,
    but now the base game does that by default,
    so the code had to be updated to account for this.

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
- When opened through sources other than the HUB Terminal (ex. the Remote Research Module in PowerSuit), the HUB interface can be inoperable until you use the regular HUB Terminal again
