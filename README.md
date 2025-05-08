# MayaLayerStack
_MayaLayerStack_ is a plug-in which implements the paper _Efficient Rendering of Layered Materials using an Atomic Decomposition with Statistical Operators by Laurent Belcour (Unity Technologies, 2018)._
It consists of a material designer/applicator interface for Maya, as well as the underlying Arnold plugin which handles the BSDF logic.

# Basic Guide

Step 1: Put everything in place
----
Maya Side: 
Download the built LayerStackPlugin.mll and place it in your devkit/plug-ins/plug-ins/ folder. This is what will be loaded with Maya’s plugin manager.
Optional: Download the “presets” folder, and place it in the same directory. 
Download the layer_stack_ui.py file and place it in <devkit path>/plug-ins/scripts so that the mll can see it. 

Arnold Side: Download the contents from the MayaLayerStack/ArnoldPlugin/plugin/ folder and place it in the directory pointed at by the ARNOLD_PLUGIN_PATH environment variable.

_At this point, after opening Maya’s hypershade window (Windows/Rendering Editors/Hypershade…) you should see the mlsLayer* materials when searching for them._

_You should also see LayerStackPlugin.mll in your Plug-In Manager window under the devkit path._

Step 2: Load LayerStack Plugin through the plug-in manager.
----
Open Maya’s plug-in manager and load the LayerStackPlugin.mll.

Step 3: Create a material or load a preset
----
Using the material designer on the right-hand side, either design your own material or load an included preset, such as CarbonFiber. We recommend having your presets folder next to your .mll application so that the preset view will be loaded automatically and any new presets should go in this folder.

Step 4: Apply the material
----
Select a mesh/shape in Maya, then click the “select” button in the UI. Once the shape’s name shows up, press “Apply Material” and you should see it become a light-gray color. 

Step 5: Render with Arnold
----
Render the scene with Arnold and you should see the result of your material.
If building the scene from scratch, ensure you have Arnold-compatible lighting and surfaces in the scene.
To quickly verify the material without having to build the scene, simply open the hypershade window, and change the renderer to “Arnold” to verify on the maya-included stock objects. 
