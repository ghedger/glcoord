#version 330 core
precision mediump float;

out vec4 FragColor;

uniform sampler2D uTexture;
varying vec3 vNormal;
varying vec3 vEye;

in vec3 FragPos;

uniform vec3 viewPos;
// #pragma glslify: matcap = require(./matcap)

// function prototypes
vec2 matcap(vec3 eye, vec3 normal);


void main() {

  vec3 norm = normalize(vNormal);

  vec2 uv = matcap(vEye, vNormal).xy;

  FragColor = vec4(
    texture2D(uTexture, uv).rgb,
    1.0
  );
}

vec2 matcap(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);
  float m = 2.8284271247461903 * sqrt( reflected.z+1.0 );
  return reflected.xy / m + 0.5;
}

