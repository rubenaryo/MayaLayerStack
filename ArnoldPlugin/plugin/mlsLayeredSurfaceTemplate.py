import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEmlsLayeredSurfaceTemplate(ShaderAETemplate):

    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.beginLayout('Layer 0', collapse=False)
        self.addControl('albedo_0', label='Top Color')
        self.addControl('eta_0', label='Top IOR')
        self.addControl('kappa_0', label='Top Kappa')
        self.addControl('alpha_0', label='Top Roughness')
        self.endLayout()

        self.beginLayout('Layer 1', collapse=False)
        self.addControl('albedo_1', label='Bottom Color')
        self.addControl('eta_1', label='Bottom IOR')
        self.addControl('kappa_1', label='Bottom Kappa')
        self.addControl('alpha_1', label='Bottom Roughness')
        self.endLayout()

        maya.mel.eval('AEdependNodeTemplate '+self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()