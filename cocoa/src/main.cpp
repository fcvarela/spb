#include <GL/glew.h>
#include <GL/glfw.h>
#include <Common.h>
#include <GameSceneManager.h>
#include <tinythread.h>

int main(void) {
	// init glfw
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// open gl window
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
	/*
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    */
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

	GLFWvidmode return_struct;
	glfwGetDesktopMode(&return_struct);

	__width__ = return_struct.Width;
	__height__ = return_struct.Height;

	if (!glfwOpenWindow(__width__, __height__, 8, 8, 8, 8, 32, 0, GLFW_FULLSCREEN)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// set window title
	glfwSetWindowTitle("SPB");

	// show cursor
	glfwEnable(GLFW_MOUSE_CURSOR);

	// initialize glew
	glewInit();

	// initialize a second context (procedural gen) that shares resources with the current
	__render_ctx__ = CGLGetCurrentContext();
	CGLPixelFormatObj current_pf = CGLGetPixelFormat(__render_ctx__);
	CGLError err = CGLCreateContext(current_pf, __render_ctx__, &__procedural_gen_ctx__);
	if (err != kCGLNoError) {
		std::cerr << "ERROR CREATING NEW CONTEXT" << std::endl;
		exit(EXIT_FAILURE);
	}

	// calculate opengl drawing parameters (window fov)
	calcFOV();

	// init font
	__font__ = new FTGLBitmapFont("data/fonts/Arial.ttf");
	__font__->FaceSize(12.0);

	// initialize the scene manager
	GameSceneManager *gsm = getGameSceneManager();
	if (!gsm->init()) {
		glfwTerminate();
		delete gsm;
		exit(EXIT_FAILURE);
	}

	tthread::thread gameloopThread(globalStep, 0);
	tthread::thread proceduralGenThread(proceduralGenLoop, 0);

	__running__ = GL_TRUE;
	while (glfwGetWindowParam(GLFW_OPENED)) {
		// ask the game manager to update content
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		getGameSceneManager()->draw();

		// swap buffers
		glfwSwapBuffers();
	}

	__running__ = GL_FALSE;
	std::cerr << "Terminating" << std::endl;

	// join threads
	gameloopThread.join();
	proceduralGenThread.join();

	// close window
	glfwTerminate();

	// exit program
	exit(EXIT_SUCCESS);
}
