import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEmlsLayerAddTemplate(ShaderAETemplate):

    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        # self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.beginLayout('Layers', collapse=False)
        self.addControl('top', label='Top Layer')
        self.addControl('bottom', label='Bottom Layer')
        self.endLayout()

        maya.mel.eval('AEdependNodeTemplate '+ self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()