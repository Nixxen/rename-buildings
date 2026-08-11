[h1]Rename Buildings[/h1]
Rename Buildings lets you rename all player-owned buildings in Kenshi.

It was primarily created for renaming workstations so they are easier to identify at a glance. For example, you can rename one Cooking Stove to "Cook Dustwich" and another to "Cook Food Cube". Both player-owned building-type objects from the build menu and purchased buildings shells can be renamed.
Selecting a supported building displays a small draggable Rename button. Clicking it opens a compact rename window where you can enter a custom name. Renamed buildings keep their names permanently, including after saving, loading and importing.
The mod is configurable through a JSON file, or Emkej's Mod Hub (under the UI section).

[b]How to Use[/b]
[list=1]
[*]Select a player-owned "building" (Cooking Stove, Research Bench, Iron Refinery, Bughouse, Wall, Gate, etc.)
[*]Click the "Rename" button on the left side of the screen (or wherever you dragged it) to open the rename window
[*]Type in the new name in the edit box
[*]Click "Rename" to confirm, or click the X button to cancel
[*]Click on any other object to refresh the displayed name
[/list]

[b]Features[/b]
[list]
[*][b]Rename all player-owned buildings[/b] - Buildable objects or building shells (rename configuration can be overridden)
[*][b]Persistent through saves[/b] - Renamed buildings keep their new name when saving, loading, and importing.
[*][b]Draggable[/b] - Reposition the button by dragging it. It stays clamped to screen edges so it cannot be moved off-screen.
[*][b]Selection-aware[/b] - The button only appears when a player-owned building is selected. It hides automatically when nothing is selected or when the selected building belongs to someone else.
[*][b]Toggle behavior[/b] - Clicking the "Rename" button while the window is already open closes it.
[*][b]Ownership check[/b] - Only buildings owned by the player can be renamed. The rename action itself also verifies ownership.
[*][b]Configurable[/b] - Supports config through JSON (out of game) or Emkej's Mod Hub (in game). You can now override which buildings you are allowed to rename, allowing you to limit the amount of buildings you can rename. You can also override the player check, so non-player buildings can be renamed.
[/list]

[b]Requirements[/b]
[list]
[*][url=https://www.nexusmods.com/kenshi/mods/847]RE_Kenshi[/url] (tested on v0.34)
[*][url=https://www.nexusmods.com/kenshi/mods/1885]Emkejs-Mod-Core[/url] (optional, for Mod Hub in-game settings UI; load before RenameBuildings)
[*]Kenshi v1.0.65+
[/list]

[b]Limitations[/b]
[list]
[*]None that I know of. If you find something, get in the comments.
[/list]

[b]Compatibility[/b]
[list]
[*]No special load order required beyond RE_Kenshi
[*]If using Emkejs Mod Core/Mod Hub for configuration, load it before this mod
[*]Does not alter any game files or UI elements beyond the added button and window
[/list]

[b]Troubleshooting[/b]
[list]
[*]Confirm RE_Kenshi is installed and enabled.
[*]If the Rename button does not appear, verify you have selected a player-owned building or have not overridden any building selections in the configuration.
[*]If the window does not disappear when clicking "Rename", you most likely have a blank/invalid name. Pure whitespace is not allowed.
[*]If the building did not update its name after the window closed, click on any other building or character to trigger the actual rename.
[*]If you have overridden a building type in the JSON configuration, but it does not correctly hide the in-game button, ensure you have also enabled the flag that allows overriding in general. The main flag is required for any of the specific overrides to work.
[/list]

[b]Check Out All My Mods[/b]
[list]
[*][url=https://steamcommunity.com/sharedfiles/filedetails/?id=3759303115]Rename Buildings[/url], Lets you rename all player-owned buildings in Kenshi.
[*][url=https://steamcommunity.com/sharedfiles/filedetails/?id=3771008243]Simple Compass[/url], Never lose your bearings in Kenshi again. A compass HUD element showing your current heading in degrees and cardinal direction.
[*][url=https://steamcommunity.com/sharedfiles/filedetails/?id=3775703318]Fix Shopping Wages[/url], gives most NPCs their daily allowance so they keep buying from player shops.
[*][url=https://steamcommunity.com/sharedfiles/filedetails/?id=3781354452]Medic Button Splint Rigging[/url], Adds the "Splint Rigging" job when shift-clicking the medic button.
[/list]

[b]Shout outs[/b]
[list]
[*][url=https://steamcommunity.com/id/bmanatee]BFrizzleFoShizzle[/url], creator of KenshiLib and RE_Kenshi. Without either, this mod would not exist.
[*][url=https://steamcommunity.com/id/anarkius]Anarkius[/url], for discovering building shells could be renamed in the game editor, giving me the debug info I needed to allow renaming buildings identifying as doors.
[*][url=https://steamcommunity.com/profiles/76561198014968620]Emkej[/url], creator of Emkejs-Mod-Core for Mod Hub and in-game settings UI
[/list]
