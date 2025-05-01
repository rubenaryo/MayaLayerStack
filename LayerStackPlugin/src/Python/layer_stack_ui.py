import maya.cmds as cmds
import maya.mel as mel
import json
import os
import glob

# Global variables to track the layer structure
layer_tree = {"root": {"type": "root", "children": []}}
layer_counter = 0
selected_param_layer = None  # Track which parameter layer is currently being edited

def create_preset_buttons():
    # Get the directory of the current script
    script_path = cmds.pluginInfo("LayerStackPlugin.mll", query=True, path=True)

    if not script_path:
        # If we can't get the path directly, try to get the currently executing script
        script_path = mel.eval('getenv("MAYA_SCRIPT_PATH")').split(';')[0]
    
    global preset_dir
    preset_dir = os.path.dirname(script_path)
    
    # Find all JSON files in the same directory
    json_files = glob.glob(os.path.join(preset_dir, "*.json"))
    
    # Create buttons for each JSON file
    if json_files:
        for json_file in json_files:
            json_file = os.path.normpath(json_file)
            file_name = os.path.basename(json_file)
            preset_name = os.path.splitext(file_name)[0]
            #print(json_file)

            # Create button with a different color to distinguish from other buttons
            cmds.button(
                label=preset_name,
                command=lambda x, path=json_file: load_layer_structure(path),
                #backgroundColor=[0.2, 0.5, 0.7],
                height=30
            )
    else:
        cmds.text(label="No presets found", align="center")


def create_ui():
    # Check if the window already exists, if so, delete it
    if cmds.window("meshSelectionUI", exists=True):
        cmds.deleteUI("meshSelectionUI")
    
    # Create the main window
    window = cmds.window("meshSelectionUI", title="Layer Material Controls", width=800, height=500)
    
    # Create the main layout as a form layout
    main_layout = cmds.formLayout(numberOfDivisions=100)
    
    # Create left panel for mesh selection
    left_panel = cmds.columnLayout("leftPanel", adjustableColumn=True, columnAttach=('both', 5), rowSpacing=10, width=200)
    cmds.text(label="Selected Mesh:", align="left", font="boldLabelFont")
    mesh_field = cmds.textField("selectedMeshField", editable=False)
    cmds.button(label="Select Mesh", command=select_mesh)
    cmds.separator(height=20, style='in')
    cmds.button(label="Apply Material", command=apply_function, height=50, backgroundColor=[0.2, 0.7, 0.2])
    cmds.button(label="Save Layer Structure", command=save_layer_structure)
    cmds.button(label="Load Layer Structure", command=load_from_file)

    # Preset section
    preset_separator = cmds.separator(height=20, style='in', horizontal=True)
    #preset_frame = cmds.frameLayout(label="Material Presets", collapsable=True, collapse=False)
    cmds.text(label="Material Presets", font="boldLabelFont", align="center")
    #preset_frame = cmds.scrollLayout(height=600, width=200, horizontalScrollBarThickness=0)
    #cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=5)
    cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), width=250, rowSpacing=3)

    create_preset_buttons()

    
    # Exit columnLayout
    cmds.setParent('..')

    # Exit preset_frame
    #cmds.setParent('..')

    # Exit Left Panel
    cmds.setParent('..')
    
    separator1 = cmds.separator(style='in', horizontal=False)
    
    # Right panel is for layer management
    right_panel = cmds.columnLayout("rightPanel", adjustableColumn=True, columnAttach=('both', 5), width=800, rowSpacing=10)
    cmds.text(label="Material Layer Structure", font="boldLabelFont", align="center")
    
    cmds.frameLayout(label="Layer Controls", collapsable=True, collapse=False)
    cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=5)
    
    cmds.button(label="Add New Material", command=add_new_material)
    
    cmds.setParent('..')
    cmds.setParent('..')
    
    # Create a scroll layout for the layer tree visualization
    global layer_tree_scroll
    layer_tree_scroll = cmds.scrollLayout(horizontalScrollBarThickness=16, verticalScrollBarThickness=16, height=800)
    
    # Create a column layout to hold the layer tree
    global layer_tree_column
    layer_tree_column = cmds.columnLayout("layerTreeColumn", adjustableColumn=True, columnAttach=('both', 5), width=800, rowSpacing=5)
    
    refresh_layer_tree_ui()
    
    cmds.setParent('..')  # Exit scroll layout
    cmds.setParent('..')  # Exit right panel
    
    # Set up the form layout
    cmds.formLayout(main_layout, edit=True,
               attachForm=[(left_panel, 'top', 5), (left_panel, 'left', 5), (left_panel, 'bottom', 5),
                          (separator1, 'top', 5), (separator1, 'bottom', 5),
                          (right_panel, 'top', 5), (right_panel, 'right', 5), (right_panel, 'bottom', 5)],
               attachPosition=[(left_panel, 'right', 5, 30)],
               attachControl=[(separator1, 'left', 2, left_panel), 
                             (right_panel, 'left', 2, separator1)])
    
    # Show the window
    cmds.showWindow(window)

