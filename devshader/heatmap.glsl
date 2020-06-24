#version 110 

void main()
{
	// transform the vertex position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// transform the texture coordinates
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// forward the vertex color
	gl_FrontColor = gl_Color; // sets bg to black
}
#DIVIDE
uniform sampler2D texture;

#define MAX_PLANETS 10
#define OBJECTS_COUNT 200

uniform int planetCount;
uniform vec2 planetPositions[MAX_PLANETS];
uniform vec3 planetColours[MAX_PLANETS];
uniform int windowHeight;
uniform float colScale;

const float G = 6.67e-11;

void main()
{
	// lookup the pixel in the texture
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	float t = 0.0;
	vec4 col=vec4(0.0, 0.0, 0.0, 1.0);

	for (int i = 0; i < planetCount; ++i)
	{
		planetPositions[i].y = float(windowHeight) - planetPositions[i].y;
		float r = distance(gl_FragCoord.xy, planetPositions[i]);
		// t += (1.0 / (r*r));
		col.xyz += ((1.0 / (r*r)) * planetColours[i]);
	}

	// t /= G;
	col = normalize(col);
	// multiply it by the color
	gl_FragColor = col*colScale;//vec4(t*1e4,0.0,0.0,1.0);// gl_Color * pixel;
}