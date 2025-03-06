import maya.cmds as cmds

def create_ui():
    # Check if the window already exists, if so, delete it
    if cmds.window("meshSelectionUI", exists=True):
        cmds.deleteUI("meshSelectionUI")
    
    # Create the main window
    window = cmds.window("meshSelectionUI", title="Layer Material Controls", width=300, height=200)
    
    # Create the main layout
    main_layout = cmds.columnLayout(adjustableColumn=True, columnAttach=('both', 5), rowSpacing=10, columnWidth=300)
    
    # Create a text field for displaying selected mesh
    cmds.text(label="Selected Mesh:", align="left")
    mesh_field = cmds.textField("selectedMeshField", editable=False)
    
    # Create a button to select mesh
    cmds.button(label="Select Mesh", command=select_mesh)
    
    # Create an "Apply" button at the bottom
    cmds.button(label="Apply", command=apply_function)
    
    # Show the window
    cmds.showWindow(window)

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
        # Do something with the selected mesh
        print(f"Applying function to mesh: {selected_mesh}")
        # Add your custom operations here
        
        # Example: Display a confirmation message
        cmds.confirmDialog(title="Success", message=f"Applied to {selected_mesh}", button=["OK"])
    else:
        cmds.warning("No mesh selected. Please select a mesh first.")

def cleanup_ui():
    # Check if the window already exists, if so, delete it
    if cmds.window("meshSelectionUI", exists=True):
        cmds.deleteUI("meshSelectionUI")