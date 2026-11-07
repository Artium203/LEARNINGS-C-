// main.cpp
// Advanced OpenGL demo: colored room + 5 spheres
// Features: point-light shadow cubemaps, dynamic reflection cubemap,
//          camera controls. Uses GLFW + GLAD + GLM.
// Compile linking to GLFW, GL, dl (linux): e.g.
// g++ -std=c++17 -O2 main.cpp glad.c -lglfw -ldl -lGL -pthread -o advanced_room
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <map>

// ---------- small helper: shader compile ----------
static GLuint compileShader(GLenum t, const char* src) {
    GLuint s = glCreateShader(t);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        char buf[4096]; glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        std::cerr<<"Shader compile error: "<<buf<<"\n";
    }
    return s;
}
static GLuint linkProgram(const std::vector<GLuint>& shaders) {
    GLuint p = glCreateProgram();
    for (auto s: shaders) glAttachShader(p, s);
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if(!ok) {
        char buf[4096]; glGetProgramInfoLog(p, sizeof(buf), nullptr, buf);
        std::cerr<<"Program link error: "<<buf<<"\n";
    }
    for (auto s: shaders) { glDetachShader(p, s); glDeleteShader(s); }
    return p;
}

// ---------- geometry helpers (cube, sphere) ----------
struct Mesh {
    GLuint vao=0, vbo=0, ebo=0;
    GLsizei indexCount=0;
};
Mesh makeCube() {
    // cube vertices (position + normal) for inward-facing cube (flip normals)
    struct V { float px,py,pz; float nx,ny,nz; float u,v; };
    std::vector<V> verts;
    std::vector<unsigned int> idx;

    // build cube centered at origin, size 2
    float s=1.0f;
    glm::vec3 positions[8] = {
        {-s,-s,-s},{ s,-s,-s},{ s, s,-s},{-s, s,-s},
        {-s,-s, s},{ s,-s, s},{ s, s, s},{-s, s, s}
    };
    // faces - we want inward normals, so vertices order reversed compared to usual outward
    struct Face { int a,b,c,d; glm::vec3 normal; };
    std::vector<Face> faces = {
        {4,5,6,7, glm::vec3(0,0,1)},  // +Z back
        {1,0,3,2, glm::vec3(0,0,-1)}, // -Z front
        {0,4,7,3, glm::vec3(-1,0,0)}, // -X left
        {5,1,2,6, glm::vec3(1,0,0)},  // +X right
        {3,7,6,2, glm::vec3(0,1,0)},  // +Y top
        {0,1,5,4, glm::vec3(0,-1,0)}  // -Y bottom
    };
    for (auto &f: faces) {
        int base = verts.size();
        glm::vec3 n = -f.normal; // invert because we want normals pointing inward in world-space (lighting may assume inward)
        // quad as two triangles
        int ids[4] = {f.a,f.b,f.c,f.d};
        for (int i=0;i<4;i++){
            V v;
            v.px = positions[ids[i]].x;
            v.py = positions[ids[i]].y;
            v.pz = positions[ids[i]].z;
            v.nx = n.x; v.ny = n.y; v.nz = n.z;
            v.u = (i&1)?1.0f:0.0f; v.v = (i>1)?1.0f:0.0f;
            verts.push_back(v);
        }
        idx.insert(idx.end(), {base, base+1, base+2, base, base+2, base+3});
    }

    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glBindVertexArray(m.vao);
    glGenBuffers(1, &m.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(V), verts.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &m.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    // position
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,px));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,nx));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,u));
    glBindVertexArray(0);
    m.indexCount = idx.size();
    return m;
}

Mesh makeSphere(int lat=64, int lon=64) {
    struct V { float px,py,pz; float nx,ny,nz; float u,v; };
    std::vector<V> verts;
    std::vector<unsigned int> idx;
    for (int y=0;y<=lat;y++){
        float v = (float)y/lat;
        float theta = v * M_PI;
        for (int x=0;x<=lon;x++){
            float u = (float)x/lon;
            float phi = u * 2.0f*M_PI;
            float px = sin(theta) * cos(phi);
            float py = cos(theta);
            float pz = sin(theta) * sin(phi);
            V vv;
            vv.px = px; vv.py = py; vv.pz = pz;
            vv.nx = px; vv.ny = py; vv.nz = pz;
            vv.u = u; vv.v = v;
            verts.push_back(vv);
        }
    }
    for (int y=0;y<lat;y++){
        for (int x=0;x<lon;x++){
            int a = y*(lon+1)+x;
            int b = a + (lon+1);
            idx.push_back(a); idx.push_back(b); idx.push_back(a+1);
            idx.push_back(b); idx.push_back(b+1); idx.push_back(a+1);
        }
    }
    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glBindVertexArray(m.vao);
    glGenBuffers(1, &m.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(V), verts.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &m.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,px));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,nx));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,u));
    glBindVertexArray(0);
    m.indexCount = idx.size();
    return m;
}

