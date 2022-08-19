
// std
#include <string>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "application.hpp"
#include "bounding_box.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


Application::Application(GLFWwindow *window) : m_window(window) {
	
	// build the shader for the model
	shader_builder color_sb;
	color_sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//default_vert.glsl"));
	color_sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//default_frag.glsl"));
	GLuint color_shader = color_sb.build();

	// build the mesh for the model
	mesh_builder teapot_mb = load_wavefront_data(CGRA_SRCDIR + std::string("//res//assets//teapot.obj"));
	gl_mesh teapot_mesh = teapot_mb.build();

	// put together an object
	m_model.shader = color_shader;
	m_model.mesh = teapot_mesh;
	m_model.color = glm::vec3(1, 0, 0);
	m_model.modelTransform = glm::mat4(1);
}


void Application::render() {
	
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// calculate the projection and view matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);
    mat4 pitchRotation = glm::rotate(m_model.modelTransform, (float) radians(m_pitch), vec3(1.0, 0.0, 0.0));
    mat4 yawRotation = glm::rotate(m_model.modelTransform, (float) radians(m_yaw), vec3(0.0, 1.0, 0.0));
	mat4 view = translate(mat4(1), vec3(0, -5, -m_distance)) * yawRotation * pitchRotation; // TODO replace view matrix with the camera transform

	// draw options
	if (m_show_grid) cgra::drawGrid(view, proj);
	if (m_show_axis) cgra::drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Camera", nullptr);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// extra drawing parameters
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	// finish creating window
	ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(5, 115), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_Once);

    ImGui::Begin("Camera", nullptr);
    ImGui::SliderFloat("Pitch", &m_pitch, -360, 360, "%.01f");
    ImGui::SliderFloat("Yaw", &m_yaw, -360, 360, "%.01f");
    ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.01f");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(315, 5), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(350, 175), ImGuiSetCond_Once);

    ImGui::Begin("Lighting", nullptr);
    ImGui::SliderFloat3("Model Colour", value_ptr(m_model.color), 0, 1, "%.2f");
    ImGui::SliderFloat3("Light Colour", value_ptr(m_model.lightColor), 0, 1, "%.2f");
    ImGui::SliderFloat3("Light Direction", value_ptr(m_model.lightPos), -1, 1, "%.2f");
    ImGui::SliderFloat("Ambient Size", &m_model.ambientSize, 0, 1, "%.3f");
    ImGui::SliderFloat("Specular", &m_model.specular, 1, 256, "%.0f");
    ImGui::SliderFloat("Specular Size", &m_model.specularSize, 0, 100, "%.3f");
    ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (!mouseClicked) {
        xPosPrevious = xpos;
        yPosPrevious = ypos;
        return;
    }

    m_pitch += (ypos - yPosPrevious);
    m_yaw += (xpos - xPosPrevious);
    xPosPrevious = xpos;
    yPosPrevious = ypos;

    if (m_pitch > 360) {m_pitch = -360;}
    else if (m_pitch < -360) {m_pitch = 360;}

    if (m_yaw > 360) {m_yaw = -360;}
    else if (m_yaw < -360) {m_yaw = 360;}
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	if (button == 0 & action == 1) {mouseClicked = true;}
    else if (button == 0 & action == 0) {mouseClicked = false;}
}


void Application::scrollCallback(double xoffset, double yoffset) {
    m_distance += (float) xoffset;
    m_distance -= (float) yoffset;
    if (m_distance < 0) {m_distance = 0;}
    if (m_distance > 100) {m_distance = 100;}
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}