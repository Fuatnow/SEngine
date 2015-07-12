#pragma once

#include <functional>

#include <GUIClickable.hpp>
#include <GUIText.hpp>

class GUIButton : public GUIClickable
{
public:
	using ClickFunc = std::function<void(void)>;
	
	GUIButton(const std::string& str, ClickFunc func) :
		_text(str),
		_func(func)
	{
		_aabb = _text.getAABB();
		_aabb.min -= glm::vec2(4.0);
		_aabb.max += glm::vec2(4.0);
	}
	
	bool onClick(const glm::vec2& coords, int button) override
	{
		if(_func)
			_func();
		else
			return false;
		
		return true;
	}
	
	void draw(const glm::vec2& resolution, const glm::vec2& position = glm::vec2(0.0)) override
	{
		auto& P = ResourcesManager::getInstance().getProgram("GUIRectangle");
		if(!P)
		{
			P = loadProgram("GUIRectangle",
					load<VertexShader>("src/GLSL/GUI/rect_vs.glsl"),
					load<GeometryShader>("src/GLSL/GUI/rect_gs.glsl"),
					load<FragmentShader>("src/GLSL/GUI/rect_fs.glsl")
				);
		}
		
		P.use();
		P.setUniform("Resolution", resolution);
		P.setUniform("Position", position + _position);
		P.setUniform("Min", _aabb.min);
		P.setUniform("Max", _aabb.max);
		P.setUniform("Color", glm::vec4(0.0, 0.3, 1.0, 0.25));
		glDrawArrays(GL_POINTS, 0, 1); // Dummy draw call
		P.useNone();
		
		_text.draw(resolution, position + _position);
	}
	
private:
	GUIText		_text;
	ClickFunc		_func;
};