// ---------- shaders (vertex + fragment for forward pass) ----------

// vertex shader for forward pass
const char* vs_forward = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
void main(){
    mat4 mv = uView * uModel;
    vWorldPos = vec3(uModel * vec4(aPos,1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vUV = aUV;
    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);
}
)GLSL";

// fragment shader for forward pass
// - supports sampling shadow depth cubemaps for point lights
// - supports reflection sampling from cubemap for mirror + glossy spheres
const char* fs_forward = R"GLSL(
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3 viewPos;

// Light data (we support up to 2 point lights)
uniform vec3 lightPos[2];
uniform vec3 lightColor[2];
uniform samplerCube shadowCube[2]; // depth cubemaps for point light shadows
uniform float lightFarPlane; // far plane used when created depth cubemap

// material
uniform vec3 baseColor;
uniform float metallic;
uniform float roughness;
uniform float emissiveStrength;

// reflection cube
uniform samplerCube envMap;
uniform int useEnvMap; // 0 no, 1 yes
uniform float reflectivity; // 0..1 for mixing env map

// Utility: sample point-light shadow cube (depth cubemap)
float samplePointShadow(int lightIndex, vec3 fragPos) {
    vec3 L = fragPos - lightPos[lightIndex];
    float currentDepth = length(L);
    // TODO: PCF softening - do a few samples
    float bias = 0.05 * lightFarPlane; // simple bias scaled
    float shadow = 0.0;
    // sample around: 1 center + 6 offsets
    float samples = 6.0;
    float diskRadius = (1.0 + (currentDepth / lightFarPlane)) / 25.0;
    // sample center
    float closest = texture(shadowCube[lightIndex], L).r;
    closest *= lightFarPlane;
    if (currentDepth - bias > closest) shadow += 1.0;
    // few extra samples
    for (int i=0;i<5;i++){
        vec3 offset = vec3(0.0);
        if(i==0) offset = vec3( diskRadius, 0, 0);
        if(i==1) offset = vec3(-diskRadius, 0, 0);
        if(i==2) offset = vec3(0, diskRadius, 0);
        if(i==3) offset = vec3(0,-diskRadius, 0);
        if(i==4) offset = vec3(0,0,diskRadius);
        float c = texture(shadowCube[lightIndex], L + offset).r * lightFarPlane;
        if (currentDepth - bias > c) shadow += 1.0;
    }
    shadow /= (samples + 1.0);
    return clamp(shadow, 0.0, 1.0);
}

void main(){
    vec3 N = normalize(vNormal);
    vec3 V = normalize(viewPos - vWorldPos);

    // emissive
    vec3 emissive = baseColor * emissiveStrength;

    // PBR-ish diffuse (Lambert) + Blinn-Phong specular approximation
    vec3 color = vec3(0.0);
    for (int i=0;i<2;i++){
        vec3 Lvec = normalize(lightPos[i] - vWorldPos);
        float NdotL = max(dot(N,Lvec), 0.0);
        // diffuse
        vec3 diff = baseColor * NdotL * lightColor[i];
        // specular (approx)
        vec3 H = normalize(Lvec + V);
        float NdotH = max(dot(N,H), 0.0);
        float spec = pow(NdotH, 64.0 * (1.0 - roughness));
        vec3 specCol = mix(vec3(0.04), baseColor, metallic) * spec * lightColor[i];
        // shadow:
        float shadow = samplePointShadow(i, vWorldPos);
        color += (1.0 - shadow) * (diff + specCol);
    }

    // environment reflection
    vec3 refl = vec3(0.0);
    if (useEnvMap == 1){
        vec3 R = reflect(-V, N);
        refl = texture(envMap, R).rgb;
    }

    vec3 final = emissive + color + refl * reflectivity;
    // gamma
    final = pow(final, vec3(1.0/2.2));
    FragColor = vec4(final, 1.0);
}
)GLSL";

