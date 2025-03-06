# LSystemInstancer.py
#   Produces random locations to be used with the Maya instancer node.
#   Made by Ruben Young (rubenayr) for CIS 6600

import sys
import random

import maya.OpenMaya as OpenMaya
import maya.OpenMayaAnim as OpenMayaAnim
import maya.OpenMayaMPx as OpenMayaMPx
import maya.OpenMayaUI as omui
import maya.cmds as cmds
import maya.mel as mel
import LSystem as ls
from functools import partial
from PySide2 import QtWidgets, QtCore
from shiboken2 import wrapInstance

# Useful functions for declaring attributes as inputs or outputs.
def MAKE_INPUT(attr):
    attr.setKeyable(True)
    attr.setStorable(True)
    attr.setReadable(True)
    attr.setWritable(True)
def MAKE_OUTPUT(attr):
    attr.setKeyable(False)
    attr.setStorable(False)
    attr.setReadable(True)
    attr.setWritable(False)

###################################################################################
# Part 3.1 Random Node START

# Define the name of the node
kPluginRandomNodeTypeName = "randomNode"

# Give the node a unique ID. Make sure this ID is different from all of your
# other nodes!
randomNodeId = OpenMaya.MTypeId(0x8704)

# Node definition
class randomNode(OpenMayaMPx.MPxNode):
    # Declare class variables:
    # TODO:: declare the input and output class variables
    #         i.e. inNumPoints = OpenMaya.MObject()
    inNumPoints = OpenMaya.MObject()
    inMinBound = OpenMaya.MObject()
    inMaxBound = OpenMaya.MObject()
    
    outPoints = OpenMaya.MObject()
    
    # constructor
    def __init__(self):
        OpenMayaMPx.MPxNode.__init__(self)

    # compute
    def compute(self,plug,dataBlock):
        # TODO:: create the main functionality of the node. Your node should 
        #         take in three floats for max position (X,Y,Z), three floats 
        #         for min position (X,Y,Z), and the number of random points to
        #         be generated. Your node should output an MFnArrayAttrsData 
        #         object containing the random points. Consult the homework
        #         sheet for how to deal with creating the MFnArrayAttrsData. 
        
        if plug == randomNode.outPoints:
            pointsData = dataBlock.outputValue(randomNode.outPoints)
            arrayDataFn = OpenMaya.MFnArrayAttrsData()
            arrayData = arrayDataFn.create()
            
            posArray = arrayDataFn.vectorArray("position")
            idArray = arrayDataFn.doubleArray("id")
            
            numPoints = dataBlock.inputValue(randomNode.inNumPoints).asInt()
            minBounds = dataBlock.inputValue(randomNode.inMinBound).asFloat3()
            maxBounds = dataBlock.inputValue(randomNode.inMaxBound).asFloat3()
           
            idCounter = 0.0
            for _ in range(numPoints):
                x = random.uniform(minBounds[0], maxBounds[0])
                y = random.uniform(minBounds[1], maxBounds[1])
                z = random.uniform(minBounds[2], maxBounds[2])
                
                mayaVector = OpenMaya.MVector(x,y,z)
                posArray.append(mayaVector)
                idArray.append(idCounter)
                idCounter = idCounter + 1.0
                
            pointsData.setMObject(arrayData)
            dataBlock.setClean(plug)
            
def randomNodeInitializer():
    tAttr = OpenMaya.MFnTypedAttribute()
    nAttr = OpenMaya.MFnNumericAttribute()

    randomNode.inNumPoints = nAttr.create("numPoints", "np", OpenMaya.MFnNumericData.kInt, 0)
    MAKE_INPUT(nAttr)
    
    randomNode.inMinBound = nAttr.create("minBound", "min", OpenMaya.MFnNumericData.k3Float, 0)
    MAKE_INPUT(nAttr)
    
    randomNode.inMaxBound = nAttr.create("maxBound", "max", OpenMaya.MFnNumericData.k3Float, 0)
    MAKE_INPUT(nAttr)
    
    randomNode.outPoints = tAttr.create("outPoints", "op", OpenMaya.MFnArrayAttrsData.kDynArrayAttrs)
    MAKE_OUTPUT(tAttr)

    try:
        randomNode.addAttribute(randomNode.inNumPoints)
        randomNode.addAttribute(randomNode.inMinBound)
        randomNode.addAttribute(randomNode.inMaxBound)
        randomNode.addAttribute(randomNode.outPoints)
        
        randomNode.attributeAffects(randomNode.inNumPoints, randomNode.outPoints)
        randomNode.attributeAffects(randomNode.inMinBound, randomNode.outPoints)
        randomNode.attributeAffects(randomNode.inMaxBound, randomNode.outPoints)

    except:
        sys.stderr.write( ("Failed to create attributes of %s node\n", kPluginRandomNodeTypeName) )

