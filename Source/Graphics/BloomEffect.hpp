#pragma once

// Credit: SFML Game Development Book
// https://github.com/SFML/SFML-Game-Development-Book

#include "PostEffect.hpp"

#include <SFML/Graphics/RenderTexture.hpp>

#include <array>

class ResourceManager;

class BloomEffect : public PostEffect
{
public:
	using RenderTextureArray = std::array<sf::RenderTexture, 2>;

public:
	explicit BloomEffect(ResourceManager& resources);

	void apply(const sf::RenderTexture& input, sf::RenderTarget& output) override;

private:
	void prepareTextures(const sf::Vector2u& size);

	void filterBright(const sf::RenderTexture& input, sf::RenderTexture& output);
	void blurMultipass(RenderTextureArray& renderTextures);
	void blur(const sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f offsetFactor);
	void downsample(const sf::RenderTexture& input, sf::RenderTexture& output);
	void add(const sf::RenderTexture& source, const sf::RenderTexture& bloom, sf::RenderTarget& target);

private:
	ResourceManager& resources;
	sf::RenderTexture brightnessTexture;
	RenderTextureArray firstPassTextures;
	RenderTextureArray secondPassTextures;

// REMOVE:
public:
	static bool hallucination;
};