// simple shader for rendering depth into cubemap (point-light)
const char* vs_depth_cube = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
void main(){
    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);
}
)GLSL";

const char* fs_depth_cube = R"GLSL(
#version 330 core
// empty: we only need depth
void main(){}
)GLSL";

// ---------- GL utility functions ----------
GLuint makeProgramFromSrc(const char* vs, const char* fs) {
    GLuint s1 = compileShader(GL_VERTEX_SHADER, vs);
    GLuint s2 = compileShader(GL_FRAGMENT_SHADER, fs);
    return linkProgram({s1,s2});
}
GLuint makeProgramFromSrcGeom(const char* vs, const char* gs, const char* fs) {
    GLuint s1 = compileShader(GL_VERTEX_SHADER, vs);
    GLuint sg = 0;
    if (gs) sg = compileShader(GL_GEOMETRY_SHADER, gs);
    GLuint s2 = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, s1);
    if (gs) glAttachShader(prog, sg);
    glAttachShader(prog, s2);
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[4096]; glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        std::cerr<<"Program link error: "<<buf<<"\n";
    }
    glDeleteShader(s1); if (gs) glDeleteShader(sg); glDeleteShader(s2);
    return prog;
}

// ---------- main app state ----------
int SCR_W = 1280, SCR_H = 800;

struct Camera {
    glm::vec3 pos = glm::vec3(0.0f, 1.5f, 8.0f);
    float yaw = -90.f, pitch = -10.f;
    float fov = 60.0f;
} camera;

bool firstMouse = true;
double lastX, lastY;
bool leftDown=false;

// simple input handlers
void mouse_button_cb(GLFWwindow* win, int button, int action, int mods){
    if(button==GLFW_MOUSE_BUTTON_LEFT) {
        leftDown = (action==GLFW_PRESS);
        if (leftDown) { glfwGetCursorPos(win, &lastX, &lastY); firstMouse=false; }
    }
}
void cursor_cb(GLFWwindow* win, double xpos, double ypos){
    if (!leftDown) return;
    float sensitivity = 0.2f;
    float dx = (xpos - lastX) * sensitivity;
    float dy = (lastY - ypos) * sensitivity;
    lastX = xpos; lastY = ypos;
    camera.yaw += dx;
    camera.pitch += dy;
    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
}

void scroll_cb(GLFWwindow* win, double xoff, double yoff){
    camera.fov -= (float)yoff;
    if (camera.fov < 20.0f) camera.fov = 20.0f;
    if (camera.fov > 80.0f) camera.fov = 80.0f;
}

float deltaTime = 0.0f, lastFrame = 0.0f;
void processInput(GLFWwindow* win){
    float speed = 5.0f * deltaTime;
    glm::vec3 front;
    front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    front.y = sin(glm::radians(camera.pitch));
    front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    front = glm::normalize(front);
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) camera.pos += front * speed;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) camera.pos -= front * speed;
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) camera.pos -= right * speed;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) camera.pos += right * speed;
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) camera.pos += glm::vec3(0,1,0) * speed;
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) camera.pos -= glm::vec3(0,1,0) * speed;
}

