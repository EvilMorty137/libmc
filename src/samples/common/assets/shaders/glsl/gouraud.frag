#version 130

flat in vec3 color;

// TODO: Write a function for Phong lighting

void main() {
  /*
  if (gl_FrontFacing) {
    gl_FragColor = vec4(color * vec3(0.4, 1.0, 0.4), 1.0);
  } else {
    gl_FragColor = vec4(color * vec3(1.0, 0.4, 0.4), 1.0);
  }
  */
  gl_FragColor = vec4(color, 1.0);
}