# creator
def randomNodeCreator():
    return OpenMayaMPx.asMPxPtr( randomNode() )
    
# Part 3.1: Random Node END
###################################################################################

###################################################################################  
# Part 3.2: L System Instancer Node START

kPluginLSystemInstanceNodeTypeName = "LSystemInstanceNode"
lSystemInstanceNodeId = OpenMaya.MTypeId(0x8705)

# Node definition
class LSystemInstanceNode(OpenMayaMPx.MPxNode):
    inAngle = OpenMaya.MObject()
    inStepSize = OpenMaya.MObject()
    inGrammarFile = OpenMaya.MObject()
    inIterations = OpenMaya.MObject()
    
    outBranches = OpenMaya.MObject()
    outFlowers = OpenMaya.MObject()
    
    # constructor
    def __init__(self):
        OpenMayaMPx.MPxNode.__init__(self)

    # compute
    def compute(self,plug,data):
       
        if plug != LSystemInstanceNode.outBranches and plug != LSystemInstanceNode.outFlowers:
            return
            
        angle = data.inputValue(LSystemInstanceNode.inAngle).asFloat()
        stepSize = data.inputValue(LSystemInstanceNode.inStepSize).asFloat()
        iterations = data.inputValue(LSystemInstanceNode.inIterations).asInt()
        grammarFile = data.inputValue(LSystemInstanceNode.inGrammarFile).asString()
       
        
        lSystem = ls.LSystem()
        lSystem.loadProgram(str(grammarFile))
        lSystem.setDefaultAngle(angle)
        lSystem.setDefaultStep(stepSize)
        
        branchVector = ls.VectorPyBranch()
        flowerVector = ls.VectorPyBranch()
        
        lSystem.processPy(iterations, branchVector, flowerVector)
        
        idCounter = 0.0
        if plug == LSystemInstanceNode.outBranches:
            branchData = data.outputValue(LSystemInstanceNode.outBranches)
            branchAAD = OpenMaya.MFnArrayAttrsData()
            branchObject = branchAAD.create()
            
            branchPosArray = branchAAD.vectorArray("position")
            branchIdArray = branchAAD.doubleArray("id")
            branchDirArray = branchAAD.vectorArray("aimDirection")
            branchScaleArray = branchAAD.vectorArray("scale")
            
            for b in branchVector:
                startX = b[0]
                startY = b[1]
                startZ = b[2]
                
                endX = b[3]
                endY = b[4]
                endZ = b[5]
                
                dirX = endX-startX
                dirY = endY-startY
                dirZ = endZ-startZ
                
                avgX = (startX+endX) * 0.5
                avgY = (startY+endY) * 0.5
                avgZ = (startZ+endZ) * 0.5
                
                branchPosArray.append(OpenMaya.MVector(avgX, avgY, avgZ))
                branchDirArray.append(OpenMaya.MVector(dirX, dirY, dirZ))
                branchScaleArray.append(OpenMaya.MVector(1.0, 0.3, 0.3))
                branchIdArray.append(idCounter)
                idCounter = idCounter + 1.0
                
            branchData.setMObject(branchObject)
            
        if plug == LSystemInstanceNode.outFlowers:
            flowerData = data.outputValue(LSystemInstanceNode.outFlowers)
            flowerAAD = OpenMaya.MFnArrayAttrsData()
            flowerObject = flowerAAD.create()
            
            flowerPosArray = flowerAAD.vectorArray("position")
            flowerScaleArray = flowerAAD.vectorArray("scale")
            flowerIdArray = flowerAAD.doubleArray("id")
                
            for flower in flowerVector:
                x = flower[0]
                y = flower[1]
                z = flower[2]
                flowerPosArray.append(OpenMaya.MVector(x, y, z))
                flowerScaleArray.append(OpenMaya.MVector(0.4, 0.4, 0.4))
                flowerIdArray.append(idCounter)
                idCounter = idCounter + 1.0
                
            flowerData.setMObject(flowerObject)
        
        data.setClean(plug)
    
