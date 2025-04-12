import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEmlsLayerDielectricTemplate(ShaderAETemplate):

    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        # self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.beginLayout('Layer Parameters', collapse=False)
        self.addControl('iOR', label='IOR')
        self.addControl('roughness', label='Roughness')
        self.endLayout()

        maya.mel.eval('AEdependNodeTemplate '+ self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()