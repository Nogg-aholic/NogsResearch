Modify the Queue Drag and Drop system to make it clearer where the "cursor" location is.



![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb#6731

## Changed Stuff

- When dragging and dropping queue items, the "anchor point" is now the **top left corner** instead of the **center of the dragged object**. You will now see a cursor visual to indicate where this point is located instead of having to guess at it.
  - The previous system was confusing because you had to drag-drop the top left corner of a schematic and then the "anchor point" was the center of the schematic
- The HUB's "Select Schematic" button is now fully hidden (use the queueing functionality instead)
  - This was done to remove the "red herring" option of interacting with the queue via this button, which was confusing to some players
  - I might add this button back later as a "add to end of queue" button.
- Dragging "new" schematics and nodes produces sound effects now, not just picking up existing queue items
- Dropping queue items produces sound

## Known Bugs

- Reordering the queue as a client can sometimes cause entries to duplicate, but the system will correct itself when the queue is processed, so this is a visual problem only
- Adding and removing items from the queue in a specific order can bypass MAM research tree dependencies
- When opened through sources other than the HUB Terminal (ex. the Remote Research Module in PowerSuit), the HUB interface can be inoperable until you use the regular HUB Terminal again