// ---------- create depth cubemap FBO for a point light ----------
struct PointShadow {
    GLuint depthCubemap = 0;
    GLuint fbo = 0;
    int size;
    float farPlane;
};
PointShadow makePointShadow(int size, float farPlane) {
    PointShadow p;
    p.size = size;
    p.farPlane = farPlane;
    glGenFramebuffers(1, &p.fbo);
    glGenTextures(1, &p.depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, p.depthCubemap);
    for (int i=0;i<6;i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, p.fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, p.depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr<<"Depth cubemap FBO incomplete\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return p;
}

// ---------- create cubemap + FBO to render scene from point (for reflections) ----------
struct DynamicCubemap {
    GLuint cubemap = 0;
    GLuint fbo = 0, rbo = 0;
    int size;
};
DynamicCubemap makeDynamicCubemap(int size) {
    DynamicCubemap d;
    d.size = size;
    glGenTextures(1, &d.cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, d.cubemap);
    for (int i=0;i<6;i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &d.fbo);
    glGenRenderbuffers(1, &d.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, d.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return d;
}

// ---------- Render helpers (render scene from camera/projection with model matrices) ----------
Mesh roomMesh;
Mesh sphereMesh;
GLuint programForward=0;
GLuint programDepth=0;

void drawRoom(const glm::mat4& view, const glm::mat4& proj, GLuint prog) {
    // draw the big cube scaled to room size; we pass color per-face via uniform depending on draw call
    glUseProgram(prog);
    // model matrix: scale large
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(8.0f)); // room half-size 8
    glUniformMatrix4fv(glGetUniformLocation(prog,"uModel"),1,GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uView"),1,GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uProj"),1,GL_FALSE, glm::value_ptr(proj));
    // Draw the 6 faces individually with their colors by drawing subsets of the cube mesh (each face 6 indices)
    glBindVertexArray(roomMesh.vao);
    // face colors list (matching order used in makeCube)
    std::vector<glm::vec3> faceCols = {
        {1,0,0}, // red
        {0,1,0}, // green
        {0,0,1}, // blue
        {1,1,0}, // yellow
        {1.0f,0.5f,0.75f}, // pink
        {1,1,1}  // white (floor)
    };
    for (int i=0;i<6;i++){
        glUniform3fv(glGetUniformLocation(prog,"baseColor"), 1, glm::value_ptr(faceCols[i]));
        // set material props for room surfaces
        glUniform1f(glGetUniformLocation(prog,"metallic"), 0.0f);
        glUniform1f(glGetUniformLocation(prog,"roughness"), 1.0f);
        glUniform1f(glGetUniformLocation(prog,"emissiveStrength"), 0.0f);
        // draw face i: each face has 6 indices: offset = i*6
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)*6*i));
    }
    glBindVertexArray(0);
}

