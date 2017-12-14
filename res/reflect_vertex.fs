#version 330 core
precision mediump float;

// function prototypes
vec2 matcap(vec3 eye, vec3 normal);

out vec4 FragColor;

uniform sampler2D uTexture;
varying vec3 vNormal;
varying vec3 vEye;
varying vec2 vN;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

void main() {
  vec3 base = texture2D( uTexture, vN ).rgb;
  gl_FragColor = vec4( base, 1.0 );
/*
	vec2 uv = matcap(vEye, vNormal).xy;

  gl_FragColor = vec4(
    texture2D(uTexture, uv).rgb,
    1.0
	);
*/
/*
  FragColor = vec4(
    texture2D(uTexture, uv + TexCoords).rgb,
    1.0
  );
*/
}

vec2 matcap(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);
  float m = 2.8284271247461903 * sqrt( reflected.z+1.0 );
  return reflected.xy / m + 0.5;
}