def add_add_node(parent_id, position=None, *args):
    global layer_counter

    # Verify the parent is an add node or surface node
    parent_type = layer_tree[parent_id]["type"]
    if parent_type != "add" and parent_type != "surface":
        cmds.warning("add nodes can only be added to surfaces or other add nodes")
        return

    # Create add node as child of surface node
    layer_counter += 1
    add_node_id = f"layer_{layer_counter}"
    
    # Add the add-node to the surface
    layer_tree[parent_id]["children"].append(add_node_id)
    layer_tree[add_node_id] = {
        "type": "add",
        "parent": parent_id,
        "children": [],
        "params": {"name": f"Add_{layer_counter}"},
        "top_layer": None,
        "bottom_layer": None
    }
    
    # If the parent is an add node, set the id reference
    if parent_type == "add" and position is not None:
        if position == "top":
            layer_tree[parent_id]["top_layer"] = add_node_id
        else: 
            layer_tree[parent_id]["bottom_layer"] = add_node_id
    
    refresh_layer_tree_ui()

def add_new_material(*args):
    global layer_counter
    
    # Create a prompt dialog for the surface name
    result = cmds.promptDialog(
        title='New Material',
        message='Enter material name:',
        text='New Multi-Layer Material',
        button=['OK', 'Cancel'],
        defaultButton='OK',
        cancelButton='Cancel',
        dismissString='Cancel'
    )
    
    if result == 'OK':
        material_name = cmds.promptDialog(query=True, text=True)
        
        # Create surface node
        layer_counter += 1
        surface_id = f"layer_{layer_counter}"
        
        # Add the surface node to the root
        layer_tree["root"]["children"].append(surface_id)
        layer_tree[surface_id] = {
            "type": "surface",
            "parent": "root",
            "children": [],
            "params": {"name": material_name}
        }
        
    # Automatically add an add-node upon creating a new material
    add_add_node(surface_id, None)

def add_parameter_layer(parent_id, position, *args):
    global layer_counter   
    
    # Create a dialog to select layer type
    result = cmds.confirmDialog(
        title=f'Select {position.capitalize()} Layer Type',
        message=f'Choose the type of layer for the {position} position:',
        button=['Dielectric', 'Volumetric', 'Metal', 'Add', 'Cancel'],
        defaultButton='Dielectric',
        cancelButton='Cancel',
        dismissString='Cancel'
    )
    
    if result == 'Cancel':
        return

    if result == 'Add':
        add_add_node(parent_id, position, *args)
        return

    # Verify the parent is an add node
    if layer_tree[parent_id]["type"] != "add":
        cmds.warning("Parameter layers can only be added to add nodes")
        return
    
    # Check if the position is already filled
    if (position == "top" and layer_tree[parent_id]["top_layer"] is not None) or \
       (position == "bottom" and layer_tree[parent_id]["bottom_layer"] is not None):
        cmds.warning(f"This add node already has a {position} layer")
        return
    
    # Create a prompt dialog for the layer name
    name_result = cmds.promptDialog(
        title=f'New {result} Layer',
        message='Enter layer name:',
        text=f'New{result}',
        button=['OK', 'Cancel'],
        defaultButton='OK',
        cancelButton='Cancel',
        dismissString='Cancel'
    )
    
    if name_result != 'OK':
        return
        
    layer_name = cmds.promptDialog(query=True, text=True)
    
    # Create the new layer
    layer_counter += 1
    layer_id = f"layer_{layer_counter}"
    
    # Add the layer to the parent's children list
    layer_tree[parent_id]["children"].append(layer_id)
    
    # Set the top or bottom reference
    if position == "top":
        layer_tree[parent_id]["top_layer"] = layer_id
    else:  # position == "bottom"
        layer_tree[parent_id]["bottom_layer"] = layer_id
    
    # Create the layer data with name parameter
    params = create_default_params(result.lower())
    params["name"] = layer_name
    
    layer_tree[layer_id] = {
        "type": result.lower(),
        "parent": parent_id,
        "children": [],
        "params": params,
        "position": position
    }
    
    # Refresh the UI
    refresh_layer_tree_ui()

