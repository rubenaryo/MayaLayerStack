#ifndef CreateLSystemCmd_H_
#define CreateLSystemCmd_H_

#include <maya/MPxCommand.h>
#include <string>

class LayerStackCmd : public MPxCommand
{
public:
    LayerStackCmd();
    virtual ~LayerStackCmd();
    static void* creator() { return new LayerStackCmd(); }
    static const char* name() { return "LayerStackCmd"; }
    MStatus doIt( const MArgList& args );
};

#endif