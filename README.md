# *Cute Animation Editor*
- ### How can I use it?
	- #### Hotkeys
		>**F1** - to change the mode of movement of the rectangles: in-game coordinates or pixels
		>**F2** - Switch to Edit mode
		>**Mouse wheel** - zooming the camera (only when ```move pen``` is selected)
		>Switching tools:
			**1** - move pen
			**2** - move part
			**3** - selection
			**4** - magic 
	 - #### Editor
		1. *Creator Mode* - in this mode, you can see all the editing elements
		2. *TAP window* - in this window you can see what the animation looks like
			- Left Part
			All created animation groups can be found here and you can select a group to view from them
			- Right Part
			There is a window for viewing the animation and tabs for changing some animation parameters
		3. *Animation Groups* - here you can divide animations into groups by their type and purpose(walk, run, idle, etc.)
		#### Tabs:
		- Load Asset
			Just enter the file name (*.json), the asset will be loaded and will be available in the [Loaded Assets](#loaded-assets) tab
		
		- New
			Creates an asset.
			- name
				Name of the asset
			- Texture path
				As a texture, you can specify a single file or a group of files (by activating the ```make by group``` flag) each group is a set of files (file should contain only one frame), which will then be automatically combined into a single texture.
			- Output file
				file  .json where all information about the asset will be stored.
		
		- Window Settings
			This tab contains the camera settings.
		
		- Loaded Assets
			Here you can set the current asset.
			
		- Magic tool
			Here you can change some settings for the ```magic tool``` and see what the texture that opencv works with looks like.
			
		- Load asset
			Here you can view all the cassettes that are in the directory ```./assets```.
		
	- #### Tools:
		- move pen
			Moves the camera.
		- move part
			Moves rectangles, the selected rectangle can be moved not only with the mouse but also with the arrows. Holding down ```Lctrl``` and clicking on the selected rectangle can cancel the selection, pressing ```esc``` can cancel all selections. All selected rectangles are moved!
		- selection
			You can select multiple rectangles to change them in the far.
		- Magic Tool
			Automatically sets rectangles for animation. To start using this tool you need to go to the ```Edit Asset``` tab and select the group to which the resulting rectangles will be added.
		
###### This was made for [sfml_engine](https://github.com/NePutin94/sfml_engine) 🥰 