#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

precision mediump float;

varying vec3 vNormal;
varying vec3 vEye;
varying vec2 vN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mNormal;
uniform vec3 uEye;

void main() {

	// Source: https://github.com/hughsk/matcap/blob/master/matcap.vert
	//TexCoords = aTexCoords;
	vEye = uEye;
  vNormal = normalize( ( mNormal *vec4(aNormal, 0.0)).xyz);

	//gl_Position = projection * view * model * vec4(aPos, 1.0);

  //Promising results; only shades vertice though
  //Source: https://github.com/inoook/UnityMatCapShader/blob/master/Assets/matCap/Shader/MatCap.shader

	vec4 oPos = projection * view * model * vec4( aPos, 1.0 );

  vec4 p = vec4( aPos, 1.0 );
  vec3 e = normalize( vec3( view * model * p ) );
  vec3 n = normalize( vec3( view * model * vec4( vNormal, 0.0 ) ) );

  vec3 r = reflect( e, n );
  float m = 2.0 * sqrt(
    pow( r.x, 2.0 ) +
    pow( r.y, 2.0 ) +
    pow( r.z + 1.0, 2.0 )
  );
  vN = r.xy / m + 0.5;

	gl_Position = oPos; //projection * model * view * p;
}