def add_sublayer_add_node(parent_id, *args):
    global layer_counter
    
    # Verify the parent is a parameter layer
    if layer_tree[parent_id]["type"] in ["dielectric", "volumetric", "metal"]:
        layer_counter += 1
        add_node_id = f"layer_{layer_counter}"
        
        # Add the add-node to the parent
        layer_tree[parent_id]["children"].append(add_node_id)
        layer_tree[add_node_id] = {
            "type": "add",
            "parent": parent_id,
            "children": [],
            "top_layer": None,
            "bottom_layer": None
        }
        
        # Refresh the UI
        refresh_layer_tree_ui()
    else:
        cmds.warning("Add nodes can only be added to parameter layers")

def create_default_params(layer_type):
    if layer_type == "dielectric":
        return {"IOR": 1.5, "roughness": 0.1}
    elif layer_type == "volumetric":
        return {"albedo": [1.0, 1.0, 1.0], "depth": 1.0}
    elif layer_type == "metal":
        return {"albedo": [0.8, 0.8, 0.8], "IOR": 2.5, "kappa": 1.0, "roughness": 0.1}
    return {}

def refresh_layer_tree_ui():
    # Delete and recreate the layer tree column layout to ensure clean slate
    if cmds.layout("layerTreeColumn", exists=True):
        cmds.deleteUI("layerTreeColumn")
    
    # Recreate the column layout for the layer tree
    cmds.setParent(layer_tree_scroll)
    global layer_tree_column
    layer_tree_column = cmds.columnLayout("layerTreeColumn", adjustableColumn=True, columnAttach=('both', 5), rowSpacing=5)
    
    # Create the layer tree UI from the root
    for child_id in layer_tree["root"]["children"]:
        create_layer_ui(child_id, 0)
    
    cmds.setParent('..')  # Exit column layout

