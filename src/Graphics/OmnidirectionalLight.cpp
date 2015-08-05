#include <OmnidirectionalLight.hpp>

#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective
#include <glm/gtx/transform.hpp> // glm::translate

#include <MathTools.hpp>
#include <Blur.hpp>
#include <ResourcesManager.hpp>

///////////////////////////////////////////////////////////////////
// Static attributes

Program* 			OmnidirectionalLight::s_depthProgram = nullptr;
VertexShader*		OmnidirectionalLight::s_depthVS = nullptr;
GeometryShader*	OmnidirectionalLight::s_depthGS = nullptr;
FragmentShader*	OmnidirectionalLight::s_depthFS = nullptr;

///////////////////////////////////////////////////////////////////

OmnidirectionalLight::OmnidirectionalLight(unsigned int shadowMapResolution) :
	_shadowMapResolution(shadowMapResolution),
	_shadowMapFramebuffer(_shadowMapResolution)
{
	updateMatrices();
}

void OmnidirectionalLight::setRange(float r)
{
	_range = r; 
	updateMatrices();
}

void OmnidirectionalLight::init()
{
	initPrograms();

	_shadowMapFramebuffer.getColor().init();
	_shadowMapFramebuffer.getColor().bind();
	for(int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, _shadowMapResolution, _shadowMapResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapS, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapT, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::WrapR, GL_CLAMP_TO_EDGE);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MinFilter, GL_LINEAR_MIPMAP_LINEAR);
	_shadowMapFramebuffer.getColor().set(Texture::Parameter::MagFilter, GL_LINEAR);
	_shadowMapFramebuffer.getColor().unbind();
	
	_shadowMapFramebuffer.init();
	
	_gpuBuffer.init();
}

void OmnidirectionalLight::updateMatrices()
{
	_projection = glm::perspective(static_cast<float>(pi() * 0.5), 1.0f, 0.5f, _range);
	
	GPUData tmpStruct = getGPUData();
	_gpuBuffer.data(&tmpStruct, sizeof(GPUData), Buffer::Usage::DynamicDraw);
}

void OmnidirectionalLight::bind() const
{
	getShadowBuffer().bind();
	getShadowBuffer().clear(BufferBit::All);
	getShadowMapProgram().setUniform("Position", _position);
	getShadowMapProgram().setUniform("Projection", _projection);
	getShadowMapProgram().use();
	Context::enable(Capability::CullFace);
}

void OmnidirectionalLight::unbind() const
{
	Context::disable(Capability::CullFace);
	Program::useNone();
	getShadowBuffer().unbind();
}

void OmnidirectionalLight::drawShadowMap(const std::vector<MeshInstance>& objects) const
{
	getShadowMap().set(Texture::Parameter::BaseLevel, 0);
	
	BoundingSphere BoundingVolume(_position, _range);
	
	bind();
	
	for(auto& b : objects)
	{
		if(intersect(b.getAABB(), BoundingVolume))
		{
			getShadowMapProgram().setUniform("ModelMatrix", b.getModelMatrix());
			b.getMesh().draw();
		}
	}
		
	unbind();
	
	getShadowMap().generateMipmaps();
	/// @todo Good blur for Cubemaps
	getShadowMap().set(Texture::Parameter::BaseLevel, downsampling);
	//getShadowMap().generateMipmaps();
}
	
void OmnidirectionalLight::initPrograms()
{
	if(s_depthProgram == nullptr)
	{
		s_depthProgram = &ResourcesManager::getInstance().getProgram("OmnidirectionalLight_Depth");
		s_depthVS = &ResourcesManager::getInstance().getShader<VertexShader>("OmnidirectionalLight_DepthVS");
		s_depthGS = &ResourcesManager::getInstance().getShader<GeometryShader>("OmnidirectionalLight_DepthGS");
		s_depthFS = &ResourcesManager::getInstance().getShader<FragmentShader>("OmnidirectionalLight_DepthFS");
	}
	
	if(s_depthProgram != nullptr && !s_depthProgram->isValid())
	{
		if(!*s_depthVS)
		{
			s_depthVS->loadFromFile("src/GLSL/cubedepth_vs.glsl");
			s_depthVS->compile();
		}
		if(!*s_depthGS)
		{
			s_depthGS->loadFromFile("src/GLSL/cubedepth_gs.glsl");
			s_depthGS->compile();
		}
		if(!*s_depthFS)
		{
			s_depthFS->loadFromFile("src/GLSL/lineardepth_fs.glsl");
			s_depthFS->compile();
		}
		s_depthProgram->attach(*s_depthVS);
		s_depthProgram->attach(*s_depthGS);
		s_depthProgram->attach(*s_depthFS);
		s_depthProgram->link();
	}
}