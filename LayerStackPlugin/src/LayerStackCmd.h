#ifndef CreateLSystemCmd_H_
#define CreateLSystemCmd_H_

#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <string>
#include <vector>
#include <unordered_map>

struct LayeredShadingGroup;

class LayerStackCmd : public MPxCommand
{
public:
    LayerStackCmd();
    virtual ~LayerStackCmd();
    static void* creator() { return new LayerStackCmd(); }
    static const char* name() { return "applyMultiLayerMaterial"; }
    static void CleanupShadingGroups();
    MStatus doIt( const MArgList& args );

    LayeredShadingGroup* FindShadingGroupForMaterialName(MString& materialName);
    LayeredShadingGroup* CreateNewShadingGroup(MString& materialName);
};

#endif