# initializer
def lSystemNodeInitializer():
    tAttr = OpenMaya.MFnTypedAttribute()
    nAttr = OpenMaya.MFnNumericAttribute()

    LSystemInstanceNode.inAngle = nAttr.create("angle", "a", OpenMaya.MFnNumericData.kFloat, 45.0)
    MAKE_INPUT(nAttr)
    
    LSystemInstanceNode.inStepSize = nAttr.create("stepSize", "ss", OpenMaya.MFnNumericData.kFloat, 1.0)
    MAKE_INPUT(nAttr)
    
    LSystemInstanceNode.inGrammarFile = tAttr.create("grammar", "g", OpenMaya.MFnData.kString, OpenMaya.MFnStringData().create("C:/Users/Ruben/Code/CIS6600/HW3/plants/simple7.txt"))
    MAKE_INPUT(nAttr)
    
    LSystemInstanceNode.inIterations = nAttr.create("iterations", "i", OpenMaya.MFnNumericData.kInt, 0)
    MAKE_INPUT(nAttr)
    
    LSystemInstanceNode.outBranches = tAttr.create("outBranches", "ob", OpenMaya.MFnArrayAttrsData.kDynArrayAttrs)
    MAKE_OUTPUT(tAttr)
    
    LSystemInstanceNode.outFlowers = tAttr.create("outFlowers", "of", OpenMaya.MFnArrayAttrsData.kDynArrayAttrs)
    MAKE_OUTPUT(tAttr)

    try:

        LSystemInstanceNode.addAttribute(LSystemInstanceNode.inAngle)
        LSystemInstanceNode.addAttribute(LSystemInstanceNode.inStepSize)
        LSystemInstanceNode.addAttribute(LSystemInstanceNode.inGrammarFile)
        LSystemInstanceNode.addAttribute(LSystemInstanceNode.inIterations)
        LSystemInstanceNode.addAttribute(LSystemInstanceNode.outBranches)
        LSystemInstanceNode.addAttribute(LSystemInstanceNode.outFlowers)
        
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inAngle, LSystemInstanceNode.outBranches)
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inAngle, LSystemInstanceNode.outFlowers)
        
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inStepSize, LSystemInstanceNode.outBranches)
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inStepSize, LSystemInstanceNode.outFlowers)
        
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inGrammarFile, LSystemInstanceNode.outBranches)
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inGrammarFile, LSystemInstanceNode.outFlowers)
        
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inIterations, LSystemInstanceNode.outBranches)
        LSystemInstanceNode.attributeAffects(LSystemInstanceNode.inIterations, LSystemInstanceNode.outFlowers)

    except:
        sys.stderr.write( ("Failed to create attributes of %s node\n", kPluginLSystemInstanceNodeTypeName) )

# creator
def lSystemNodeCreator():
    return OpenMayaMPx.asMPxPtr( LSystemInstanceNode() )
    
# Part 3.2: L System Instancer Node END
###################################################################################

