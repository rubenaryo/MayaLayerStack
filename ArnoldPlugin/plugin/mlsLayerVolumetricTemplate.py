import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEmlsLayerVolumetricTemplate(ShaderAETemplate):

    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        # self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.beginLayout('Layer Parameters', collapse=False)
        self.addControl('albedo', label='Albedo')
        self.addControl('depth', label='Depth')
        self.addControl('g', label='G')
        self.endLayout()

        maya.mel.eval('AEdependNodeTemplate '+ self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()