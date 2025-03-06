#include "LayerStackCmd.h"


#include <maya/MGlobal.h>
#include <list>
LayerStackCmd::LayerStackCmd() : MPxCommand()
{
}

LayerStackCmd::~LayerStackCmd() 
{
}

MStatus LayerStackCmd::doIt( const MArgList& args )
{
	// message in Maya output window
    cout << "Hello World" <<endl;
	std::cout.flush();

	// message in scriptor editor
	MGlobal::displayInfo("Hello World");

    return MStatus::kSuccess;
}

