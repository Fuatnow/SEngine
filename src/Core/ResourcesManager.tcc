#pragma once

template<typename ShaderType>
inline ShaderType& ResourcesManager::getShader(const std::string& name)
{
	auto it = _shaders.find(name);
	if(it != _shaders.end())
	{
		return *static_cast<ShaderType*>(it->second.get());
	} else {
		auto newShader = new ShaderType();
		_shaders[name].reset(newShader);
		return *newShader;
	}
} 

template<typename T>
inline T& ResourcesManager::getTexture(const std::string& name)
{
	auto it = _textures.find(name);
	if(it != _textures.end())
	{
		return *static_cast<T*>(it->second.get());
	} else {
		auto newTexture = new T();
		_textures[name].reset(newTexture);
		return *newTexture;
	}
} 

template<typename ShaderType>
ShaderType& load(const std::string& path)
{
	auto& s = ResourcesManager::getInstance().getShader<ShaderType>(path);
	s.loadFromFile(path);
	s.compile();
	assert(s);
	return s;
}

template<typename ... ShaderTypes>
Program& loadProgram(const std::string& name, ShaderTypes& ... shaders)
{
	auto& p = ResourcesManager::getInstance().getProgram(name);
	p.attach_chain(shaders...);
	p.link();
	assert(p);
	return p;
}