###################################################################################  
# Part 4.1: The random node GUI.
class RandomInstanceGUI:
    def __init__(self):
        self.window_name = "randomInstanceWindow"
        self.selected_object = None
        
    def create_window(self):
        # Delete window if it exists
        if cmds.window(self.window_name, exists=True):
            cmds.deleteUI(self.window_name)
            
        # Create window
        window = cmds.window(self.window_name, title="Random Instance Creator", width=300)
        
        # Main layout
        main_layout = cmds.columnLayout(adjustableColumn=True, rowSpacing=5, columnOffset=["both", 5])
        
        # Object selection section
        cmds.frameLayout(label="Object Selection", collapsable=False)
        cmds.button(label="Record Selected Object", command=self.record_object)
        self.object_text = cmds.text(label="No object selected")
        cmds.setParent('..')
        
        # Instance count section
        cmds.frameLayout(label="Instance Count", collapsable=False)
        self.instance_slider = cmds.intSliderGrp(
            field=True,
            minValue=0,
            maxValue=20,
            value=10,
            fieldMinValue=0,
            fieldMaxValue=20
        )
        cmds.setParent('..')
        
        # Bounds section
        cmds.frameLayout(label="Instance Bounds", collapsable=False)
        
        # Minimum bounds
        cmds.text(label="Minimum Bounds:")
        min_layout = cmds.rowLayout(numberOfColumns=6)
        cmds.text(label="X:")
        self.min_x = cmds.floatField(value=-5.0, precision=2)
        cmds.text(label="Y:")
        self.min_y = cmds.floatField(value=-5.0, precision=2)
        cmds.text(label="Z:")
        self.min_z = cmds.floatField(value=-5.0, precision=2)
        cmds.setParent('..')
        
        # Maximum bounds
        cmds.text(label="Maximum Bounds:")
        max_layout = cmds.rowLayout(numberOfColumns=6)
        cmds.text(label="X:")
        self.max_x = cmds.floatField(value=5.0, precision=2)
        cmds.text(label="Y:")
        self.max_y = cmds.floatField(value=5.0, precision=2)
        cmds.text(label="Z:")
        self.max_z = cmds.floatField(value=5.0, precision=2)
        cmds.setParent('..')
        cmds.setParent('..')
        
        # Create node button
        cmds.button(label="Create Randomizer Node", command=self.create_random_node)
        
        cmds.showWindow(window)
    
    def record_object(self, *args):
        selection = cmds.ls(selection=True)
        if selection:
            self.selected_object = selection[0]
            cmds.text(self.object_text, edit=True, label=f"Selected: {self.selected_object}")
        else:
            cmds.warning("Nothing selected")
    
    def create_random_node(self, *args):
        if not self.selected_object:
            cmds.warning("Please select an object first")
            return
            
        # Get values from UI
        num_instances = cmds.intSliderGrp(self.instance_slider, query=True, value=True)
        min_bounds = [
            cmds.floatField(self.min_x, query=True, value=True),
            cmds.floatField(self.min_y, query=True, value=True),
            cmds.floatField(self.min_z, query=True, value=True)
        ]
        max_bounds = [
            cmds.floatField(self.max_x, query=True, value=True),
            cmds.floatField(self.max_y, query=True, value=True),
            cmds.floatField(self.max_z, query=True, value=True)
        ]
        
        # Create randomNode
        random_node = cmds.createNode("randomNode")
        
        # Set attributes
        cmds.setAttr(f"{random_node}.numPoints", num_instances)
        cmds.setAttr(f"{random_node}.minBound", *min_bounds, type="float3")
        cmds.setAttr(f"{random_node}.maxBound", *max_bounds, type="float3")
        
        # Create instancer
        instancer_obj = cmds.instancer()
        
        # Connect the selected object's matrix to the instancer
        cmds.connectAttr(f"{self.selected_object}.matrix", f"{instancer_obj}.inputHierarchy[0]")
        
        # Connect the randomNode's output points to the instancer's input.
        cmds.connectAttr(f"{random_node}.outPoints", f"{instancer_obj}.inputPoints")
        
# 4.1 The random node GUI END
###################################################################################

###################################################################################
# Part 4.2: PyQt GUI
def maya_main_window():
    """Return Maya's main window as a Qt widget"""
    main_window = omui.MQtUtil.mainWindow()
    return wrapInstance(int(main_window), QtWidgets.QWidget)