void drawSphereAt(const glm::vec3& pos, float radius, const glm::mat4& view, const glm::mat4& proj, GLuint prog,
                  glm::vec3 baseColor, float metallic, float roughness, float emissive, bool useEnv, float reflectivity, bool wire=false)
{
    glUseProgram(prog);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uModel"),1,GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uView"),1,GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uProj"),1,GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(prog,"baseColor"),1,glm::value_ptr(baseColor));
    glUniform1f(glGetUniformLocation(prog,"metallic"), metallic);
    glUniform1f(glGetUniformLocation(prog,"roughness"), roughness);
    glUniform1f(glGetUniformLocation(prog,"emissiveStrength"), emissive);
    glUniform1i(glGetUniformLocation(prog,"useEnvMap"), useEnv?1:0);
    glUniform1f(glGetUniformLocation(prog,"reflectivity"), reflectivity);

    if (wire){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glBindVertexArray(sphereMesh.vao);
    glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// ---------- Main ----------
int main(){
    if (!glfwInit()) { std::cerr<<"Failed to init GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(SCR_W, SCR_H, "Advanced Room - shadows + reflections", nullptr, nullptr);
    if (!win) { std::cerr<<"Failed to create window\n"; return -1; }
    glfwMakeContextCurrent(win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr<<"Failed to load GLAD\n"; return -1; }

    glfwSetMouseButtonCallback(win, mouse_button_cb);
    glfwSetCursorPosCallback(win, cursor_cb);
    glfwSetScrollCallback(win, scroll_cb);

    glEnable(GL_DEPTH_TEST);
    // Create meshes
    roomMesh = makeCube();
    sphereMesh = makeSphere(48,48);

    programForward = makeProgramFromSrc(vs_forward, fs_forward);
    programDepth = makeProgramFromSrc(vs_depth_cube, fs_depth_cube);

    // Lights positions (match your description: two lights)
    glm::vec3 lightPos0 = glm::vec3(5.0f, 1.0f, -1.0f);
    glm::vec3 lightPos1 = glm::vec3( 2.0f, 1.0f,  1.0f);
    glm::vec3 lightColor0 = glm::vec3(1.0f, 0.15f, 0.15f); // reddish
    glm::vec3 lightColor1 = glm::vec3(0.15f, 1.0f, 0.15f); // greenish

    // Create point light shadow cubemaps
    int shadowSize = 1024;
    float lightFar = 25.0f;
    PointShadow ps0 = makePointShadow(shadowSize, lightFar);
    PointShadow ps1 = makePointShadow(shadowSize, lightFar);

    // dynamic cubemap for mirror + glossy
    DynamicCubemap dynCub = makeDynamicCubemap(512);

    // projection for cubemap faces (90 degrees)
    float aspect = 1.0f;
    glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 50.0f);
    // cubemap face views for +X,-X,+Y,-Y,+Z,-Z
    std::vector<glm::mat4> captureViews;
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(1,0,0),  glm::vec3(0,-1,0)));
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)));
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,1,0),  glm::vec3(0,0,1)));
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,-1,0), glm::vec3(0,0,-1)));
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,1),  glm::vec3(0,-1,0)));
    captureViews.push_back(glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,-1,0)));

    // For depth cubemap rendering we need per-face view/projection from light
    auto makeLightMatrices = [&](glm::vec3 lightPos, float farPlane){
        glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
        std::vector<glm::mat4> views;
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 1, 0, 0), glm::vec3(0,-1,0)));
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0,-1,0)));
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0,0,1)));
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0,-1, 0), glm::vec3(0,0,-1)));
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0,-1,0)));
        views.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0, 0,-1), glm::vec3(0,-1,0)));
        return std::pair<glm::mat4,std::vector<glm::mat4>>(proj, views);
    };
    auto light0Mats = makeLightMatrices(lightPos0, lightFar);
    auto light1Mats = makeLightMatrices(lightPos1, lightFar);

    // Setup GL state for texunits used in shader
    glUseProgram(programForward);
    glUniform1i(glGetUniformLocation(programForward,"envMap"), 5); // we'll bind env map to unit 5
    // shadow cubemap units
    glUniform1i(glGetUniformLocation(programForward,"shadowCube[0]"), 2);
    glUniform1i(glGetUniformLocation(programForward,"shadowCube[1]"), 3);

    // Main loop
    while(!glfwWindowShouldClose(win)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
        processInput(win);

        // calculate camera matrices
        glm::vec3 camFront;
        camFront.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        camFront.y = sin(glm::radians(camera.pitch));
        camFront.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        camFront = glm::normalize(camFront);
        glm::mat4 view = glm::lookAt(camera.pos, camera.pos + camFront, glm::vec3(0,1,0));
        glm::mat4 proj = glm::perspective(glm::radians(camera.fov), (float)SCR_W/(float)SCR_H, 0.1f, 100.0f);

        // ---------- 1) render depth cubemaps for both lights ----------
        // Render scene to each light's depth cubemap (depth-only)
        auto renderDepthCubemap = [&](PointShadow& ps, glm::vec3 lightPos, const std::pair<glm::mat4,std::vector<glm::mat4>>& mats){
            glViewport(0,0,ps.size,ps.size);
            glBindFramebuffer(GL_FRAMEBUFFER, ps.fbo);
            glUseProgram(programDepth);
            // set projection uniform (same for all faces)
            // We'll upload uProj and uView per face
            for (int face=0; face<6; ++face){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, ps.depthCubemap, 0);
                glClear(GL_DEPTH_BUFFER_BIT);
                glm::mat4 viewLight = mats.second[face];
                glm::mat4 projLight = mats.first;
                // draw room + objects (simple models)
                // room
                glUniformMatrix4fv(glGetUniformLocation(programDepth,"uProj"),1,GL_FALSE, glm::value_ptr(projLight));
                glUniformMatrix4fv(glGetUniformLocation(programDepth,"uView"),1,GL_FALSE, glm::value_ptr(viewLight));
                // model room
                glm::mat4 modelRoom = glm::scale(glm::mat4(1.0f), glm::vec3(8.0f));
                glUniformMatrix4fv(glGetUniformLocation(programDepth,"uModel"),1,GL_FALSE, glm::value_ptr(modelRoom));
                glBindVertexArray(roomMesh.vao);
                glDrawElements(GL_TRIANGLES, roomMesh.indexCount, GL_UNSIGNED_INT, 0);
                // spheres / other objects: draw sphere meshes at positions same as main scene
                // Sphere positions:
                std::vector<std::pair<glm::vec3,float>> spheres = {
                    {glm::vec3(-2.0f,1.0f,-1.0f), 0.35f}, // red (light)
                    {glm::vec3( 2.0f,1.0f, 1.0f), 0.35f}, // green (light)
                    {glm::vec3( 0.0f,0.8f, 0.0f), 0.7f},  // mirror
                    {glm::vec3(-1.0f,0.6f,-1.5f), 0.5f}, // reflective
                    {glm::vec3( 2.0f,0.6f,-0.5f), 0.5f}   // wire white
                };
                for (auto &sp: spheres){
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), sp.first) * glm::scale(glm::mat4(1.0f), glm::vec3(sp.second));
                    glUniformMatrix4fv(glGetUniformLocation(programDepth,"uModel"),1,GL_FALSE, glm::value_ptr(model));
                    glBindVertexArray(sphereMesh.vao);
                    glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, 0);
                }
                glBindVertexArray(0);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        };

        renderDepthCubemap(ps0, lightPos0, light0Mats);
        renderDepthCubemap(ps1, lightPos1, light1Mats);

        // ---------- 2) dynamic cubemap render for mirror/reflective spheres ----------
        // We will render the scene 6 times from the mirror sphere center into dynCub.cubemap
        // Mirror sphere center (0,0.8,0)
        glm::vec3 mirrorCenter = glm::vec3(0.0f, 0.8f, 0.0f);
        glViewport(0,0,dynCub.size,dynCub.size);
        glBindFramebuffer(GL_FRAMEBUFFER, dynCub.fbo);
        for (int face=0; face<6; ++face){
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, dynCub.cubemap, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dynCub.rbo);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClearColor(0.08f,0.08f,0.08f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // set view/proj offset by mirror center
            glm::mat4 viewC = captureViews[face];
            // shift by mirror center: view = lookAt(mirrorCenter + dir, mirrorCenter, up)
            glm::mat4 viewFromCenter = glm::translate(glm::mat4(1.0f), -mirrorCenter) * viewC;
            // but simpler: compute camera matrices manually:
            glm::mat4 viewM = glm::lookAt(mirrorCenter, mirrorCenter + glm::vec3(
                captureViews[face][2][0], captureViews[face][2][1], captureViews[face][2][2]
            ), glm::vec3(captureViews[face][1][0], captureViews[face][1][1], captureViews[face][1][2]));
            // For reliability we reconstruct lookAt vectors explicitly:
            // However above is a simplification; we can instead compute direct vectors:
            // We'll reuse captureViews but offset by translation:
            glm::mat4 viewOffset = glm::translate(glm::mat4(1.0f), -mirrorCenter) * captureViews[face];
            // we will just use lookAt with mirror center:
            glm::vec3 centerDir;
            if(face==0) centerDir = glm::vec3(1,0,0);
            if(face==1) centerDir = glm::vec3(-1,0,0);
            if(face==2) centerDir = glm::vec3(0,1,0);
            if(face==3) centerDir = glm::vec3(0,-1,0);
            if(face==4) centerDir = glm::vec3(0,0,1);
            if(face==5) centerDir = glm::vec3(0,0,-1);
            glm::vec3 upv = (face==2)?glm::vec3(0,0,1):glm::vec3(0,-1,0);
            glm::mat4 viewFace = glm::lookAt(mirrorCenter, mirrorCenter + centerDir, upv);
            // render scene normally into FBO but using forward shader
            glUseProgram(programForward);
            // set camera uniforms for forward shader to use during render to cubemap
            glUniform3fv(glGetUniformLocation(programForward,"viewPos"),1, glm::value_ptr(mirrorCenter));
            // bind shadow cubemaps to units 2 and 3
            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, ps0.depthCubemap);
            glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_CUBE_MAP, ps1.depthCubemap);
            // env map not needed for this pass (we're generating it)
            glUniform1f(glGetUniformLocation(programForward,"lightFarPlane"), lightFar);
            // set light positions/cols
            glUniform3fv(glGetUniformLocation(programForward,"lightPos[0]"),1, glm::value_ptr(lightPos0));
            glUniform3fv(glGetUniformLocation(programForward,"lightPos[1]"),1, glm::value_ptr(lightPos1));
            glUniform3fv(glGetUniformLocation(programForward,"lightColor[0]"),1, glm::value_ptr(lightColor0));
            glUniform3fv(glGetUniformLocation(programForward,"lightColor[1]"),1, glm::value_ptr(lightColor1));
            // set view/proj
            glUniformMatrix4fv(glGetUniformLocation(programForward,"uView"),1,GL_FALSE, glm::value_ptr(viewFace));
            glUniformMatrix4fv(glGetUniformLocation(programForward,"uProj"),1,GL_FALSE, glm::value_ptr(captureProj));
            // Draw scene (room + spheres) but omit mirror sphere itself (so it reflects the world, not itself)
            // Room:
            glm::mat4 modelRoom = glm::scale(glm::mat4(1.0f), glm::vec3(8.0f));
            glUniformMatrix4fv(glGetUniformLocation(programForward,"uModel"),1,GL_FALSE, glm::value_ptr(modelRoom));
            // baseColor for room faces: we'll just pick neutral grey for cubemap generation
            glUniform3f(glGetUniformLocation(programForward,"baseColor"), 0.9f, 0.9f, 0.9f);
            glUniform1f(glGetUniformLocation(programForward,"metallic"), 0.0f);
            glUniform1f(glGetUniformLocation(programForward,"roughness"), 1.0f);
            glUniform1f(glGetUniformLocation(programForward,"emissiveStrength"), 0.0f);
            glBindVertexArray(roomMesh.vao);
            glDrawElements(GL_TRIANGLES, roomMesh.indexCount, GL_UNSIGNED_INT, 0);
            // draw other spheres except mirror
            // sphere 1 red emitter
            drawSphereAt(lightPos0, 0.35f, viewFace, captureProj, programForward, glm::vec3(1,0.15,0.15), 0.0f, 0.5f, 1.0f, 0, 0.0f);
            drawSphereAt(lightPos1, 0.35f, viewFace, captureProj, programForward, glm::vec3(0.15,1,0.15), 0.0f, 0.5f, 1.0f, 0, 0.0f);
            // reflective sphere
            drawSphereAt(glm::vec3(-1.0f,0.6f,-1.5f), 0.5f, viewFace, captureProj, programForward, glm::vec3(0.2f), 0.2f, 0.2f, 0.0f, 0, 0.5f);
            // wire sphere
            drawSphereAt(glm::vec3(2.0f,0.6f,-0.5f), 0.5f, viewFace, captureProj, programForward, glm::vec3(1.0f), 0.0f, 1.0f, 0.0f, 0, 0.0f, true);
            glBindVertexArray(0);
        }
        // generate mipmaps for cubemap so glossy sampling works better
        glBindTexture(GL_TEXTURE_CUBE_MAP, dynCub.cubemap);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ---------- 3) main forward render to screen ----------
        glViewport(0,0,SCR_W,SCR_H);
        glClearColor(0.08f,0.08f,0.08f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(programForward);
        glUniformMatrix4fv(glGetUniformLocation(programForward,"uView"),1,GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(programForward,"uProj"),1,GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(glGetUniformLocation(programForward,"viewPos"),1, glm::value_ptr(camera.pos));
        // bind shadow cubemaps
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, ps0.depthCubemap);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_CUBE_MAP, ps1.depthCubemap);
        glUniform1f(glGetUniformLocation(programForward,"lightFarPlane"), lightFar);
        glUniform3fv(glGetUniformLocation(programForward,"lightPos[0]"),1, glm::value_ptr(lightPos0));
        glUniform3fv(glGetUniformLocation(programForward,"lightPos[1]"),1, glm::value_ptr(lightPos1));
        glUniform3fv(glGetUniformLocation(programForward,"lightColor[0]"),1, glm::value_ptr(lightColor0));
        glUniform3fv(glGetUniformLocation(programForward,"lightColor[1]"),1, glm::value_ptr(lightColor1));

        // bind dynamic cubemap as env map to unit 5
        glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_CUBE_MAP, dynCub.cubemap);

        // draw room (colored faces)
        drawRoom(view, proj, programForward);

        // draw 5 spheres:
        // 1: red emitter (also a light)
        drawSphereAt(lightPos0, 0.35f, view, proj, programForward, glm::vec3(1.0f,0.15f,0.15f), 0.0f, 0.5f, 1.0f, 0, 0.0f);
        // 2: green emitter
        drawSphereAt(lightPos1, 0.35f, view, proj, programForward, glm::vec3(0.15f,1.0f,0.15f), 0.0f, 0.5f, 1.0f, 0, 0.0f);
        // 3: mirror sphere - perfect reflection using env map & reflectivity=1.0
        drawSphereAt(glm::vec3(0.0f,-5.8f,0.0f), 0.9f, view, proj, programForward, glm::vec3(0.8f), 0.0f, 0.05f, 0.0f, 1, 1.0f);
        // 4: reflective glossy sphere
        drawSphereAt(glm::vec3(-1.0f,-7.6f,-1.5f), 1.5f, view, proj, programForward, glm::vec3(0.6f), 0.0f, 0.2f, 0.0f, 1, 0.6f);
        // 5: white wire sphere
        drawSphereAt(glm::vec3(2.0f,-0.6f,-0.5f), 0.5f, view, proj, programForward, glm::vec3(1.0f), 0.0f, 1.0f, 0.0f, 0, 0.0f, true);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
