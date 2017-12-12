#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

precision mediump float;

varying vec3 vNormal;
varying vec3 vEye;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 mNormal;
uniform vec3 uEye;

void main() {

  FragPos = vec3(model * vec4(aPos, 1.0));

  vEye = uEye;
  Normal = normalize((mNormal * vec4(aNormal, 0.0)).xyz);
  TexCoords = aTexCoords;

  gl_Position = projection * model * view * vec4(aPos, 1.0);
  //gl_Position = projection * model * view * vec4(FragPos, 1.0);
}

