#version 330 core
precision mediump float;

out vec4 FragColor;

uniform sampler2D uTexture;
varying vec2 vN;

in vec3 FragPos;
in vec3 vNormal;
in vec2 TexCoords;
in vec3 vEye;

uniform vec3 viewPos;

// function prototypes
vec2 matcap(vec3 eye, vec3 normal);

void main() {
  vec2 uv = matcap(vEye, vNormal);
  FragColor = vec4(
    texture2D(uTexture, uv).rgb,
    1.0
  );
}

vec2 matcap(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);
  // sqrt(0.5) * 4
  float m = 2.8284271247461903 * sqrt( reflected.z + 1.0 );
  return reflected.xy / m + 0.5;
}
