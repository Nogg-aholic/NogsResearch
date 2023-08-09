Recompile for Update 7 (CL208572) support. Config option to enable Research Accelerator lights vfx; now disabled by default for lag reasons. Bugfix for submitting research items too early in the save loadign process.




![Image](https://i.imgur.com/btFKmZt.jpg)

This update brought to you by Robb#6731

Side note, if you want this for U6 and not U7, ask for it on the discord (I sent it to Xynariz).

## New Stuff

- New config option - **Startup Item Submission Delay**
  - How soon (in seconds) the NogsResearch systems are allowed to start submitting items after the game save first loads it in.
  - If it submits items too soon, it could accidentally sumbit items bypassing research cost tweaks that other mods are applying.
  - It already waits until Game World PostInit phase, but you can increase this to add an arbitrary delay in case some mod makes it own changes later in the process.
  - Default 0.5 seconds, you should not have to change this.

## Changed Stuff

- New config option to enable Research Accelerator lights vfx; they are now disabled by default for lag reasons.

## Bugfixes

- Fixed that the Nog's Research subsystem was submitting items to milestones sooner than intended after being loaded in from a save file, allowing it to bypass other mods' milestone cost modifications (ex. Difficulty Tuner). Thanks Xynariz and SifVerT!
