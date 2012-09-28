#include <GL/glfw.h>
#include <Common.h>

int main(void) {
	int running = GL_TRUE;

	// init glfw
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// open gl window
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

    GLFWvidmode return_struct;
	glfwGetDesktopMode(&return_struct);

	if (!glfwOpenWindow(/*return_struct.Width*/800, 500, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// initialize the scene manager
	GameSceneManager *gsm = getGameSceneManager();
	if (!gsm->init()) {
		glfwTerminate();
		delete gsm;
		exit(EXIT_FAILURE);
	}

	// main loop
	while (running) {
		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);

		// global step advance
		globalStep();
		getGameSceneManager()->step();
	}

	// close window
	glfwTerminate();

	// exit program
	exit(EXIT_SUCCESS);
}