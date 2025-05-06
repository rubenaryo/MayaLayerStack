#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(MLSAddNodeMtd);

enum LayerStackAddParams {
    p_top,
    p_bottom
};

node_parameters
{
    AiParameterStr("top", "");
    AiParameterStr("bottom", "");
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    AtString top = AiShaderEvalParamStr(p_top);
    AtString bottom = AiShaderEvalParamStr(p_bottom);

    std::string topstd(top.c_str());
    std::string bottomstd(bottom.c_str());

    std::string outstd = topstd + bottomstd;

    AtString out(outstd.c_str());

    sg->out.STR() = out;
}
