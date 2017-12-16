#pragma once

namespace sf
{
	class RenderTarget;
	class RenderTexture;
	class Shader;
}

class PostEffect
{
public:
	PostEffect() = default;
	virtual ~PostEffect() = default;

	PostEffect(const PostEffect&) = delete;
	PostEffect& operator=(const PostEffect&) = delete;

	virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output) = 0;

	static bool isSupported();

protected:
	static void applyShader(const sf::Shader& shader, sf::RenderTarget& output);
};
