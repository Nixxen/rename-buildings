[h1]Rename Buildings[/h1]

The mod allows you to rename all player-owned buildings in Kenshi. It was designed for renaming workstations (e.g., "Cooking Stove" -> "Cook Dustwich", and "Cooking Stove" -> "Cook Food Cube"), and it should work on other "building" objects as well.
Rename Buildings adds a small, draggable "Rename" button that appears when you select a player-owned building. Click it to open a compact rename window, type in a new name, and your building is renamed - permanently, even through saves and imports.

[b]Requirements[/b]
[list]
[*][url=https://www.nexusmods.com/kenshi/mods/847]RE_Kenshi[/url] (tested on v0.34)
[*]Kenshi v1.0.65+
[/list]

[b]How to Use[/b]
[list=1]
[*]Select a player-owned workstation "building" (Cooking Stove, Research Bench, Iron Refinery, Bughouse, Wall, Gate, etc.)
[*]Click the "Rename" button on the left side of the screen (or wherever you dragged it) to open the rename window
[*]Type in the new name in the edit box
[*]Click "Rename" to confirm, or click the X button to cancel
[/list]

[b]Features[/b]
[list]
[*][b]Selection-aware[/b] - The button only appears when a player-owned building is selected. It hides automatically when nothing is selected or when the selected building belongs to someone else.
[*][b]Draggable[/b] - Reposition the button by dragging it. It stays clamped to screen edges so it cannot be moved off-screen.
[*][b]Toggle behavior[/b] - Clicking the "Rename" button while the window is already open closes it.
[*][b]Ownership check[/b] - Only buildings owned by the player can be renamed. The rename action itself also verifies ownership.
[*][b]Persistent through saves[/b] - Renamed buildings keep their new name when saving and loading.
[/list]

[b]Limitations[/b]
[list]
[*]None that I know of. If you find something, get in the comments.
[/list]

[b]Compatibility[/b]
[list]
[*]No special load order required beyond RE_Kenshi
[*]Does not alter any game files or UI elements beyond the added button and window
[/list]

[b]Troubleshooting[/b]
[list]
[*]Confirm RE_Kenshi is installed and enabled.
[*]If the Rename button does not appear, verify you have selected a player-owned building.
[*]If the window does not disappear when clicking "Rename", you most likely have a blank/invalid name. Pure whitespace is not allowed.
[*]If the building did not update its name after the window closed, click on any other building or character to trigger the actual rename.
[/list]

[b]Shout outs[/b]
[list]
[*]BFrizzleFoShizzle, the creator of KenshiLib and RE_Kenshi. Without either, this mod would not exist.
[*][url=https://steamcommunity.com/id/anarkius]Anarkius[/url], for discovering building shells could be renamed in the game editor, giving me the debug info I needed to allow renaming buildings identifying as doors.
[/list]
