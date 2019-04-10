#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;
in vec3 Position;
uniform sampler2D texture0;

void main()
{
    
    FragColor = texture(texture0, TexCoord);
    // vec4 rawColor = vec4(0.5, 0.5, 0.5, 1.0);
    // float temp = texture(texture0, TexCoord).x;
    // float pct = 50.0 / 255.0 / ((rawColor.x + rawColor.y + rawColor.z) / 3.0);
    // FragColor = vec4(rawColor.x *(1- pct*(1.0f/0.3f)*( temp * 2 - 1)), 
    //     rawColor.y *(1- pct*(1.0f/0.3f)*( temp * 2 - 1)), 
    //     rawColor.z *(1- pct*(1.0f/0.3f)*( temp * 2 - 1)), 1.0);
}