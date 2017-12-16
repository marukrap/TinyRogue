#include "ResourceManager.hpp"

#include <stdexcept> // runtime_error
#include <cassert>

void ResourceManager::loadFont(FontID id, const std::string& filename)
{
	auto font = std::make_unique<sf::Font>();

	if (!font->loadFromFile(filename))
		throw std::runtime_error("ResourceManager::loadFont - Failed to load " + filename);

	fonts[id] = std::move(font);
}

sf::Font& ResourceManager::getFont(FontID id)
{
	auto found = fonts.find(id);
	assert(found != fonts.end());

	return *found->second;
}

void ResourceManager::loadShader(ShaderID id, const std::string& vertexShader, const std::string& fragmentShader)
{
	auto shader = std::make_unique<sf::Shader>();

	if (!shader->loadFromFile(vertexShader, fragmentShader))
		throw std::runtime_error("ResourceManager::loadShader - Failed to load " + vertexShader);

	shaders[id] = std::move(shader);
}

sf::Shader& ResourceManager::getShader(ShaderID id)
{
	auto found = shaders.find(id);
	assert(found != shaders.end());

	return *found->second;
}

void ResourceManager::loadSound(SoundID id, const std::string& filename)
{
	auto sound = std::make_unique<sf::SoundBuffer>();

	if (!sound->loadFromFile(filename))
		throw std::runtime_error("ResourceManager::loadSound - Failed to load " + filename);

	sounds[id] = std::move(sound);
}

sf::SoundBuffer& ResourceManager::getSound(SoundID id)
{
	auto found = sounds.find(id);
	assert(found != sounds.end());

	return *found->second;
}
