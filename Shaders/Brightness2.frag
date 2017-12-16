uniform sampler2D source;

const float Threshold = 0.7;
const float Factor = 4.0;

const float fmin = 0.7;

void main()
{
	vec4 sourceFragment = texture2D(source, gl_TexCoord[0].xy);
	float luminance = 1.0;
	sourceFragment *= clamp(luminance - Threshold, 0.0, 1.0) * Factor;
	gl_FragColor = sourceFragment;
}
