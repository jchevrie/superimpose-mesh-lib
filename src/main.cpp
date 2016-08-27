#include <yarp/os/all.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SuperimposeHand.h"
#include "SuperimposeHandThread.h"

#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 240

using namespace yarp::os;


int main(int argc, char *argv[])
{
    ConstString log_ID = "[Main]";
    yInfo() << log_ID << "Configuring and starting module...";

    Network yarp;
    if (!yarp.checkNetwork(3.0)) {
        yError() << log_ID << "YARP seems unavailable.";
        return -1;
    }

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("superimpose-hand_config.ini");
    rf.setDefaultContext("superimpose-hand");
    rf.configure(argc, argv);

    yInfo() << log_ID << "Setting up OpenGL.";
    
    /* Initialize GLFW. */
    if (glfwInit() == GL_FALSE) {
        yError() << log_ID << "Failed to initialize GLFW.";
        return false;
    }
    
    /* Set context properties by "hinting" specific (property, value) pairs. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
#ifdef GLFW_MAC
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    /* Create a window. */
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Window", nullptr, nullptr);
    if (window == nullptr) {
        yError() << log_ID << "Failed to create GLFW window.";
        glfwTerminate();
        return false;
    }
    /* Make the OpenGL context of window the current one handled by this thread. */
    glfwMakeContextCurrent(window);
    
    /* Initialize GLEW to use the OpenGL implementation provided by the videocard manufacturer. */
    /* Note: remember that the OpenGL are only specifications, the implementation is provided by the manufacturers. */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        yError() << log_ID << "Failed to initialize GLEW.";
        return false;
    }
    
    /* Set OpenGL rendering frame for the current window. */
    /* Note that the real monitor width and height may differ w.r.t. the choosen one in hdpi monitors. */
    int hdpi_width;
    int hdpi_height;
    glfwGetFramebufferSize(window, &hdpi_width, &hdpi_height);
    glViewport(0, 0, hdpi_width, hdpi_height);
    yInfo() << log_ID << "OpenGL viewport set to "+std::to_string(hdpi_width)+"x"+std::to_string(hdpi_height)+".";
    
    /* Set GL property. */
    glEnable(GL_DEPTH_TEST);

    glfwPollEvents();

    yInfo() << log_ID << "OpenGL succesfully set up.";

    /* SuperimposeHand, derived from RFModule, must be declared by the main thread (thread_0). */
    SuperimposeHand sh;
    SuperimposeHandThread trd_sh(sh, rf, window);

    trd_sh.start();
    while (trd_sh.isRunning()) {
        glfwPollEvents();
    }
    
    glfwMakeContextCurrent(NULL);
    glfwTerminate();

    yInfo() << log_ID << "Main returning.";
    yInfo() << log_ID << "Application closed.";
    return 0;
}
