#include <sstream>
#include <iomanip>
#include <deque>

#include <glm/gtx/transform.hpp>
#include <glmext.hpp>
#include <imgui.h>

#include <Query.hpp>

#include <SpotLight.hpp>
#include <OrthographicLight.hpp>
#include <DeferredRenderer.hpp>

#include <MathTools.hpp>

#include <Log.hpp>

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
	out << std::fixed;
    out << std::setprecision(n) << a_value;
    return out.str();
}

class Test : public DeferredRenderer
{
public:
	Test(int argc, char* argv[]) : 
		DeferredRenderer(argc, argv)
	{
	}
	
	virtual void run_init() override
	{
		DeferredRenderer::run_init();
		
		auto& LightDraw = Resources::loadProgram("LightDraw",
			Resources::load<VertexShader>("src/GLSL/Debug/light_vs.glsl"),
			Resources::load<GeometryShader>("src/GLSL/Debug/light_gs.glsl"),
			Resources::load<FragmentShader>("src/GLSL/Debug/light_fs.glsl")
		);
		
		_camera.speed() = 15;
		_camera.setPosition(glm::vec3(0.0, 15.0, -20.0));
		_camera.lookAt(glm::vec3(0.0, 5.0, 0.0));
		
		LightDraw.bindUniformBlock("Camera", _camera_buffer); 

		float R = 0.95f;
		float F0 = 0.15f;
		const auto Paths = {
			"in/3DModels/sponza/sponza.obj"
			//,"in/3DModels/sibenik/sibenik.obj"
		};
		const auto Matrices = {
			glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.04))
			//,glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(200.0, 0.0, 0.0)), glm::vec3(3.0))
		};
		for(size_t i = 0; i < Paths.size(); ++i)
		{
			auto m = Mesh::load(Paths.begin()[i]);
			for(auto& part : m)
			{
				part->createVAO();
				part->getMaterial().setUniform("R", R);
				part->getMaterial().setUniform("F0", F0);
				_scene.add(MeshInstance(*part, Matrices.begin()[i]));
			}
		}

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.8, 7.1, -1.5), 	// Position
			10.0f,
			glm::vec3(2.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(42.0, 23.1, 16.1), 	// Position
			15.0f,
			glm::vec3(2.0), // Color
			0.0f
		});

		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-50.0, 22.8, -18.6), 	// Position
			20.0f,
			glm::vec3(2.0), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, 5.8), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-24.7, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(19.5, 5.4, -8.7), 	// Position
			5.0f,
			glm::vec3(0.8, 0.1, 0.2), // Color
			0.0f
		});
		
		_scene.getPointLights().push_back(PointLight{
			glm::vec3(-47.0, 4.5, -1.5), 	// Position
			20.0f,
			glm::vec3(1.8), // Color
			0.0f
		});
		
		LightDraw.bindUniformBlock("LightBlock", _scene.getPointLightBuffer());

		_volumeSamples = 16;
		// Shadow casting lights ---------------------------------------------------
		
		OrthographicLight* o = _scene.add(new OrthographicLight());
		o->init();
		o->dynamic = false;
		o->setColor(glm::vec3(2.0));
		o->setDirection(glm::normalize(glm::vec3{58.8467 - 63.273, 161.167 - 173.158, -34.2005 - -37.1856}));
		o->_position = glm::vec3{63.273, 173.158, -37.1856};
		o->updateMatrices();
		
		OrthographicLight* o2 = _scene.add(new OrthographicLight());
		o2->init();
		o2->dynamic = false;
		o2->setColor(glm::vec3(2.0));         
		o2->setDirection(glm::normalize(glm::vec3{220.472 - 63.273, -34.6538 - 0.0, 0.789395 - 0.0}));
		o2->_position = glm::vec3{127.27, 0.0, 0.0};
		o2->updateMatrices();
		
		SpotLight* s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(45.0, 12.0, -18.0));
		s->lookAt(glm::vec3(45.0, 0.0, -18.0));
		s->setRange(20.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		
		s = _scene.add(new SpotLight());
		s->init();
		s->setColor(glm::vec3(1.5));
		s->setPosition(glm::vec3(0.0, 20.0, 00.0));
		s->lookAt(glm::vec3(0.0, 0.0, 0.0));
		s->setRange(50.0f);
		s->setAngle(3.14159f * 0.5f);
		s->updateMatrices();
		/*
		_scene.getOmniLights().resize(2);
		_scene.getOmniLights()[0].setResolution(2048);
		_scene.getOmniLights()[0].dynamic = true;
		_scene.getOmniLights()[0].init();
		_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0, 25.0, -2.0));
		_scene.getOmniLights()[0].setColor(glm::vec3(1.5));
		_scene.getOmniLights()[0].setRange(40.0f);
		_scene.getOmniLights()[0].updateMatrices();
		
		_scene.getOmniLights()[1].setResolution(2048);
		_scene.getOmniLights()[1].dynamic = true;
		_scene.getOmniLights()[1].init();
		_scene.getOmniLights()[1].setPosition(glm::vec3(200.0, -28.0, 0.0));
		_scene.getOmniLights()[1].setColor(glm::vec3(0.5));
		_scene.getOmniLights()[1].setRange(150.0f);
		_scene.getOmniLights()[1].updateMatrices();
		*/
		for(size_t i = 0; i < _scene.getLights().size(); ++i)
			_scene.getLights()[i]->drawShadowMap(_scene.getObjects());
		
		for(size_t i = 0; i < _scene.getOmniLights().size(); ++i)
			_scene.getOmniLights()[i].drawShadowMap(_scene.getObjects());

		_scene.getSkybox().loadCubeMap({"in/Textures/skybox/posx.png",
				"in/Textures/skybox/negx.png",
				"in/Textures/skybox/posy.png",
				"in/Textures/skybox/negy.png",
				"in/Textures/skybox/posz.png",
				"in/Textures/skybox/negz.png"});
	}
	
	virtual void update() override
	{
		_updateTiming.begin(Query::Target::TimeElapsed);
		if(!_paused)
		{
			if(_scene.getPointLights().size() > 6)
			{
				_scene.getPointLights()[3].position = glm::vec3(19.5, 5.4, 5.8) +  glm::ballRand(0.25f);
				_scene.getPointLights()[3].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
				
				_scene.getPointLights()[4].position = glm::vec3(-24.7, 5.4, 5.8) +  glm::ballRand(0.25f);
				_scene.getPointLights()[4].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
				
				_scene.getPointLights()[5].position = glm::vec3(-24.7, 5.4, -8.7) +  glm::ballRand(0.25f);
				_scene.getPointLights()[5].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
				
				_scene.getPointLights()[6].position = glm::vec3(19.5, 5.4, -8.7) +  glm::ballRand(0.25f);
				_scene.getPointLights()[6].color = glm::vec3(0.8, 0.28, 0.2) * (4.0f + 0.75f * rand<float>());
			}
			
			if(!_scene.getOmniLights().empty() && _scene.getOmniLights()[0].dynamic)
				_scene.getOmniLights()[0].setPosition(glm::vec3(-20.0 + 15.0 * cos(_time), 25.0, -2.0));
			
			if(_scene.getOmniLights().size() > 1 && _scene.getOmniLights()[1].dynamic)
				_scene.getOmniLights()[1].setPosition(glm::vec3(200.0 + 50.0 * cos(0.1 * _time), -25.0, 0.0));
		}
	
		DeferredRenderer::update();
		_updateTiming.end();
	}
	
	virtual void renderGUI() override
	{	
		// Plots
		static float last_update = 2.0;
		last_update += TimeManager::getInstance().getRealDeltaTime();
		static std::deque<float> frametimes;
		static std::deque<float> updatetimes;
		static std::deque<float> gbuffertimes;
		static std::deque<float> lighttimes;
		static std::deque<float> postprocesstimes;
		static std::deque<float> guitimes;
		const size_t max_samples = 100;
		float ms = TimeManager::getInstance().getRealDeltaTime() * 1000;
		if(last_update > 0.05 || frametimes.empty())
		{
			if(frametimes.size() > max_samples) frametimes.pop_front();
			frametimes.push_back(ms);
			if(updatetimes.size() > max_samples) updatetimes.pop_front();
			updatetimes.push_back(_updateTiming.get<GLuint64>() / 1000000.0);
			if(gbuffertimes.size() > max_samples) gbuffertimes.pop_front();
			gbuffertimes.push_back(_GBufferPassTiming.get<GLuint64>() / 1000000.0);
			if(lighttimes.size() > max_samples) lighttimes.pop_front();
			lighttimes.push_back(_lightPassTiming.get<GLuint64>() / 1000000.0);
			if(postprocesstimes.size() > max_samples) postprocesstimes.pop_front();
			postprocesstimes.push_back(_postProcessTiming.get<GLuint64>() / 1000000.0);
			if(guitimes.size() > max_samples) guitimes.pop_front();
			guitimes.push_back(_lastGUITiming / 1000000.0);
			last_update = 0.0;
		}
		
		ImGui::Begin("Statistics");
		{
			ImGui::Text("%.4f ms/frame (%.1f FPS)", 
				frametimes.back(), 
				1000.0 / frametimes.back()
			);
			auto lamba_data = [](void* data, int idx) {
				return static_cast<std::deque<float>*>(data)->at(idx);
			};
			ImGui::PlotLines("FrameTime", lamba_data, &frametimes, frametimes.size(), 0, to_string(frametimes.back(), 4).c_str(), 0.0, 20.0); 
			ImGui::PlotLines("Update", lamba_data, &updatetimes, updatetimes.size(), 0, to_string(updatetimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GBuffer", lamba_data, &gbuffertimes, gbuffertimes.size(), 0, to_string(gbuffertimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Lights", lamba_data, &lighttimes, lighttimes.size(), 0, to_string(lighttimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("Post Process",lamba_data, &postprocesstimes, postprocesstimes.size(), 0, to_string(postprocesstimes.back(), 4).c_str(), 0.0, 10.0);    
			ImGui::PlotLines("GUI", lamba_data, &guitimes, guitimes.size(), 0, to_string(guitimes.back(), 4).c_str(), 0.0, 10.0);         
		}
		ImGui::End();
		
		ImGui::Begin("Debug");
		{
			ImGui::Checkbox("Pause", &_paused);
			ImGui::SliderFloat("Time Scale", &_timescale, 0.0f, 5.0f);
			if(ImGui::Button("Update shadow maps"))
			{
				for(auto l : _scene.getLights())
				{
					l->updateMatrices();
					l->drawShadowMap(_scene.getObjects());
				}
				
				for(auto& l : _scene.getOmniLights())
				{
					l.updateMatrices();
					l.drawShadowMap(_scene.getObjects());
				}
			}
			ImGui::Separator(); 
			ImGui::Checkbox("Toggle Debug", &_debug_buffers);
			const char* debugbuffer_items[] = {"Color","Position", "Normal"};
			const Attachment debugbuffer_values[] = {Attachment::Color0, Attachment::Color1, Attachment::Color2};
			static int debugbuffer_item_current = 0;
			if(ImGui::Combo("Buffer to Display", &debugbuffer_item_current, debugbuffer_items, 3))
				_framebufferToBlit = debugbuffer_values[debugbuffer_item_current];
		}
		ImGui::End();
		
		ImGui::Begin("Rendering Options");
		{
			if(ImGui::Checkbox("Fullscreen", &_fullscreen))
				setFullscreen(_fullscreen);
			ImGui::SameLine();
			if(ImGui::Checkbox("Vsync", &_vsync))
				glfwSwapInterval(_vsync);
			ImGui::Text("Window resolution: %d * %d", _width, _height);
			const char* internal_resolution_items[] = {"Windows resolution", "1920 * 1080", "2715 * 1527", "3840 * 2160"};
			static int internal_resolution_item_current = 0;
			if(ImGui::Combo("Internal Resolution", &internal_resolution_item_current, internal_resolution_items, 4))
			{
				switch(internal_resolution_item_current)
				{
					case 0: setInternalResolution(0, 0); break;
					case 1: setInternalResolution(1920, 1080); break;
					case 2: setInternalResolution(2715, 1527); break;
					case 3: setInternalResolution(3840, 2160); break;
				}
			}
			
			ImGui::Separator();
			
			static bool bloom_toggle = _bloom > 0.0;
			if(ImGui::Checkbox("Toggle Bloom", &bloom_toggle))
				_bloom = -_bloom;
			ImGui::DragFloat("Bloom", &_bloom, 0.05, 0.0, 5.0);
			ImGui::DragFloat("Exposure", &_exposure, 0.05, 0.0, 5.0);
			ImGui::DragFloat("MinVariance (VSM)", &_minVariance, 0.000001, 0.0, 0.00005);
			ImGui::DragInt("AOSamples", &_aoSamples, 1, 0, 32);
			ImGui::DragFloat("AOThresold", &_aoThreshold, 0.05, 0.0, 5.0);
			ImGui::DragFloat("AORadius", &_aoRadius, 1.0, 0.0, 400.0);
			ImGui::DragInt("VolumeSamples", &_volumeSamples, 1, 0, 64);
			ImGui::DragFloat("AtmosphericDensity", &_atmosphericDensity, 0.001, 0.0, 0.02);
		
			ImGui::Separator();

			ImGui::ColorEdit3("Ambiant Color", &_ambiant.r);
		}
		ImGui::End();
		
		ImGui::Begin("Scene");
		{
			if(ImGui::TreeNode(("Objects (" + std::to_string(_scene.getObjects().size()) + ")").c_str()))
			{
				for(auto& o : _scene.getObjects())
				{
					ImGui::PushID(&o);
					ImGui::Text("Object");
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			
			if(ImGui::TreeNode(("Directional Lights (" + std::to_string(_scene.getLights().size()) + ")").c_str()))
			{
				for(auto& l : _scene.getLights())
				{
					ImGui::PushID(&l);
					ImGui::PushItemWidth(150);
					float c[3] = {l->getColor().r, l->getColor().g, l->getColor().b};
					if(ImGui::InputFloat3("Color", c))
						l->setColor(glm::vec3{c[0], c[1], c[2]});
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			
			if(ImGui::TreeNode(("Point Lights (" + std::to_string(_scene.getPointLights().size()) + ")").c_str()))
			{
				for(auto& l : _scene.getPointLights())
				{
					ImGui::PushID(&l);
					ImGui::PushItemWidth(150);
					ImGui::InputFloat3("Position", &l.position.x);
					ImGui::SameLine();
					ImGui::InputFloat3("Color", &l.color.r);
					ImGui::SameLine();
					ImGui::PushItemWidth(50);
					ImGui::InputFloat("Range", &l.range);
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
		
		ImGui::Begin("Logs");
		{
			const ImVec4 LogColors[3] = {
				ImVec4{1, 1, 1, 1},
				ImVec4{1, 1, 0, 1},
				ImVec4{1, 0, 0, 1}
			};
			static int log_level_current = 0;
			ImGui::Combo("Log Level", &log_level_current, Log::_log_types.data(), 3);
			std::vector<Log::LogLine*> tmp_logs;
			if(log_level_current > 0)
				for(auto& l : Log::_logs)
					if(log_level_current <= l.type)
						tmp_logs.push_back(&l);
			
			ImGui::BeginChild("Logs Lines");
			ImGuiListClipper clipper(log_level_current > 0 ? tmp_logs.size() : Log::_logs.size());
			while(clipper.Step())
				for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					if(log_level_current > 0)
						ImGui::TextColored(LogColors[tmp_logs[i]->type], "%s", tmp_logs[i]->str().c_str());
					else
						ImGui::TextColored(LogColors[Log::_logs[i].type], "%s", std::string(Log::_logs[i]).c_str());
				}
			ImGui::EndChild();
		}
		ImGui::End();
		
		if(_selectedObject != nullptr)
		{
			auto aabb = _selectedObject->getAABB().getBounds();
			std::array<ImVec2, 8> screen_aabb;
			for(int i = 0; i < 8; ++i)
			{
				screen_aabb[i] = project(aabb[i]);
			}
			// Dummy Window for "on field" widgets
			ImGui::SetNextWindowSize(ImVec2{static_cast<float>(_width), static_cast<float>(_height)});
			ImGui::Begin("SelectedObject", nullptr, ImVec2{static_cast<float>(_width), static_cast<float>(_height)}, 0.0,
				ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
			ImDrawList* drawlist = ImGui::GetWindowDrawList();
			
			// Bounding Box Gizmo
			constexpr std::array<size_t, 24> segments{
				0, 1, 1, 3, 3, 2, 2, 0,
				4, 5, 5, 7, 7, 6, 6, 4,
				0, 4, 1, 5, 2, 6, 3, 7
			};
			for(int i = 0; i < 24; i += 2)
				drawlist->AddLine(screen_aabb[segments[i]], screen_aabb[segments[i + 1]], 
					ImGui::ColorConvertFloat4ToU32(ImVec4(0.0, 0.0, 1.0, 0.5)));
			
			/////////////////////////////////////
			// Position Gizmo
			// @todo Debug it, Clean in, Package it 
			const std::array<glm::vec2, 4> gizmo_points{
				project(_selectedObject->getTransformation().getPosition() + glm::vec3{0.0, 0.0, 0.0}),
				project(_selectedObject->getTransformation().getPosition() + glm::vec3{1.0, 0.0, 0.0}),
				project(_selectedObject->getTransformation().getPosition() + glm::vec3{0.0, 1.0, 0.0}),
				project(_selectedObject->getTransformation().getPosition() + glm::vec3{0.0, 0.0, 1.0})
			};
			static bool dragging[3] = {false, false, false};
			static glm::vec3 origin_position;
			for(int i = 0; i < 3; ++i)
			{
				if(dragging[i] && ImGui::IsMouseDragging(0))
				{
					auto newP = glm::vec2{ImGui::GetMousePos()};
					auto oldP = newP - glm::vec2{ImGui::GetMouseDragDelta()};
					auto newR = getScreenRay(newP.x, newP.y);
					auto oldR = getScreenRay(oldP.x, oldP.y);
					Plane pl{_selectedObject->getTransformation().getPosition(), -_camera.getDirection()}; // @todo Project onto something else (line)
					float d0 = std::numeric_limits<float>::max(), d1 = std::numeric_limits<float>::max();
					glm::vec3 p0, p1, n0, n1;
					trace(newR, pl, d0, p0, n0);
					trace(oldR, pl, d1, p1, n1);
					auto newPosition = origin_position;
					newPosition[i] += p0[i] - p1[i];
					_selectedObject->getTransformation().setPosition(newPosition);
				} 
				if(dragging[i] && ImGui::IsMouseReleased(0))
				{
					dragging[i] = false;
					ImGui::GetIO().WantCaptureMouse = false;
				}
				if(ImGui::IsMouseClicked(0) && point_line_distance(glm::vec2{_mouse}, gizmo_points[0], gizmo_points[1 + i]) < 5.0f)
				{
					dragging[i] = true;
					origin_position = _selectedObject->getTransformation().getPosition();
				}
			}
			if(dragging[0] || dragging[1] || dragging[2])
				ImGui::GetIO().WantCaptureMouse = true;
			
			drawlist->AddLine(gizmo_points[0], gizmo_points[1], ImGui::ColorConvertFloat4ToU32(ImVec4(1.0, 0.0, 0.0, dragging[0] ? 1.0 : 0.5)), 2.0);
			drawlist->AddLine(gizmo_points[0], gizmo_points[2], ImGui::ColorConvertFloat4ToU32(ImVec4(0.0, 1.0, 0.0, dragging[1] ? 1.0 : 0.5)), 2.0);
			drawlist->AddLine(gizmo_points[0], gizmo_points[3], ImGui::ColorConvertFloat4ToU32(ImVec4(0.0, 0.0, 1.0, dragging[2] ? 1.0 : 0.5)), 2.0);
			////////////////////////////////////////////////////
			
			ImGui::End();
		}
		
		ImGui::Begin("Object Inspector");
		if(_selectedObject != nullptr)
		{
			ImGui::Text("Name: %s", _selectedObject->getMesh().getName().c_str());
			ImGui::Text("Path: %s", _selectedObject->getMesh().getPath().c_str());
			glm::vec3 p = _selectedObject->getTransformation().getPosition();
			if(ImGui::InputFloat3("Position", &p.x))
			{
				_selectedObject->getTransformation().setPosition(p);
			}
			glm::quat r = _selectedObject->getTransformation().getRotation();
			if(ImGui::InputFloat4("Rotation", &r.x))
			{
				_selectedObject->getTransformation().setRotation(r);
			}
			glm::vec3 s = _selectedObject->getTransformation().getScale();
			if(ImGui::InputFloat3("Scale", &s.x))
			{
				_selectedObject->getTransformation().setScale(s);
			}
			
			if(ImGui::TreeNode("Material"))
			{
				Uniform<glm::vec3>* uniform = _selectedObject->getMaterial().searchUniform<glm::vec3>("Color");
				if(uniform != nullptr)
				{
					// @todo Yes, Color is set on select, so this is stupid :D
					ImGui::Text("Color: %f, %f, %f", uniform->getValue().x, uniform->getValue().y, uniform->getValue().z);
				}
				auto uniform_tex = _selectedObject->getMaterial().searchUniform<Texture>("Texture");
				if(uniform_tex != nullptr)
				{
					ImGui::Text("Has a Texture");
				}
				ImGui::TreePop();
			}
		} else {
			ImGui::Text("No object selected.");
		}
		ImGui::End();

		if(!ImGui::GetIO().WantCaptureMouse)
		{
			if(ImGui::IsMouseClicked(0))
			{
				const auto r = getMouseRay();
				float depth = std::numeric_limits<float>::max();
				if(_selectedObject)
					_selectedObject->getMaterial().setUniform("Color", _selectedObjectColor);
				_selectedObject = nullptr;
				for(auto& o : _scene.getObjects())
				{
					if(trace(r, o, depth))
					{
						_selectedObject = &o;
					}
				}
				if(_selectedObject)
				{
					_selectedObjectColor = _selectedObject->getMaterial().getUniform<glm::vec3>("Color");
					_selectedObject->getMaterial().setUniform("Color", glm::vec3{0.5, 0.5, 1.5});
				}
			}
		}
		
		DeferredRenderer::renderGUI();
	}
		
protected:
	MeshInstance*	_selectedObject = nullptr;
	glm::vec3		_selectedObjectColor;
};

int main(int argc, char* argv[])
{
	Test _app(argc, argv);
	_app.init();	
	_app.run();
}
