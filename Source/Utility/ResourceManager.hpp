#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <unordered_map>
#include <memory>

enum class FontID
{
	Console,
	UI
};

enum class ShaderID
{
	Brightness,
	Brightness2,
	DownSample,
	GaussianBlur,
	Add
};

enum class SoundID
{
	Armor, // TODO: Wear/take off
	Bell, // TODO:
	Confusion,
	Eat,
	Experience,
	Fire,
	Found, // TODO: Search
	Gold,
	Ice,
	MonsterDie,
	MonsterHit,
	MonsterMiss,
	RogueDie,
	RogueHit,
	RogueMiss,
	Stairs,
	Teleport,
	Tick, // TODO: NoCommand
	Trap,
	Win, // TODO: Victory
	Zap,
};

class ResourceManager
{
public:
	void loadFont(FontID id, const std::string& filename);
	sf::Font& getFont(FontID id);
	
	void loadShader(ShaderID id, const std::string& vertexShader, const std::string& fragmentShader);
	sf::Shader& getShader(ShaderID id);

	void loadSound(SoundID id, const std::string& filename);
	sf::SoundBuffer& getSound(SoundID id);

private:
	std::unordered_map<FontID, std::unique_ptr<sf::Font>> fonts;
	std::unordered_map<ShaderID, std::unique_ptr<sf::Shader>> shaders;
	std::unordered_map<SoundID, std::unique_ptr<sf::SoundBuffer>> sounds;
};
