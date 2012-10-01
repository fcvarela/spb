#include <GL/glfw.h>

#include <Common.h>
#include <GameSceneManager.h>

int main(void) {
	int running = GL_TRUE;

	// init glfw
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// open gl window
	/*
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
   	*/

    GLFWvidmode return_struct;
	glfwGetDesktopMode(&return_struct);

	__width__ = 800; //return_struct.Width
	__height__ = 500;

	if (!glfwOpenWindow(__width__, __height__, 8, 8, 8, 8, 24, 0, GLFW_WINDOW)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowTitle("SPB");

	// setup key handler
	glfwSetKeyCallback(My_Key_Callback);

	// calculate opengl drawing parameters (window fov)
	calcFOV();

	// initialize the scene manager
	GameSceneManager *gsm = getGameSceneManager();
	if (!gsm->init()) {
		glfwTerminate();
		delete gsm;
		exit(EXIT_FAILURE);
	}

	// main loop
	while (running) {
		// are we still running?
		running = glfwGetWindowParam(GLFW_OPENED);

		// process time delta
		globalStep();

		// ask the game manager to update content
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		getGameSceneManager()->step();

		// swap buffers
		glfwSwapBuffers();
	}

	// close window
	glfwTerminate();

	// exit program
	exit(EXIT_SUCCESS);
}
