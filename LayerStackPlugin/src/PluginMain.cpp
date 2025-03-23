#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

#include "LayerStackCmd.h"

#define LAYERSTACK_NAME_STR "LayerStack"
#define LAYERSTACK_MENU_STR "LayerStackMenu"
#define LAYERSTACK_CONTROLS_LABEL "Layer Material Controls"

void createMenuItem()
{
    MGlobal::executePythonCommand("import maya.cmds as cmds");

    // Remove any existing menu first to prevent duplicates
    MGlobal::executePythonCommand("if cmds.menu('" LAYERSTACK_MENU_STR "', exists=True): cmds.deleteUI('" LAYERSTACK_MENU_STR "')");

    MGlobal::executePythonCommand(
        "cmds.menu('" LAYERSTACK_MENU_STR "', label='" LAYERSTACK_NAME_STR "', parent='MayaWindow')\n"
        "cmds.menuItem(label='" LAYERSTACK_CONTROLS_LABEL "', parent='" LAYERSTACK_MENU_STR "', command=lambda x:layer_stack_ui.create_ui())");
}

void removeMenuItem()
{
    // Remove any existing menu
    MGlobal::executePythonCommand("if cmds.menu('" LAYERSTACK_MENU_STR "', exists=True): cmds.deleteUI('" LAYERSTACK_MENU_STR "')");

    // Destroy open windows
    MGlobal::executePythonCommand(
        "import layer_stack_ui\n"
        "layer_stack_ui.cleanup_ui()");
}

MStatus initializePlugin( MObject obj )
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "MyPlugin", "1.0", "Any");

    // Register Command
    status = plugin.registerCommand( LayerStackCmd::name(), LayerStackCmd::creator);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    // Load python scripts
    MString pluginPath = plugin.loadPath() + "/../scripts";

    MString command = "import sys\n"
        "plugin_path = r'" + pluginPath + "'\n"
        "if plugin_path not in sys.path:\n"
        "    sys.path.append(plugin_path)\n"
        "import layer_stack_ui";

    MGlobal::executePythonCommand(command);

    // Add custom menu item
    createMenuItem();

    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

    // Cleanup menu item.
    removeMenuItem();

    status = plugin.deregisterCommand( LayerStackCmd::name() );
    if (!status) {
	    status.perror("deregisterCommand");
	    return status;
    }

    return status;
}


