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
/*
	mat4 modelViewMatrix = model * view;
  vec4 p = vec4( aPos, 1. );

  vec3 e = normalize( vec3( modelViewMatrix * p ) );
  vec3 n = normalize( normalMatrix * aNormal );

  vec3 r = reflect( e, n );
  float m = 2. * sqrt(
    pow( r.x, 2. ) +
    pow( r.y, 2. ) +
    pow( r.z + 1., 2. )
  );
  vN = r.xy / m + .5;

	Normal = vN;
	FragPos = p;

  gl_Position = projectionMatrix * modelViewMatrix * p;


*/
  FragPos = vec3(model * vec4(aPos, 1.0));

  vec4 p = vec4( aPos, 1. );
  vEye = uEye;
  Normal = normalize((mNormal * vec4(aNormal, 0.0)).xyz);
	vNormal = Normal;
  TexCoords = aTexCoords;

  gl_Position = projection * model * view * p;
  //gl_Position = projection * model * view * vec4(FragPos, 1.0);

}


/*
in vec4 modelViewMatrix;
in vec4 normalMatrix;


void main() {

  vec4 p = vec4( aPos, 1. );

  vec3 e = normalize( vec3( modelViewMatrix * p ) );
  vec3 n = normalize( normalMatrix * aNormal );

  vec3 r = reflect( e, n );
  float m = 2. * sqrt(
    pow( r.x, 2. ) +
    pow( r.y, 2. ) +
    pow( r.z + 1., 2. )
  );
  vN = r.xy / m + .5;

  gl_Position = projectionMatrix * modelViewMatrix * p;
}
*/


