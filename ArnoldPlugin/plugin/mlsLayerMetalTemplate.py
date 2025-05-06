import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEmlsLayerMetalTemplate(ShaderAETemplate):

    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        # self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.beginLayout('Layer Parameters', collapse=False)
        self.addControl('albedo', label='Albedo')
        self.addControl('IOR', label='IOR')
        self.addControl('kappa', label='Kappa')
        self.addControl('roughness', label='Roughness')
        self.endLayout()

        maya.mel.eval('AEdependNodeTemplate '+ self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()