
#include "graphics.h"
using namespace chai3d;
using namespace std;


static const int Tgraphics = 20;  // minimum time between graphic updates [msec] = 50 Hz

static int screenW;              // screen width (for relative window sizing/positioning)
static int screenH;              // screen height
static int windowW;              // graphics window width
static int windowH;              // graphics window height
static int windowPosX;           // graphics window X-coordinate
static int windowPosY;           // graphics window Y-coordinate
static bool fullscreen = false;  // for toggling in/out of fullscreen

static cWorld* world;             // CHAI world
static cCamera* camera;           // camera to render the world
static cDirectionalLight* light;  // light to illuminate the world
static cShapeSphere* target;      // target for trial
static cShapeSphere* cursor;      // position of cursor
static cMaterial* targetMat;	  // material for coloring target
static cMaterial* cursorMat;	  // material for coloring cursor
static cLabel* opMode;            // label to display simulation operating mode
static cLabel* input;             // label to display input mode
static cLabel* controller;        // label to display controller number
static cLabel* phantomRate;       // label to display PHANTOM rate
static cLabel* neurotouchRate;    // label to display NeuroTouch rate
static cLabel* trial;             // label to display trial count
static cLabel* message;           // label to display message to user

static shared_data* p_sharedData;  // structure for sharing data between threads (NOTE: for graphics,
                                   // need a function to access this module-level variable)


