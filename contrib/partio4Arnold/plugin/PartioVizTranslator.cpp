#include "PartioVizTranslator.h"

#include "attributes/AttrHelper.h"

#include <ai_msg.h>
#include <ai_nodes.h>
#include <ai_ray.h>

#include <maya/MBoundingBox.h>
#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MRenderUtil.h>
#include <maya/MString.h>
#include <maya/MFloatVector.h>
#include <maya/MFileObject.h>

namespace {
    const AtString ginstanceName("ginstance");
}

void* CPartioVizTranslator::creator()
{
    return new CPartioVizTranslator();
}

void CPartioVizTranslator::NodeInitializer(CAbTranslator context)
{
#if MTOA12
    CExtensionAttrHelper helper(context.maya, "procedural");
#else
    CExtensionAttrHelper helper(context.maya, "partioGenerator");
#endif
    CShapeTranslator::MakeCommonAttributes(helper);

    CAttrData data;

    MStringArray enumNames;
    enumNames.append("points");
    enumNames.append("spheres");
    enumNames.append("quads");
#ifdef ARNOLD5
    data.defaultValue.INT() = 0;
#else
    data.defaultValue.INT = 0;
#endif
    data.name = "aiRenderPointsAs";
    data.shortName = "ai_render_points_as";
    data.enums = enumNames;
    helper.MakeInputEnum(data);

#ifdef ARNOLD5
    data.defaultValue.BOOL() = false;
#else
    data.defaultValue.BOOL = false;
#endif
    data.name = "aiOverrideRadiusPP";
    data.shortName = "ai_override_radiusPP";
    helper.MakeInputBoolean(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 1000000.0f;
    data.min.FLT() = 0.0f;
    data.softMax.FLT() = 1000000.0f;
#else
    data.defaultValue.FLT = 1000000.0f;
    data.min.FLT = 0.0f;
    data.softMax.FLT = 1000000.0f;
#endif
    data.name = "aiMaxParticleRadius";
    data.shortName = "ai_max_particle_radius";
    data.hasMin = true;
    data.hasSoftMax = true;
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 0.0f;
    data.softMax.FLT() = 1.0f;
#else
    data.defaultValue.FLT = 0.0f;
    data.softMax.FLT = 1.0f;
#endif
    data.name = "aiMinParticleRadius";
    data.shortName = "ai_min_particle_radius";
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 0.2f;
#else
    data.defaultValue.FLT = 0.2f;
#endif
    data.name = "aiRadius";
    data.shortName = "ai_radius";
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 1.0f;
#else
    data.defaultValue.FLT = 1.0f;
#endif
    data.name = "aiRadiusMultiplier";
    data.shortName = "ai_radius_multiplier";
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 1.0f;
    data.softMin.FLT() = 0.0f;
#else
    data.defaultValue.FLT = 1.0f;
    data.softMin.FLT = 0.0f;
#endif
    data.name = "aiMotionBlurMultiplier";
    data.shortName = "ai_motion_blur_multiplier";
    data.hasMin = false;
    data.hasSoftMin = true;
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 0.f;
    data.min.FLT() = 0.f;
    data.softMax.FLT() = 2.f;
#else
    data.defaultValue.FLT = 0.f;
    data.min.FLT = 0.f;
    data.softMax.FLT = 2.f;
#endif
    data.name = "aiStepSize";
    data.shortName = "ai_step_size";
    data.hasMin = true;
    data.hasSoftMax = true;
    data.hasSoftMin = false;
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.FLT() = 8.0f;
    data.min.FLT() = 0.0f;
    data.softMax.FLT() = 10.0f;
    data.softMin.FLT() = 7.0f;
#else
    data.defaultValue.FLT = 8.0f;
    data.min.FLT = 0.0f;
    data.softMax.FLT = 10.0f;
    data.softMin.FLT = 7.0f;
#endif
    data.name = "aiFilterSmallParticles";
    data.shortName = "ai_filter_small_particles";
    data.hasMin = true;
    data.hasSoftMax = true;
    data.hasSoftMin = true;
    helper.MakeInputFloat(data);

#ifdef ARNOLD5
    data.defaultValue.STR() = AtString("");
#else
    data.defaultValue.STR = "";
#endif
    data.name = "aiExportAttributes";
    data.shortName = "ai_export_attributes";
    helper.MakeInputString(data);

#ifdef ARNOLD5
    data.defaultValue.STR() = AtString("");
#else
    data.defaultValue.STR = "";
#endif
    data.name = "aiOverrideProcedural";
    data.shortName = "ai_override_procedural";
    helper.MakeInputString(data);
}

AtNode* CPartioVizTranslator::CreateArnoldNodes()
{
#if MTOA12
    return IsMasterInstance() ? AddArnoldNode("procedural") : AddArnoldNode("ginstance");
#else
    return IsMasterInstance() ? AddArnoldNode("partioGenerator") : AddArnoldNode("ginstance");
#endif
}

void CPartioVizTranslator::Export(AtNode* anode)
{
    if (AiNodeIs(anode, ginstanceName)) {
        ExportInstance(anode, GetMasterInstance());
    } else {
        ExportProcedural(anode, false);
    }
}

#ifdef MTOA12
void CPartioVizTranslator::ExportMotion(AtNode* anode, unsigned int step)
{
    ExportMatrix(anode, step);
}

void CPartioVizTranslator::Update(AtNode* anode)
{
    if (AiNodeIs(anode, ginstanceName))
        ExportInstance(anode, GetMasterInstance());
    else
        ExportProcedural(anode, true);
}

void CPartioVizTranslator::UpdateMotion(AtNode* anode, unsigned int step)
{
    ExportMatrix(anode, step);
}
#else
void CPartioVizTranslator::ExportMotion(AtNode* anode)
{
    ExportMatrix(anode);
}
#endif

// Deprecated : Arnold support procedural instance, but it's not safe.
//
AtNode* CPartioVizTranslator::ExportInstance(AtNode* instance, const MDagPath& masterInstance)
{
    AtNode* masterNode = AiNodeLookUpByName(masterInstance.partialPathName().asChar());

    AiNodeSetStr(instance, "name", m_dagPath.partialPathName().asChar());

#ifdef MTOA12
    ExportMatrix(instance, 0);
#else
    ExportMatrix(instance);
#endif

    AiNodeSetPtr(instance, "node", masterNode);
    AiNodeSetBool(instance, "inherit_xform", false);

    int visibility = AiNodeGetInt(masterNode, "visibility");
    AiNodeSetInt(instance, "visibility", visibility);

    m_DagNode.setObject(masterInstance);

    ExportPartioVizShaders(instance);

    return instance;
}

void CPartioVizTranslator::ExportShaders()
{
    AiMsgWarning("[mtoa] Shaders untested with new multitranslator and standin code.");
    /// TODO: Test shaders with standins.
#if MTOA12
    ExportPartioVizShaders(GetArnoldRootNode());
#else
    ExportPartioVizShaders(GetArnoldNode());
#endif
}

void CPartioVizTranslator::ExportPartioVizShaders(AtNode* procedural)
{
    int instanceNum = m_dagPath.isInstanced() ? m_dagPath.instanceNumber() : 0;

    std::vector<AtNode*> meshShaders;

    MPlug shadingGroupPlug = GetNodeShadingGroup(m_dagPath.node(), instanceNum);
    if (!shadingGroupPlug.isNull())
    {
#if MTOA12
        AtNode* shader = ExportNode(shadingGroupPlug);
#else
        AtNode* shader = ExportConnectedNode(shadingGroupPlug);
#endif
        if (shader != 0)
        {
            AiNodeSetPtr(procedural, "shader", shader);
            meshShaders.push_back(shader);
        }
        else
        {
            AiMsgWarning("[mtoa] [translator %s] ShadingGroup %s has no surfaceShader input",
                         GetTranslatorName().asChar(), MFnDependencyNode(shadingGroupPlug.node()).name().asChar());
            AiNodeSetPtr(procedural, "shader", 0);
        }
    }
}

AtNode* CPartioVizTranslator::ExportProcedural(AtNode* procedural, bool update)
{
    m_DagNode.setObject(m_dagPath.node());

    AiNodeSetStr(procedural, "name", m_dagPath.partialPathName().asChar());

#if MTOA12
    ExportMatrix(procedural, 0);
#else
    ExportMatrix(procedural);
#endif
    ProcessRenderFlags(procedural);
    ExportPartioVizShaders(procedural);

    if (!update) {
        MString dso = "[PARTIO_ARNOLD_PROCEDURAL]";

        // we add this here so we can add in a custom   particle reading procedural instead of the default one
        MString overrideProc = m_DagNode.findPlug("aiOverrideProcedural").asString();

        if (overrideProc.length() > 0)
            dso = overrideProc;

        MString formattedName = m_DagNode.findPlug("renderCachePath").asString();
        int frameNum = m_DagNode.findPlug("time").asInt();
        int frameOffset = m_DagNode.findPlug("cacheOffset").asInt();
        int byFrame = m_DagNode.findPlug("byFrame").asInt();

        int finalFrame = (frameNum + frameOffset) * byFrame;

        const int dotPosition = formattedName.rindexW('.');
        if (dotPosition == -1)
            return 0;
        /// TODO:  this is only temporary
        /// need to make the node actually update itself properly
        MString formatExt = formattedName.substring(dotPosition + 1, (formattedName.length() - 1));
        int cachePadding = 4;

        MString formatString = "%0";
        // special case for PDCs and maya nCache files because of the funky naming convention  TODO: support substepped/retiming  caches
        if (formatExt == "pdc") {
            finalFrame *= 6000 / 24;
            cachePadding = 1;
        }

        char frameString[10] = "";

        formatString += cachePadding;
        formatString += "d";

        const char* fmt = formatString.asChar();

        sprintf(frameString, fmt, finalFrame);

        MString frameNumFormattedName;

        MStringArray foo;

        formattedName.split('<', foo);
        MString newFoo;
        if (foo.length() > 1) {
            foo[1] = foo[1].substring(6, foo[1].length() - 1);
            newFoo = foo[0] + MString(frameString) + foo[1];
        } else {
            newFoo = foo[0];
        }

        if (!fileCacheExists(newFoo.asChar())) {
            AiMsgWarning("[mtoa] PartioVisualizer %s being skipped, can't find cache file.", m_DagNode.name().asChar());
            return 0;
        }

        // TODO: change these attributes to FindMayaPlug to support MtoA override nodes

        MTime oneSec(1.0, MTime::kSeconds);
        float fps = (float)oneSec.asUnits(MTime::uiUnit());

        MPlug partioAttrs = m_DagNode.findPlug("partioCacheAttributes");

        MFloatVector defaultColor;

        MPlug defaultColorPlug = m_DagNode.findPlug("defaultPointColor");

        MPlug defaultColorComp = defaultColorPlug.child(0);
        defaultColorComp.getValue(defaultColor.x);
        defaultColorComp = defaultColorPlug.child(1);
        defaultColorComp.getValue(defaultColor.y);
        defaultColorComp = defaultColorPlug.child(2);
        defaultColorComp.getValue(defaultColor.z);

        float defaultOpac = m_DagNode.findPlug("defaultAlpha").asFloat();      

        /////////////////////////////////////////
        // support  exporting volume step size
        float stepSize = m_DagNode.findPlug("aiStepSize").asFloat();

        m_customAttrs = m_DagNode.findPlug("aiExportAttributes").asString();

#if MTOA12
        AiNodeSetStr(procedural, "dso", dso.asChar());
        AiNodeSetBool(procedural, "load_at_init", true);

        AiNodeDeclare(procedural, "arg_file", "constant STRING");
        AiNodeSetStr(procedural, "arg_file", newFoo.asChar());

        AiNodeDeclare(procedural, "arg_radius", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_radius", m_DagNode.findPlug("defaultRadius").asFloat());

        AiNodeDeclare(procedural, "arg_maxParticleRadius", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_maxParticleRadius", m_DagNode.findPlug("aiMaxParticleRadius").asFloat());

        AiNodeDeclare(procedural, "arg_minParticleRadius", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_minParticleRadius", m_DagNode.findPlug("aiMinParticleRadius").asFloat());

        AiNodeDeclare(procedural, "arg_filterSmallParticles", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_filterSmallParticles", m_DagNode.findPlug("aiFilterSmallParticles").asFloat());

        AiNodeDeclare(procedural, "overrideRadiusPP", "constant BOOL");
        AiNodeSetBool(procedural, "overrideRadiusPP", m_DagNode.findPlug("aiOverrideRadiusPP").asBool());

        AiNodeDeclare(procedural, "global_motionBlurSteps", "constant INT");
        AiNodeSetInt(procedural, "global_motionBlurSteps", (int)GetNumMotionSteps());

        AiNodeDeclare(procedural, "global_motionByFrame", "constant FLOAT");
        AiNodeSetFlt(procedural, "global_motionByFrame", (float)GetMotionByFrame());

        AiNodeDeclare(procedural, "global_fps", "constant FLOAT");
        AiNodeSetFlt(procedural, "global_fps", fps);

        AiNodeDeclare(procedural, "arg_motionBlurMult", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_motionBlurMult", m_DagNode.findPlug("aiMotionBlurMultiplier").asFloat());

        AiNodeDeclare(procedural, "arg_renderType", "constant INT");
        AiNodeSetInt(procedural, "arg_renderType", m_DagNode.findPlug("aiRenderPointsAs").asInt());

        AiNodeDeclare(procedural, "arg_radiusMult", "constant FLOAT");
        AiNodeSetFlt(procedural, "arg_radiusMult", m_DagNode.findPlug("aiRadiusMultiplier").asFloat());

        AiNodeDeclare(procedural, "arg_velFrom", "constant STRING");
        AiNodeDeclare(procedural, "arg_accFrom", "constant STRING");
        AiNodeDeclare(procedural, "arg_rgbFrom", "constant STRING");
        AiNodeDeclare(procedural, "arg_opacFrom", "constant STRING");
        AiNodeDeclare(procedural, "arg_radFrom", "constant STRING");
        AiNodeDeclare(procedural, "arg_incandFrom", "constant STRING");

        AiNodeSetStr(procedural, "arg_velFrom", m_DagNode.findPlug("velocityFrom").asString().asChar());
        AiNodeSetStr(procedural, "arg_accFrom", m_DagNode.findPlug("accelerationFrom").asString().asChar());
        AiNodeSetStr(procedural, "arg_rgbFrom", m_DagNode.findPlug("colorFrom").asString().asChar());
        AiNodeSetStr(procedural, "arg_opacFrom", m_DagNode.findPlug("opacityFrom").asString().asChar());
        AiNodeSetStr(procedural, "arg_radFrom", m_DagNode.findPlug("radiusFrom").asString().asChar());
        AiNodeSetStr(procedural, "arg_incandFrom", m_DagNode.findPlug("incandescenceFrom").asString().asChar());

        AiNodeDeclare(procedural, "arg_defaultColor", "constant RGB");
        AiNodeDeclare(procedural, "arg_defaultOpac", "constant FLOAT");

        AiNodeSetRGB(procedural, "arg_defaultColor", defaultColor.x, defaultColor.y, defaultColor.z);
        AiNodeSetFlt(procedural, "arg_defaultOpac", defaultOpac);

        if (stepSize > 0)
        {
            AiNodeDeclare(procedural, "arg_stepSize", "constant FLOAT");
            AiNodeSetFlt(procedural, "arg_stepSize", stepSize);
        }

        if (m_customAttrs.length() > 0)
        {
            AiNodeDeclare(procedural, "arg_extraPPAttrs", "constant STRING");
            AiNodeSetStr(procedural, "arg_extraPPAttrs", m_customAttrs.asChar());
        }
#else
        AiNodeSetStr(procedural, "file", newFoo.asChar());
        AiNodeSetFlt(procedural, "radius", m_DagNode.findPlug("defaultRadius").asFloat());
        AiNodeSetFlt(procedural, "maxParticleRadius", m_DagNode.findPlug("aiMaxParticleRadius").asFloat());
        AiNodeSetFlt(procedural, "minParticleRadius", m_DagNode.findPlug("aiMinParticleRadius").asFloat());
        AiNodeSetFlt(procedural, "filterSmallParticles", m_DagNode.findPlug("aiFilterSmallParticles").asFloat());
        AiNodeSetBool(procedural, "overrideRadiusPP", m_DagNode.findPlug("aiOverrideRadiusPP").asBool());
        AiNodeSetInt(procedural, "global_motionBlurSteps", (int)GetNumMotionSteps());
        AiNodeSetFlt(procedural, "global_motionByFrame", (float)GetMotionByFrame());
        AiNodeSetFlt(procedural, "global_fps", fps);
        AiNodeSetFlt(procedural, "motionBlurMult", m_DagNode.findPlug("aiMotionBlurMultiplier").asFloat());
        AiNodeSetInt(procedural, "renderType", m_DagNode.findPlug("aiRenderPointsAs").asInt());
        AiNodeSetFlt(procedural, "radiusMult", m_DagNode.findPlug("aiRadiusMultiplier").asFloat());
        AiNodeSetStr(procedural, "velFrom", m_DagNode.findPlug("velocityFrom").asString().asChar());
        AiNodeSetStr(procedural, "accFrom", m_DagNode.findPlug("accelerationFrom").asString().asChar());
        AiNodeSetStr(procedural, "rgbFrom", m_DagNode.findPlug("colorFrom").asString().asChar());
        AiNodeSetStr(procedural, "opacFrom", m_DagNode.findPlug("opacityFrom").asString().asChar());
        AiNodeSetStr(procedural, "radFrom", m_DagNode.findPlug("radiusFrom").asString().asChar());
        AiNodeSetStr(procedural, "incandFrom", m_DagNode.findPlug("incandescenceFrom").asString().asChar());
        AiNodeSetRGB(procedural, "defaultColor", defaultColor.x, defaultColor.y, defaultColor.z);
        AiNodeSetFlt(procedural, "defaultOpac", defaultOpac);

        if (stepSize > 0) {
            AiNodeSetFlt(procedural, "stepSize", stepSize);
        }

        if (m_customAttrs.length() > 0) {
            AiNodeSetStr(procedural, "extraPPAttrs", m_customAttrs.asChar());
        }
#endif
        ExportLightLinking(procedural);
    }

    return procedural;
}

bool CPartioVizTranslator::fileCacheExists(const char* fileName)
{
    struct stat fileInfo;
    int intStat = stat(fileName, &fileInfo);
    return intStat == 0;
}
