#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "rs232.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <thread>
#include <string>
#include "BpmHttpManager.h"
#include "BpmRs232Manager.h"
#define plotSize 8334

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

int bdrate = 115200;
uint16_t plotMax;
float plotF[plotSize];
int beamPositionOrder[6] = { 1,4,0,3,2,5 };
int triggerDelay = 0;
int conversionFactor = 7020;
int threshold = 20;
uint16_t* plot;
char titleParameters[7][40] = { "Peak left edge" , "Peak centre" , "Peak right edge","Peak deviation", "Beam intensity","Beam FWHM","Beam skewness" };
int plotUpdateFreq=10;
std::string message;
bool dcOffset = false;
bool newBool = false;
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1960, 1080, "BPM Monitor", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    BpmCommunicationManager* comManager;
    std::cout << "Enter 1 for rs232 communication or enter 2 for http communication:" << std::endl;
    int choise;
    std::cin >> choise;
    if (choise == 1) {
        std::cout << "You chose rs232" << std::endl;
        std::cout << "Enter COM number:" << std::endl;
        int com;
        std::cin >> com;
        if(com>0)
            comManager = new BpmRs232Manager(com-1);
        std::cout << "Please reset the BPM analyzer." << std::endl;

    }
    else if (choise == 2) {
        std::cout << "you chose http" << std::endl;
        std::cout << "Enter port number:" << std::endl;
        int port;
        std::cin >> port;
        if (port > 0)
            comManager = new BpmHttpManager(port);
    }
    else {
        return 0;
    }
    // Open COM connection and start thread
    if (!comManager->setupCommunication()) {
        return 0;
    }

    std::thread communicator(&BpmCommunicationManager::requestData,comManager);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        //display beam parameters in a table
        {
            ImVec2 size{ 1800,1000 };
            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
            ImGui::Begin("BPM monitor");
            ImGui::SetWindowSize(size);
            if (ImGui::BeginTable("Parameters", 3, flags)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                ImGui::TableHeader("X crossection");
                ImGui::TableNextColumn();
                ImGui::TableHeader("Y crossection");
                uint16_t* positions = comManager->getBeamLocation();
                //beam positions
                for (int row = 0; row < 3; row++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(titleParameters[row]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%f mm",(float) (positions[beamPositionOrder[2 * row]])/69.45);
                    ImGui::TableNextColumn();
                    ImGui::Text("%f mm", (float)(positions[beamPositionOrder[2 * row + 1]]- 4167) / 69.45);
                }
                //deviation
                float* stdeviation = comManager->getDeviation();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(titleParameters[3]);
                ImGui::TableNextColumn();
                ImGui::Text("%f mm", stdeviation[0]/ 69.45);
                ImGui::TableNextColumn();
                ImGui::Text("%f mm", stdeviation[1]/ 69.45);
                //Intensity
                uint32_t* intensity = comManager->getIntensity();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(titleParameters[4]);
                ImGui::TableNextColumn();
                ImGui::Text("%f nA",(float)(*intensity)/conversionFactor);
                //FWHM
                uint16_t* FWHM = comManager->getFwhm();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(titleParameters[5]);
                ImGui::TableNextColumn();
                ImGui::Text("%f mm",(float) (FWHM[0])/69.45);
                ImGui::TableNextColumn();
                ImGui::Text("%f mm", (float)(FWHM[1]) / 69.45);
                //skewness
                float* skewness = comManager->getSkewness();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(titleParameters[6]);
                ImGui::TableNextColumn();
                ImGui::Text("%f", skewness[0]);
                ImGui::TableNextColumn();
                ImGui::Text("%f", skewness[1]);
                ImGui::EndTable();
            }

            // collector plot display
            {
                if (comManager->newPlotDataAvailable()) {
                    plotMax = 0;
                    plot = comManager->getPlot();
                    for (int i = 0;i < plotSize; i++) {
                        plotF[i] = plot[i];
                        if (plotF[i] > plotMax) {
                            plotMax = plotF[i];
                        }
                    }
                    comManager->setPlotDataAvailable(false);
                }
                ImGui::PlotLines("Lines", plotF, 8334, 0, 0, 0, plotMax + 100, ImVec2(1800, 600));
            }

            //Plot update frequency slider 
            ImGui::Text("Seconds between plot update");
            ImGui::SliderInt("plot update freq", &plotUpdateFreq, 1, 60);
            comManager->setPlotUpdateFrequency(plotUpdateFreq);

            // trigger delay update field
            ImGui::InputInt("Trigger Delay", &triggerDelay);
            ImGui::SameLine();
            if (ImGui::Button("Update Delay")) { // only allow delay's within a certain range
                if (triggerDelay > 60) {
                    triggerDelay = 60;
                }
                if (triggerDelay < 0) {
                    triggerDelay = 0;
                }
                comManager->updateTriggerDelay(triggerDelay);
            }
            ImGui::InputInt("Peak threshold", &threshold);
            ImGui::SameLine();
            if (ImGui::Button("Update threshold")) { 
                if (threshold < 0) {
                    threshold = 0;
                }
                if (threshold > 255) {
                    threshold = 255;
                }
                comManager->updatePeakThreshold(threshold);
            }
            ImGui::InputInt("Update conversion factor", &conversionFactor);
           
            ImGui::Checkbox("DC-correction", &dcOffset);
            if (dcOffset != newBool) {
                comManager->updateDcCorrection(dcOffset);
                newBool = dcOffset;
            }

            // extra text
            message = comManager->getEcchoMessage();
            ImGui::Text(message.c_str());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    // Cleanup
    comManager->setRunning(false);
    communicator.join();
    comManager->cleanUpCommunication();
    delete comManager;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