// initialize graphics before starting the main graphics rendering loop
void initGraphics(int argc, char* argv[]) {
    
    // initialize GLUT & GLEW
    glutInit(&argc, argv);
    
    // size/position graphics window
    screenW = glutGet(GLUT_SCREEN_WIDTH);
    screenH = glutGet(GLUT_SCREEN_HEIGHT);
    windowW = (int)(0.8 * screenH);
    windowH = (int)(0.5 * screenH);
    windowPosY = (screenH - windowH) / 2;
    windowPosX = windowPosY;
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(windowW, windowH);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutSetWindowTitle("NeuroTouch");
    if (fullscreen) glutFullScreen();
    
    // initialize GLEW library (must happen after window creation)
    glewInit();
    
    // set up GLUT event handlers
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(respToKey);
    glutReshapeFunc(resizeWindow);
    
    // create a black world
    world = new cWorld();
    world->m_backgroundColor.setBlack();
    
    // position/orient the camera in the world
    camera = new cCamera(world);
    world->addChild(camera);
    camera->set(cVector3d (0.0, 0.0, 0.5),   // camera position
                cVector3d (0.0, 0.0, 0.0),   // look at center
                cVector3d (0.0, 1.0, 0.0));  // "up" vector
    camera->setClippingPlanes(0.01, 10.0);
    
    // add the light source
    light = new cDirectionalLight(world);
    world->addChild(light);
    light->setEnabled(true);
    light->setDir(-1.0, 0.0, 0.0);
    
    // create spheres to represent target and cursor
    target = new cShapeSphere(TARGET_SIZE);
    cursor = new cShapeSphere(CURSOR_SIZE);
    world->addChild(target);
    world->addChild(cursor);

	// add materials to target and cursor
	targetMat = new cMaterial();
	cursorMat = new cMaterial();
	targetMat->setColorf(255,0,0,1.0);	    // red
	cursorMat->setColorf(255,255,255,0.5);  // transparent white
	target->setUseMaterial(targetMat);
	cursor->setUseMaterial(cursorMat);
    
    // create labels
    cFont* font = NEW_CFONTCALIBRI20();
    opMode = new cLabel(font);
    input = new cLabel(font);
    controller = new cLabel(font);
    phantomRate = new cLabel(font);
    neurotouchRate = new cLabel(font);
    trial = new cLabel(font);
    message = new cLabel(font);
    camera->m_frontLayer->addChild(opMode);
    camera->m_frontLayer->addChild(input);
    camera->m_frontLayer->addChild(controller);
    camera->m_frontLayer->addChild(phantomRate);
    camera->m_frontLayer->addChild(neurotouchRate);
    camera->m_frontLayer->addChild(trial);
    camera->m_frontLayer->addChild(message);
    
}

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToGraphics(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// update and re-render the graphics
void updateGraphics(void) {
    
    // update target and cursor
    double targetPos = (p_sharedData->targetSide) * TARGET_DIST;
    target->setLocalPos(targetPos, 0, 0);
    if (p_sharedData->trialSuccess) targetMat->setColorf(0,255,0,1.0);  // target turns green upon success
    else                            targetMat->setColorf(255,0,0,1.0);
    cursor->setLocalPos(p_sharedData->cursorPos, 0, 0);
    
    // update labels
    if (p_sharedData->opMode == DEMO) opMode->setString("Mode: DEMO");
    else                              opMode->setString("Mode: EXPERIMENT");
    if (p_sharedData->input == AUTO)   input->setString("Input: AUTO");
    else if (p_sharedData->input == BCI) {
        if (p_sharedData->bci == GTEC) input->setString("Input: G.MOBILAB+");
        else                           input->setString("Input: EMOTIV");
    }
    else                               input->setString("Input: PHANTOM");
    controller->setString("Controller #" + to_string(static_cast<long long>(p_sharedData->controller)));
    phantomRate->setString("PHANTOM: " + to_string(static_cast<long long>(p_sharedData->phantomFreqCounter.getFrequency())));
    neurotouchRate->setString("NeuroTouch: " + to_string(static_cast<long long>(p_sharedData->neurotouchFreqCounter.getFrequency())));
    trial->setString("Trial: " + to_string(static_cast<long long>(p_sharedData->trialNum)));
    
    opMode->setLocalPos(10, (int) (windowH - 1.0 * opMode->getHeight()), 0);
    input->setLocalPos(10, (int) (windowH - 2.0 * input->getHeight()), 0);
    controller->setLocalPos(10, (int) (windowH - 4.0 * controller->getHeight()), 0);
    phantomRate->setLocalPos(10, (int) (windowH - 6.0 * phantomRate->getHeight()), 0);
    neurotouchRate->setLocalPos(10, (int) (windowH - 7.0 * neurotouchRate->getHeight()), 0);
    trial->setLocalPos((int) (windowW - trial->getWidth() - 10), (int) (windowH - 2.0 * opMode->getHeight()), 0);
    
    // update message to user
    message->setString(p_sharedData->message);
    message->setLocalPos((int) (0.5 * (windowW - message->getWidth())), (int) (0.5 * (windowH - message->getHeight())), 0);  // center of window
    
    // toggle label visibility
    if (p_sharedData->opMode == DEMO) {  // hide target if in demo mode
        
        target->setShowEnabled(false);
        cursor->setShowEnabled(true);
        opMode->setShowEnabled(true);
        input->setShowEnabled(true);
        controller->setShowEnabled(true);
        if (p_sharedData->input == PHANTOM) phantomRate->setShowEnabled(true);
        else								phantomRate->setShowEnabled(false);
        neurotouchRate->setShowEnabled(true);
        trial->setShowEnabled(false);
        message->setShowEnabled(false);
        
    } else if (p_sharedData->experimentState == NO_HAPTICS_1 ||
               p_sharedData->experimentState == HAPTICS_1 ||
               p_sharedData->experimentState == NO_HAPTICS_2 ||
               p_sharedData->experimentState == HAPTICS_2) {
        
        target->setShowEnabled(true);
        cursor->setShowEnabled(true);
        opMode->setShowEnabled(true);
        input->setShowEnabled(true);
        controller->setShowEnabled(true);
        if (p_sharedData->input == PHANTOM) phantomRate->setShowEnabled(true);
        else								phantomRate->setShowEnabled(false);
        neurotouchRate->setShowEnabled(true);
        trial->setShowEnabled(true);
        message->setShowEnabled(false);
        
    } else {
        
        target->setShowEnabled(false);
        cursor->setShowEnabled(false);
        opMode->setShowEnabled(false);
        input->setShowEnabled(false);
        controller->setShowEnabled(false);
        phantomRate->setShowEnabled(false);
        neurotouchRate->setShowEnabled(false);
        trial->setShowEnabled(false);
        message->setShowEnabled(true);
    }
    
    // render and (smoothly, via buffer swapping) display world
    camera->renderView(windowW, windowH);
    glutSwapBuffers();
    
    // check for OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: %s\n" << gluErrorString(err);
    
}

// dictates update frequency for the graphics
void graphicsTimer(int data) {
    
    if (p_sharedData->simulationRunning) {
        glutPostRedisplay();  // flag to update CHAI world
    }
    glutTimerFunc(Tgraphics, graphicsTimer, 0);
    
}

// dictates response to resizing the graphics window
void resizeWindow(int W, int H) {
    
    windowW = W;
    windowH = H;
    
}

// dictates response to a keypress within the graphics window
void respToKey(unsigned char key, int x, int y) {
    
    switch (key) {
        
        // m/M = operation mode toggle
        case 'm':
        case 'M':
            
            if (p_sharedData->opMode == DEMO) {
                // can only use BCI for experiment
                if (p_sharedData->input == BCI) {
                    p_sharedData->opMode = EXPERIMENT;
                    initExperiment();
                } else {
                    printf("\nPlease switch to BCI before starting experiment.\n");
                }
            } else {
                // any input goes for demo
                p_sharedData->opMode = DEMO;
                closeExperiment();
            }
            break;
        
        // i/I = input toggle
        case 'i':
        case 'I':
            
            if (p_sharedData->opMode == DEMO) {
                // auto -> BCI -> PHANTOM
                if (p_sharedData->input == AUTO) {
                    p_sharedData->input = BCI;
					initBCI();
                } else if (p_sharedData->input == BCI) {
                    p_sharedData->input = PHANTOM;
                    initPhantom();
                } else {
                    p_sharedData->input = AUTO;
                    closePhantom();
                }
            } else {
                printf("\nMust use BCI for experiment.\n");
            }
            break;
        
        // c/C = controller toggle
        case 'c':
        case 'C':
            
            if (p_sharedData->controller == 4)  p_sharedData->controller = 0;
            else                               (p_sharedData->controller)++;
            break;
        
        // o/O = increase autonomous cursor frequency
        case 'o':
        case 'O':
            
            if (p_sharedData->input == AUTO) p_sharedData->autoFreq += 0.02;
            break;
        
        // l/L = decrease autonomous cursor frequency
        case 'l':
        case 'L':
            
            if (p_sharedData->input == AUTO && p_sharedData->autoFreq >= 0.02) p_sharedData->autoFreq -= 0.02;
            break;
        
        // f/F = fullscreen toggle
        case 'f':
        case 'F':
            
            if (fullscreen) {
                windowPosX = glutGet(GLUT_INIT_WINDOW_X);
                windowPosY = glutGet(GLUT_INIT_WINDOW_Y);
                windowW = glutGet(GLUT_INIT_WINDOW_WIDTH);
                windowH = glutGet(GLUT_INIT_WINDOW_HEIGHT);
                glutPositionWindow(windowPosX, windowPosY);
                glutReshapeWindow(windowW, windowH);
                fullscreen = false;
            } else {
                glutFullScreen();
                fullscreen = true;
            }
            break;
        
        // q/Q = quit, 27 = ESC key
        case 'q':
        case 'Q':
        case 27:
            
            close();
            exit(0);
            break;
            
        default:
            break;
    }
    
}

// shut down the simulation in response to quitting/ESCaping from within graphics window
void close(void) {
    
    // force feedback off for safety
    p_sharedData->controller = HAPTICS_OFF;
    
    // terminate the experiment
    if (p_sharedData->opMode == EXPERIMENT) closeExperiment();
    
    // stop the simulation
    p_sharedData->simulationRunning = false;
    
    // wait for NeuroTouch thread to terminate
    while (!p_sharedData->simulationFinished) cSleepMs(100);
    
    // close all devices
    if (p_sharedData->input == PHANTOM) closePhantom();
    closeNeuroTouch();
    
}