def create_layer_ui(layer_id, indent_level):
    """Creates UI elements for a single layer and recursively for its children"""
    layer_type = layer_tree[layer_id]["type"]
    
    # Get layer name if available
    layer_name = ""
    if "params" in layer_tree[layer_id] and "name" in layer_tree[layer_id]["params"]:
        layer_name = layer_tree[layer_id]["params"]["name"]
    
    # Create frame label based on layer type
    if layer_type == "surface":
        frame_label = f"Surface: {layer_name}"
        bg_color = [0.3, 0.3, 0.4]  # Darker color for surface nodes
    elif layer_type == "add":
        frame_label = f"Add: {layer_name}"
        bg_color = [0.25, 0.25, 0.35]  # Medium color for add nodes
    else:  # Parameter layers
        position_text = ""
        if "position" in layer_tree[layer_id]:
            position_text = f" ({layer_tree[layer_id]['position'].capitalize()})"
        frame_label = f"{layer_type.capitalize()}: {layer_name}{position_text}"
        bg_color = [0.2, 0.2, 0.3]  # Lighter color for parameter layers
    
    # Create a frame for this layer with appropriate indentation
    cmds.frameLayout(
        label=frame_label,
        collapsable=True,
        collapse=False,
        marginWidth=5,
        marginHeight=5,
        backgroundColor=bg_color,
        labelVisible=True,
        font="boldLabelFont"
    )
    
    # Use a column layout with left margin based on indent level
    cmds.columnLayout(adjustableColumn=True, columnAttach=('left', 20 * indent_level), width=800, rowSpacing=3)
    
    # Create buttons row based on layer type
    if layer_type == "surface":
        #cmds.rowLayout(numberOfColumns=2, adjustableColumn=True, columnAttach=[(1, 'left', 5), (2, 'right', 5)])
        cmds.button(label="Edit Name", command=lambda x, lid=layer_id: add_parameter_editors(lid))
        cmds.button(label="Remove", command=lambda x, lid=layer_id: remove_layer(lid), backgroundColor=[0.5, 0.2, 0.2])
        
        child_list = layer_tree[layer_id].get("children")
        if not child_list:
            cmds.setParent('..')
            cmds.button(label="Add Node", command=lambda x, lid=layer_id: add_add_node(lid))

        cmds.setParent('..')
    
    elif layer_type == "add":
        cmds.button(label="Remove", command=lambda x, lid=layer_id: remove_layer(lid), backgroundColor=[0.5, 0.2, 0.2])
        cmds.setParent('..')
        
        # Add top layer section
        cmds.separator(height=5, style='none')
        cmds.rowLayout(numberOfColumns=2, columnWidth2=(100, 200), columnAttach=[(1, 'left', 5), (2, 'left', 5)])
        cmds.text(label="Top Layer:", font="boldLabelFont", align="right")
        top_layer = layer_tree[layer_id].get("top_layer")
        if top_layer:
            top_name = layer_tree[top_layer]["params"].get("name", "Unnamed")
            cmds.text(label=f"{layer_tree[top_layer]['type'].capitalize()}: {top_name}")
        else:
            cmds.button(label="Add Top Layer", command=lambda x, lid=layer_id: add_parameter_layer(lid, "top"))
        cmds.setParent('..')
        
        # Add bottom layer section
        cmds.rowLayout(numberOfColumns=2, columnWidth2=(100, 200), columnAttach=[(1, 'left', 5), (2, 'left', 5)])
        cmds.text(label="Bottom Layer:", font="boldLabelFont", align="right")
        bottom_layer = layer_tree[layer_id].get("bottom_layer")
        if bottom_layer:
            bottom_name = layer_tree[bottom_layer]["params"].get("name", "Unnamed")
            cmds.text(label=f"{layer_tree[bottom_layer]['type'].capitalize()}: {bottom_name}")
        else:
            cmds.button(label="Add Bottom Layer", command=lambda x, lid=layer_id: add_parameter_layer(lid, "bottom"))
        cmds.setParent('..')
    
    else:  # Parameter layers (dielectric, volumetric, metal)
        add_parameter_editors(layer_id)
        cmds.button(label="Remove", command=lambda x, lid=layer_id: remove_layer(lid), backgroundColor=[0.5, 0.2, 0.2])
        cmds.setParent('..')
    
    # Now recursively create UI for children
    for child_id in layer_tree[layer_id]["children"]:
        # For each child, increment the indent level and create its UI
        # - First create a separator for visual clarity
        cmds.separator(height=5, style='none')
        # - Then create the child's UI with increased indentation
        create_layer_ui(child_id, indent_level + 1)
    
    cmds.setParent('..')  # Exit column layout
    cmds.setParent('..')  # Exit frame layout