class SimpleWindow(QtWidgets.QDialog):
    def __init__(self, parent=None):
        super(SimpleWindow, self).__init__(parent)
        
        self.selected_object = None
        
        self.setWindowTitle("Random Node Creator")
        self.setMinimumWidth(300)
        self.setWindowFlags(self.windowFlags() ^ QtCore.Qt.WindowContextHelpButtonHint)
        
        self.create_widgets()
        self.create_layouts()
        self.create_connections()
    
    def create_widgets(self):
        # Object Selection Section
        self.object_label = QtWidgets.QLabel("Selected Object: None")
        self.record_btn = QtWidgets.QPushButton("Record Selected Object")
        
        # Instance Count Section
        self.instance_label = QtWidgets.QLabel("Number of Instances:")
        self.instance_slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.instance_slider.setRange(0, 20)
        self.instance_slider.setValue(10)
        self.instance_spinbox = QtWidgets.QSpinBox()
        self.instance_spinbox.setRange(0, 20)
        self.instance_spinbox.setValue(10)
        
        # Bounds Section
        # Minimum Bounds
        self.min_group = QtWidgets.QGroupBox("Minimum Bounds")
        self.min_x = QtWidgets.QDoubleSpinBox()
        self.min_y = QtWidgets.QDoubleSpinBox()
        self.min_z = QtWidgets.QDoubleSpinBox()
        
        for spinbox in [self.min_x, self.min_y, self.min_z]:
            spinbox.setRange(-999.99, 999.99)
            spinbox.setValue(-5.0)
            spinbox.setDecimals(2)
            
        # Maximum Bounds
        self.max_group = QtWidgets.QGroupBox("Maximum Bounds")
        self.max_x = QtWidgets.QDoubleSpinBox()
        self.max_y = QtWidgets.QDoubleSpinBox()
        self.max_z = QtWidgets.QDoubleSpinBox()
        
        for spinbox in [self.max_x, self.max_y, self.max_z]:
            spinbox.setRange(-999.99, 999.99)
            spinbox.setValue(5.0)
            spinbox.setDecimals(2)
            
        # Create Node Button
        self.create_btn = QtWidgets.QPushButton("Create Randomizer Node")
        
    def create_layouts(self):
        # Main Layout
        main_layout = QtWidgets.QVBoxLayout(self)
        
        # Object Selection Layout
        object_layout = QtWidgets.QVBoxLayout()
        object_layout.addWidget(self.object_label)
        object_layout.addWidget(self.record_btn)
        
        # Instance Count Layout
        instance_layout = QtWidgets.QHBoxLayout()
        instance_layout.addWidget(self.instance_label)
        instance_layout.addWidget(self.instance_slider)
        instance_layout.addWidget(self.instance_spinbox)
        
        # Minimum Bounds Layout
        min_layout = QtWidgets.QHBoxLayout()
        min_layout.addWidget(QtWidgets.QLabel("X:"))
        min_layout.addWidget(self.min_x)
        min_layout.addWidget(QtWidgets.QLabel("Y:"))
        min_layout.addWidget(self.min_y)
        min_layout.addWidget(QtWidgets.QLabel("Z:"))
        min_layout.addWidget(self.min_z)
        self.min_group.setLayout(min_layout)
        
        # Maximum Bounds Layout
        max_layout = QtWidgets.QHBoxLayout()
        max_layout.addWidget(QtWidgets.QLabel("X:"))
        max_layout.addWidget(self.max_x)
        max_layout.addWidget(QtWidgets.QLabel("Y:"))
        max_layout.addWidget(self.max_y)
        max_layout.addWidget(QtWidgets.QLabel("Z:"))
        max_layout.addWidget(self.max_z)
        self.max_group.setLayout(max_layout)
        
        # Add all layouts to main layout
        main_layout.addLayout(object_layout)
        main_layout.addSpacing(10)
        main_layout.addLayout(instance_layout)
        main_layout.addSpacing(10)
        main_layout.addWidget(self.min_group)
        main_layout.addWidget(self.max_group)
        main_layout.addSpacing(10)
        main_layout.addWidget(self.create_btn)
        
    def create_connections(self):
        self.record_btn.clicked.connect(self.record_object)
        self.create_btn.clicked.connect(self.create_random_node)
        self.instance_slider.valueChanged.connect(self.instance_spinbox.setValue)
        self.instance_spinbox.valueChanged.connect(self.instance_slider.setValue)
        
    def record_object(self):
        selection = cmds.ls(selection=True)
        if selection:
            self.selected_object = selection[0]
            self.object_label.setText(f"Selected Object: {self.selected_object}")
        else:
            cmds.warning("Nothing selected")
            
    def create_random_node(self):
        if not self.selected_object:
            cmds.warning("Please select an object first")
            return
       
        # Get values from UI
        num_instances = self.instance_spinbox.value()
        min_bounds = [
            self.min_x.value(),
            self.min_y.value(),
            self.min_z.value()
        ]
        max_bounds = [
            self.max_x.value(),
            self.max_y.value(),
            self.max_z.value()
        ]
        
        # Create randomNode
        random_node = cmds.createNode("randomNode")
        
        # Set attributes
        cmds.setAttr(f"{random_node}.numPoints", num_instances)
        cmds.setAttr(f"{random_node}.minBound", *min_bounds, type="float3")
        cmds.setAttr(f"{random_node}.maxBound", *max_bounds, type="float3")
        
        # Create instancer
        instancer_obj = cmds.instancer()
        
        # Connect the selected object's matrix to the instancer
        cmds.connectAttr(f"{self.selected_object}.matrix", f"{instancer_obj}.inputHierarchy[0]")
        
        # Connect the randomNode's output points to the instancer's input.
        cmds.connectAttr(f"{random_node}.outPoints", f"{instancer_obj}.inputPoints")
        
