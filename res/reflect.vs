#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

precision mediump float;

out vec3 FragPos;
out vec3 vNormal;
out vec2 TexCoords;
out vec3 vEye;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mNormal;
uniform vec3 uEye;

void main() {
	// Transform the normal by the model matrix (which because normals transform
	// differently means multiplication by the normal matrix):
	vNormal = normalize(vec3(mNormal * vec4(aNormal, 0.0)));
	// Transform the position by the model matrix:
	vec4 mp = model * vec4(aPos, 1.0);
	// Compute the direction of the eye relative to the position:
	vEye = normalize(mp.xyz - uEye);
	// Transfomr the *directions* of the normal and position-relative-to-eye so
	// that the matcap stays aligned with the view:
	vNormal = mat3(view) * vNormal;
	vEye = mat3(view) * vEye;
	gl_Position = projection * view * mp;
}


