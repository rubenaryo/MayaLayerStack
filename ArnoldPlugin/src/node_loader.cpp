#include <ai.h>

extern const AtNodeMethods* MLSNodeMtd;
extern const AtNodeMethods* MLSAddNodeMtd;
extern const AtNodeMethods* MLSMetalNodeMtd;
extern const AtNodeMethods* MLSDielectricNodeMtd;
extern const AtNodeMethods* MLSVolumetricNodeMtd;

node_loader
{
    
    switch (i)
    {
    case 0:
        node->methods = MLSNodeMtd;
        node->output_type = AI_TYPE_CLOSURE;
        node->name = "layerstack";
        node->node_type = AI_NODE_SHADER;
        break;
    
    case 1:
        node->methods = MLSAddNodeMtd;
        node->output_type = AI_TYPE_STRING;
        node->name = "layerstack_add";
        node->node_type = AI_NODE_SHADER;
        break;

    case 2:
        node->methods = MLSMetalNodeMtd;
        node->output_type = AI_TYPE_STRING;
        node->name = "layerstack_metal";
        node->node_type = AI_NODE_SHADER;
        break;

    case 3:
        node->methods = MLSDielectricNodeMtd;
        node->output_type = AI_TYPE_STRING;
        node->name = "layerstack_dielectric";
        node->node_type = AI_NODE_SHADER;
        break;

    case 4:
        node->methods = MLSVolumetricNodeMtd;
        node->output_type = AI_TYPE_STRING;
        node->name = "layerstack_volumetric";
        node->node_type = AI_NODE_SHADER;
        break;
    
    default:
        return false;
    }
    
    strcpy(node->version, AI_VERSION);
    return true;
}