def add_parameter_editors(layer_id, *args):
    # Create a new window for parameter editing
    
    layer_type = layer_tree[layer_id]["type"]
    layer_name = layer_tree[layer_id]["params"].get("name", "Unnamed Layer")
    
    # Add type-specific parameters
    if layer_type == "dielectric":
        cmds.frameLayout(label="Dielectric Parameters", collapsable=False, marginHeight=10)
        cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=10)

        ior_slider_name = f"iorSlider{layer_id}"
        rough_slider_name = f"roughSlider{layer_id}"
        
        # IOR parameter
        cmds.floatSliderGrp(
            ior_slider_name,
            label='IOR: ',
            value=layer_tree[layer_id]["params"]["IOR"],
            field=True,
            minValue=1.0,
            maxValue=3.0,
            fieldMinValue=1.0,
            fieldMaxValue=10.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "IOR", val)
        )
        
        # Roughness parameter
        cmds.floatSliderGrp(
            rough_slider_name,
            label='Roughness: ',
            value=layer_tree[layer_id]["params"]["roughness"],
            field=True,
            minValue=0.0,
            maxValue=1.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "roughness", val)
        )
        cmds.setParent('..')
        
    elif layer_type == "volumetric":
        cmds.frameLayout(label="Volumetric Parameters", collapsable=False, marginHeight=10)
        cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=10)
        
        albedo_slider_name = f"albedoSlider{layer_id}"
        depth_slider_name = f"depthSlider{layer_id}"

        # Albedo parameter (color)
        cmds.colorSliderGrp(
            albedo_slider_name,
            label='Albedo: ',
            rgb=(layer_tree[layer_id]["params"]["albedo"][0], 
                layer_tree[layer_id]["params"]["albedo"][1], 
                layer_tree[layer_id]["params"]["albedo"][2]),
            changeCommand=lambda *args, lid=layer_id: update_color_param(lid, "albedo", albedo_slider_name)
        )
        
        # Depth parameter
        cmds.floatSliderGrp(
            depth_slider_name,
            label='Depth: ',
            value=layer_tree[layer_id]["params"]["depth"],
            field=True,
            minValue=0.0,
            maxValue=10.0,
            fieldMinValue=0.0,
            fieldMaxValue=100.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "depth", val)
        )
        cmds.setParent('..')
        
    elif layer_type == "metal":
        cmds.frameLayout(label="Metal Parameters", collapsable=False, marginHeight=10)
        cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=10)

        albedo_slider_name = f"albedoSlider{layer_id}"
        ior_slider_name = f"iorSlider{layer_id}"
        kappa_slider_name = f"kappaSlider{layer_id}"
        rough_slider_name = f"roughSlider{layer_id}"
        
        # Albedo parameter (color)
        cmds.colorSliderGrp(
            albedo_slider_name,
            label='Albedo: ',
            rgbValue=(layer_tree[layer_id]["params"]["albedo"][0], 
                layer_tree[layer_id]["params"]["albedo"][1], 
                layer_tree[layer_id]["params"]["albedo"][2]),
            changeCommand=lambda *args, lid=layer_id: update_color_param(lid, "albedo", albedo_slider_name)
        )
        
        # IOR parameter
        cmds.floatSliderGrp(
            ior_slider_name,
            label='IOR: ',
            value=layer_tree[layer_id]["params"]["IOR"],
            field=True,
            minValue=1.0,
            maxValue=5.0,
            fieldMinValue=1.0,
            fieldMaxValue=10.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "IOR", val)
        )
        
        # Kappa parameter
        cmds.floatSliderGrp(
            kappa_slider_name,
            label='Kappa: ',
            value=layer_tree[layer_id]["params"]["kappa"],
            field=True,
            minValue=0.0,
            maxValue=5.0,
            fieldMinValue=0.0,
            fieldMaxValue=10.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "kappa", val)
        )
        
        # Roughness parameter
        cmds.floatSliderGrp(
            rough_slider_name,
            label='Roughness: ',
            value=layer_tree[layer_id]["params"]["roughness"],
            field=True,
            minValue=0.0,
            maxValue=1.0,
            changeCommand=lambda val, lid=layer_id: update_param(lid, "roughness", val)
        )

        cmds.setParent('..')

def update_param(layer_id, param_name, value):
    layer_tree[layer_id]["params"][param_name] = value

def update_name_param(layer_id, value):
    layer_tree[layer_id]["params"]["name"] = value
    refresh_layer_tree_ui()  # Refresh to update displayed names

def update_color_param(layer_id, param_name, slider_name):
    # For color parameters, we get r, g, b as separate arguments
    color_values = cmds.colorSliderGrp(slider_name, query=True, rgb=True)
    r, g, b = color_values[0], color_values[1], color_values[2]
    layer_tree[layer_id]["params"][param_name] = [r, g, b]

def remove_layer(layer_id, *args):
    # Get the parent layer
    parent_id = layer_tree[layer_id]["parent"]
    
    # Remove this layer from its parent's children list
    if parent_id and parent_id in layer_tree:
        if layer_id in layer_tree[parent_id]["children"]:
            layer_tree[parent_id]["children"].remove(layer_id)
        
        # If this was a top or bottom layer of an add node, clear the reference
        if layer_tree[parent_id]["type"] == "add":
            if layer_tree[parent_id].get("top_layer") == layer_id:
                layer_tree[parent_id]["top_layer"] = None
            if layer_tree[parent_id].get("bottom_layer") == layer_id:
                layer_tree[parent_id]["bottom_layer"] = None
    
    # Recursive function to delete a layer and all its children
    def delete_layer_recursive(layer_to_delete):
        # First delete all children
        children_to_delete = layer_tree[layer_to_delete]["children"].copy()
        for child_id in children_to_delete:
            delete_layer_recursive(child_id)
        # Then delete the layer itself
        if layer_to_delete in layer_tree:
            del layer_tree[layer_to_delete]
    
    # Delete the layer and its children
    delete_layer_recursive(layer_id)
    
    # Refresh the UI
    refresh_layer_tree_ui()

