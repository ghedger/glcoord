#version 330 core
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

precision mediump float;

attribute vec3 aPos;
attribute vec3 aNormal;

varying vec3 vNormal;
varying vec3 vEye;

out vec3 FragPos;
uniform mat4 projection2;
uniform mat4 model2;
uniform mat4 view2;
uniform mat4 mNormal;
uniform vec3 uEye;

void main() {

  FragPos = vec3(model2 * vec4(aPos, 1.0));

  vEye = uEye;
  //vNormal = normalize((mNormal * vec4(aNormal, 0.0)).xyz);
  //vNormal = normalize((mNormal * vec4(aNormal, 0.0)).xyz);
	//vNormal = mat3(transpose(inverse(model2))) * aNormal;
	vNormal = normalize(mat3(transpose(inverse(model2)) * mNormal ) * aNormal);

/*
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
*/

  //gl_Position = projection2 * model2 * view2 * vec4(aPos, 1.0);
  gl_Position = projection2 * view2 * vec4(FragPos, 1.0);
}




/*
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
*/





/*
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

varying vec2 vN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vec4 p = vec4( aPos, 1.0 );

  vec3 e = normalize( vec3( model * p ) );
  //vec3 n = normalize( normalMatrix * normal );
  vec3 n = normalize( view * normal );

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
