#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

uniform vec3 uColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform float ambientSize;
uniform float specular;
uniform float specularSize;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	vec3 ambient = ambientSize * lightColor;

	vec3 norm = normalize(f_in.normal);
	vec3 lightDir = normalize(-lightPos);

	float diff = max(dot(-lightDir, norm), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(f_in.position);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specular);
	vec3 specular = specularSize * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * uColor;
	fb_color = vec4(result, 1);
}