def select_mesh(*args):
    # Get the current selection
    selection = cmds.ls(selection=True, type="transform")
    
    # Check if something is selected
    if selection:
        # Check if the selected object has a mesh shape
        shapes = cmds.listRelatives(selection[0], shapes=True)
        if shapes and cmds.objectType(shapes[0]) == "mesh":
            # Update the text field with the selected mesh name
            cmds.textField("selectedMeshField", edit=True, text=selection[0])
        else:
            cmds.warning("Selected object is not a mesh.")
    else:
        cmds.warning("Nothing selected. Please select a mesh.")

def apply_function(*args):
    # Get the selected mesh name from the text field
    selected_mesh = cmds.textField("selectedMeshField", query=True, text=True)
    
    # Check if a mesh is selected
    if selected_mesh:
        if not layer_tree["root"]["children"]:
            cmds.warning("No materials defined. Please add at least one layer.")
            return
            
        validation_errors = validate_layer_tree()
        if validation_errors:
            cmds.confirmDialog(
                title="Structure Validation Failed", 
                message=f"The material structure has the following issues:\n\n{validation_errors}",
                button=["OK"]
            )
            return
        
        print("Applying material stack to mesh: {}".format(selected_mesh))
        print("Layer structure:")
        #print(json.dumps(layer_tree, indent=2))

        json_tree = json.dumps(layer_tree)
        print(json_tree)
        #cmds.displayInfo(f"[PYTHON] {json_tree}")

        first_material = layer_tree["root"]["children"][0]
        first_material_name = layer_tree[first_material]["params"]["name"]
        cmds.applyMultiLayerMaterial(selected_mesh, json_tree, first_material_name)

        try:
            #cmds.confirmDialog(title="Success", message=f"Applied material {first_material_name} to {selected_mesh}", button=["OK"])
            cmds.confirmDialog(title="Success", message="Applied material to mesh", button=["OK"])
        except Exception as e:
            cmds.error("Error applying material: {}".format(str(e)))
    else:
        cmds.warning("No mesh selected. Please select a mesh first.")

def validate_layer_tree():
    """Validate the layer tree structure for completeness"""
    errors = []
    
    def validate_add_node(node_id):
        node = layer_tree[node_id]
        if node["top_layer"] is None:
            errors.append(f"Add node is missing a top layer")
        if node["bottom_layer"] is None:
            errors.append(f"Add node is missing a bottom layer")
    
    # Check all add nodes recursively
    def check_nodes(node_id):
        node = layer_tree[node_id]
        if node["type"] == "add":
            validate_add_node(node_id)
        for child_id in node["children"]:
            check_nodes(child_id)
    
    for child_id in layer_tree["root"]["children"]:
        check_nodes(child_id)
    
    return "\n".join(errors)

def save_layer_structure(*args):
    file_path = cmds.fileDialog2(fileFilter="JSON Files (*.json)", dialogStyle=2, fileMode=0, caption="Save Layer Structure")
    
    if file_path:
        try:
            with open(file_path[0], 'w') as file:
                json.dump(layer_tree, file, indent=2)
            cmds.confirmDialog(title="Success", message="Layer structure saved successfully", button=["OK"])
        except Exception as e:
            cmds.error("Error saving layer structure: {}".format(str(e)))

def load_from_file(*args):
    global preset_dir
    file_path = cmds.fileDialog2(fileFilter="JSON Files (*.json)", dialogStyle=2, fileMode=1, caption="Load Layer Structure", startingDirectory=preset_dir)
    
    if not file_path:
        return

    load_layer_structure(file_path[0])

def load_layer_structure(file_path):

    if file_path:
        try:
            with open(file_path, 'r') as file:
                global layer_tree, layer_counter
                loaded_tree = json.load(file)
                
                if "root" not in loaded_tree or "type" not in loaded_tree["root"] or loaded_tree["root"]["type"] != "root":
                    cmds.error("Invalid layer structure file format")
                    return
                
                layer_tree = loaded_tree
                
                # Update the layer counter to be higher than any existing layer id
                for layer_id in layer_tree:
                    if layer_id != "root" and layer_id.startswith("layer_"):
                        try:
                            counter = int(layer_id.split("_")[1])
                            layer_counter = max(layer_counter, counter)
                        except:
                            pass
                
            refresh_layer_tree_ui()
            cmds.confirmDialog(title="Success", message="Layer structure loaded successfully", button=["OK"])
        except Exception as e:
            cmds.error("Error loading layer structure: {}".format(str(e)))

def cleanup_ui():
    if cmds.window("meshSelectionUI", exists=True):
        cmds.deleteUI("meshSelectionUI")
