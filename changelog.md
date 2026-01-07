# RevisedLevelCells Changelog
## v1.3.2
- Minor changes to allow fetching a previously saved level's Song IDs and SFX IDs outside of the Saved Levels menu.
- Fix a minor bug that led to the mod displaying redundant Song ID information if a level uses only one Song ID but at least one SFX ID. 
## v1.3.1
- Added song info support for levels using songs from vanilla GD.
- Added song ID display support for levels using multiple songs.
- Added slightly stricter account ID enforcement for ignoring/favoriting users.
## v1.3.0
- Added options to treat your friends list in GD as favorite people and your list of blocked users in GD as ignored people.
- Added options to *truly* hide levels/level lists from ignored people (requested by [Nightcat](https://github.com/Nightcaat)).
- Added logging options, because why not.
## v1.2.1
- Properly fix the previously mentioned nasty bug where level descriptions were misaligned when viewing levels from a level list, as the previous fix did jack squat.
## v1.2.0
- Fix a nasty bug where level descriptions were misaligned when viewing levels from a level list.
- Removed `Remove Level List Placement` support for editable level lists.
- Experimental iOS support.
## v1.1.2
- Some [adjective describing a person] reported a "crash" that shouldn't even be possible, so here we are.
## v1.1.1
- Fix the consistency where level placements for level cells weren't being removed in some edge cases.
- Refactor a few things here and there.
- Added Jane Wickline.
  - Probably the most controversial change to any of my mods; this specific change might get reverted in the next update. We'll find out together in August.
## v1.1.0
- Added personal phrases filter and ignored/favorite users systems!
  - Also supports LevelListCells!
  - Additional options to hide LevelCells and LevelListCells of rated levels/lists.
  - There will not be a "syncing with VariousCommentTweaks" option.
  - There will also not be a "ignore specific level ID" option.
- Expand `Readable chatFont Labels` support to LevelListCells.
- Softtoggle to disable that this mod makes on LevelListCells specifically.
- Partially known issue: level list placement removal might not 100% work. A consistent patchfix for this bug will come when it's ready.
## v1.0.4
- A quick oneliner hotfix for a bug report from [Terma](https://github.com/Termantita).
## v1.0.3
- Redo approach to removing level list placement by re-using Cvolton's code, but applying it to all level cells.
  - As a result, the Compact Lists dependency has been removed.
  - This was necessary to ensure proper compatibility with upcoming changes from the Level Thumbnails mod.
## v1.0.2
- Some cleanup from behind the scenes. Nothing noticeable for end users, though.
## v1.0.1
- Port to 2.2074.
## v1.0.0
- Initial release (on GitHub).