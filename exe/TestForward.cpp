#include <sstream>
#include <iomanip>

#include <Query.hpp>

#include <Random.hpp>
#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>
#include <GUIText.hpp>
#include <GUIButton.hpp>
#include <GUICheckbox.hpp>
#include <GUIEdit.hpp>
#include <GUIGraph.hpp>
#include <GUISeparator.hpp>
#include <Axes.hpp>

#include <glm/gtx/transform.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public Application
{
public:
	Test(int argc, char* argv[]) : 
		Application(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		Application::run_init();
		
		auto& LightDraw = loadProgram("LightDraw",
			load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			load<FragmentShader>("src/GLSL/Debug/light_star_fs.glsl")
		);
		
		auto& Simple = loadProgram("Simple",
			load<VertexShader>("src/GLSL/vs.glsl"),
			load<FragmentShader>("src/GLSL/fs.glsl")
		);
		Simple.bindUniformBlock("Camera", _camera_buffer); 
		
		auto& Forward = loadProgram("Forward",
			load<VertexShader>("src/GLSL/Forward/forward_vs.glsl"),
			load<FragmentShader>("src/GLSL/Forward/forward_fs.glsl")
		);
		Forward.bindUniformBlock("Camera", _camera_buffer); 
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());
		
		auto TestMesh = Mesh::load("in/3DModels/sponza/sponza.obj", Forward);
		for(auto part : TestMesh)
		{
			part->createVAO();
			_scene.add(MeshInstance(*part, glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))));
		}
		
		for(size_t i = 0; i < 1000; ++i)
		{
			_scene.getPointLights().push_back(PointLight{
				glm::vec3(0.0f), 	// Position
				10.0f,
				glm::vec3(1.0), // Color
				0.0f
			});
		}
		
		_partVelocities.resize(_scene.getPointLights().size());
		
		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/ame_nebula/posx.tga",
				"in/Textures/skybox/ame_nebula/negx.tga",
				"in/Textures/skybox/ame_nebula/posy.tga",
				"in/Textures/skybox/ame_nebula/negy.tga",
				"in/Textures/skybox/ame_nebula/posz.tga",
				"in/Textures/skybox/ame_nebula/negz.tga"});
		
		auto w = _gui.add(new GUIWindow());
		w->add(new GUIGraph<float>("Update (ms): ", [&]() -> float { return 1000.f * _updateTiming; }, 0.0, 15.0, 7.5));
		w->add(new GUISeparator(w));
		w->add(new GUIGraph<float>("Frame Time (ms): ", [&]() -> float { return 1000.f * TimeManager::getInstance().getRealDeltaTime(); }, 0.0, 20.0, 7.5));
		w->add(new GUIGraph<float>("FPS: ", [&]() -> float { return TimeManager::getInstance().getInstantFrameRate(); }, 0.0, 450.0, 7.5));
		w->add(new GUIText([&]() -> std::string {
			return to_string(1000.f * TimeManager::getInstance().getRealDeltaTime(), 1) + "ms - " + 
						to_string(1.0f/TimeManager::getInstance().getRealDeltaTime(), 0) + " FPS";
		}));
		w->add(new GUISeparator(w));
		w->add(new GUIText("Stats"));
		
		auto w2 = _gui.add(new GUIWindow());
		w2->add(new GUICheckbox("Pause", &_paused));
		w2->add(new GUISeparator(w2));
		w2->add(new GUIText("Controls"));
		
		_axes.reset(new Axes());
	}

	virtual void update() override
	{
		auto start = TimeManager::getInstance().getRuntime();
		Application::update();
		
		if(!_paused)
		{
			static float time = 0.0;
			static float cooldown = 0.0;
			
			time += _frameTime;
			cooldown += _frameTime;
			if(cooldown >  1.0)
			{
				cooldown -= 1.0;
				RandomHelper r;
				size_t first = 100 * (static_cast<size_t>(time) % 10);
				auto p = (1.0f + 5.0f * (float) r.get0_1()) * r.getSpherical();
				auto c = 0.5f + 0.5f * glm::vec3(r.get0_1(), r.get0_1(), r.get0_1());
				// Flash
				_scene.getPointLights()[first].info = 0.5;
				_scene.getPointLights()[first].range = 5.0;
				_scene.getPointLights()[first].position = p;
				_scene.getPointLights()[first].color = 2.0f * c;
				_partVelocities[first] = glm::vec3(0.0, 0.5 * 9.81, 0.0);
				for(size_t i = 1; i < 100; ++i)
				{
					_scene.getPointLights()[first + i].info = 9.0;
					_scene.getPointLights()[first + i].position = p;
					_scene.getPointLights()[first + i].color = c;
					
					auto v = r.getSpherical();
					float s = 5.0 + 10.0 * r.get0_1();
					_partVelocities[first + i] = s * v;
				}
			}
			
			for(size_t i = 0; i < _partVelocities.size(); ++i)
			{
				if(_scene.getPointLights()[i].info > 0.0)
				{
					_scene.getPointLights()[i].info -= _frameTime;
					_partVelocities[i] += _frameTime * glm::vec3(0.0f, -9.8f, 0.0);
					_scene.getPointLights()[i].position += _frameTime * _partVelocities[i];
				}
			}
		}
		ResourcesManager::getInstance().getProgram("Forward").setUniform("CameraPosition", _camera.getPosition());
		
		_updateTiming = TimeManager::getInstance().getRuntime() - start;
	}

	virtual void render() override
	{
		Context::clear();
	
		_scene.draw(_projection, _camera.getMatrix());
		
		_lightsTiming.begin(Query::Target::TimeElapsed);
		
		Context::disable(Capability::CullFace);
		auto& ld = ResourcesManager::getInstance().getProgram("LightDraw");
		ld.setUniform("CameraPosition", _camera.getPosition());
		ld.use();
		glDrawArrays(GL_POINTS, 0, _scene.getPointLights().size());
		ld.useNone();
		
		_lightsTiming.end();
		
		Context::enable(Capability::Blend);
		auto& s = ResourcesManager::getInstance().getProgram("Simple");
		s.use();
		s.setUniform("Color", glm::vec4(1.0));
		_axes->draw();
		s.setUniform("Color", glm::vec4(1.0, 1.0, 1.0, 0.25));
		_axes->drawMarks();
		s.useNone();
		
		renderGUI();
	}
	
protected:
	float	_updateTiming;
	Query	_lightsTiming;
	
	std::vector<glm::vec3>	_partVelocities;
	std::unique_ptr<Axes>	_axes;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