# 4.2 END
###################################################################################

###################################################################################
# 4.4: Node Setup Menu START

# Plugin initialization
gui_instance = None
dialog_instance = None
useQt = False # Global variable to control whether to use 4.1 GUI or 4.2 GUI

class SelectionMenu:
    def __init__(self):
        self.window_name = "LSystemInstance"
        self.create_window()
        
    def create_window(self):
        # Delete window if it exists
        if cmds.window(self.window_name, exists=True):
            cmds.deleteUI(self.window_name)
            
        # Create window
        window = cmds.window(self.window_name, title="Selection Menu", width=200)
        layout = cmds.columnLayout(adjustableColumn=True)
        
        # Create buttons
        cmds.button(label="Create RandomNode Network", command=self.create_randomNode_network)
        cmds.button(label="Create default LSystem Instance Network", command=self.create_default_LSystemInstance)
        cmds.button(label="Create user LSystem Instance Network (select 2 objects)", command=self.create_user_LSystemInstance)
        
        # Show window
        cmds.showWindow(window)
    
    def create_randomNode_network(self, *args):
        print("Button A was clicked!")
        # Create GUI upon loading plugin.
        global gui_instance
        global dialog_instance
        global useQt
        if useQt:
            dialog_instance = SimpleWindow()
            dialog_instance.show()
        else:
            gui_instance = RandomInstanceGUI()
            gui_instance.create_window()
        
    def create_LSystemInstance(self, branchObject, flowerObject):
        print (f"create_LSystemInstance {branchObject} {flowerObject}")
        branch_instancer = mel.eval("instancer")
        flower_instancer = mel.eval("instancer")
        
        lSystem_node_name = mel.eval("createNode LSystemInstanceNode")
        print(lSystem_node_name)
        
        command_str =  f"""
            connectAttr {branchObject}.matrix {branch_instancer}.inputHierarchy[0];
            connectAttr {lSystem_node_name}.outBranches {branch_instancer}.inputPoints;
            connectAttr {flowerObject}.matrix {flower_instancer}.inputHierarchy[0];
            connectAttr {lSystem_node_name}.outFlowers {flower_instancer}.inputPoints;
        """
                        
        mel.eval(command_str)
        
    def create_default_LSystemInstance(self, *args):  
        poly_cube_name = mel.eval("polyCube")[0]
        poly_sphere_name = mel.eval("polySphere")[0]
        print(f"{poly_cube_name}")
        self.create_LSystemInstance(poly_cube_name, poly_sphere_name)
        
    def create_user_LSystemInstance(self, *args):
        selection = cmds.ls(selection=True)
        if len(selection) == 2:
            print(f"Two objects selected: {selection[0]} and {selection[1]}")
            self.create_LSystemInstance(selection[0], selection[1])
        else:
            cmds.warning("Please select exactly two objects!")

selection_menu = None

# initialize the script plug-in
def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.registerNode( kPluginRandomNodeTypeName, randomNodeId, randomNodeCreator, randomNodeInitializer )
        mplugin.registerNode( kPluginLSystemInstanceNodeTypeName, lSystemInstanceNodeId, lSystemNodeCreator, lSystemNodeInitializer )
       
    except:
        sys.stderr.write( "Failed to register node: %s\n" % kPluginRandomNodeTypeName )
        sys.stderr.write( "Failed to register node: %s\n" % kPluginLSystemInstanceNodeTypeName )
        
    global selection_menu
    selection_menu = SelectionMenu()
    selection_menu.create_window()

# uninitialize the script plug-in
def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    
    # Clean up GUI
    if useQt:
        global dialog_instance
        if dialog_instance is not None:
            dialog_instance.deleteLater()
    else:
        windowExists = cmds.window("randomInstanceWindow", exists=True)
        if windowExists:
            cmds.deleteUI("randomInstanceWindow")
            
    windowExists = cmds.window("selectionMenu", exists=True)
    if windowExists:
        cmds.deleteUI("selectionMenu")
        
    try:
        mplugin.deregisterNode( randomNodeId )
        mplugin.deregisterNode( lSystemInstanceNodeId )
    except:
        sys.stderr.write( "Failed to unregister node: %s\n" % kPluginRandomNodeTypeName )
        sys.stderr.write( "Failed to unregister node: %s\n" % kPluginLSystemInstanceNodeTypeName )
