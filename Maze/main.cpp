/*
 * MAZE Game Framework
 * Written by Dr. Dhanyu Amarasinghe Spring 2018
 */

#include <string.h>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <Timer.h>
#include <player.h>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <Enemies.h>

#include <wall.h>
#include <math.h>

/* GLUT callback Handlers */

using namespace std;

Maze *M = new Maze(20);                         // Set Maze grid size
Player *P = new Player();                       // create player

wall W[10000];
int wallArray[50][50];                                    // wall with number of bricks
Enemies E[100];                                  // create number of enemies
Timer *T0 = new Timer();
 string line, obj;
    int x, y;
    int numWall = 0;
    int numEnemy = 0;                       // animation timer

float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping


void display(void);                             // Main Display : this runs in a loop

void resize(int width, int height)              // resizing case on the window
{
    wWidth = width;
    wHeight = height;

    if(width<=height)
        glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
        glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);
}

void init()
{
    glEnable(GL_COLOR_MATERIAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);

    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     M->loadBackgroundImage("images/bak.jpg");           // Load maze background image

    fstream myfile ("maze.txt");
if (myfile.is_open()){
    cout << "Able to open file" << endl;
    while ( getline (myfile,line) ){
        stringstream ss(line);
        ss >> obj >> x >> y;
        //cout << obj << endl;
        //cout << x << endl;
        //cout << y << endl;
        if(obj == "enemy"){
            E[numEnemy].initEnm(M->getGridSize(),4,"images/e.png"); //Load enemy image
            E[numEnemy].placeEnemy(float(x%(M->getGridSize())),float(y%(M->getGridSize())));
            numEnemy++;
            }
       else if(obj == "wall"){
            W[numWall].wallInit(M->getGridSize(),"images/wall.png");// Load walls
            W[numWall].placeWall(x,y);
            wallArray[x][y] = 1;
            numWall++;                              // place each brick
            }
        else if(obj =="arrow"){
            M->loadSetOfArrowsImage("images/arrow.png");      // load set of arrows image
            M->placeStArrws(x,y);
        }
        else if(obj =="player"){
            P->initPlayer(M->getGridSize(),"images/k.png",6);   // initialize player pass grid size,image and number of frames
            P->loadArrowImage("images/arr.png");                // Load arrow image
            P->placePlayer(x,y);
        }
         else if(obj =="chest"){
           M->loadChestImage("images/chest1.png");              // load chest image
            M->placeChest(x,y);
            }
        }
    myfile.close();
    }
}

void display(void)
{
  glClear (GL_COLOR_BUFFER_BIT);        // clear display screen

        glPushMatrix();
         M->drawBackground();
        glPopMatrix();


        for(int i=0; i<1000;i++)
        {
           W[i].drawWall();
        }

        glPushMatrix();
            M->drawGrid();
        glPopMatrix();

        glPushMatrix();
            P->drawplayer();
        glPopMatrix();

        for(int i=0; i<100;i++)
        {
        E[i].drawEnemy();
        }

        glPushMatrix();
            P->drawArrow();
        glPopMatrix();

         glPushMatrix();
           M->drawChest();
        glPopMatrix();

        glPushMatrix();
           M->drawArrows();
        glPopMatrix();

    glutSwapBuffers();
}




void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case ' ':
             P->shootArrow();
        break;
        case 27 :                       // esc key to exit
        case 'q':
            exit(0);
            break;
    }

    glutPostRedisplay();
}


 void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    xPos =posX ;
    yPos =posY ;
}

 void idle(void)
{

    //Your Code here

    glutPostRedisplay();
}


void mouse(int btn, int state, int x, int y){

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;


      case GLUT_RIGHT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;
    }
     glutPostRedisplay();
};

void Specialkeys(int key, int x, int y)
{
    // Your Code here
    switch(key)
    {
    case GLUT_KEY_UP:
        if(P->livePlayer){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
         if(wallArray[P->getPlayerLoc().x][P->getPlayerLoc().y + 1] == 1 ){} // do nothing, hit wall
         else{P->movePlayer("up",P->frames);} // move up
         if((P->getPlayerLoc().x == M->GetStArrwsLoc().x) && (P->getPlayerLoc().y == M->GetStArrwsLoc().y)){
            P->arrowStatus = true;
            P->shootArrow();}
            for(int i = 0; i < numEnemy; i++){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().x)){
            P->livePlayer = false;}
            }
        }
    break;

    case GLUT_KEY_DOWN:
        if(P->livePlayer){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(wallArray[P->getPlayerLoc().x][P->getPlayerLoc().y - 1] == 1 ){} // do nothing hit wall
          else{P->movePlayer("down",P->frames);} // move down
        if((P->getPlayerLoc().x == M->GetStArrwsLoc().x) && (P->getPlayerLoc().y == M->GetStArrwsLoc().y)){
            P->arrowStatus = true;
            P->shootArrow();}
         for(int i = 0; i < numEnemy; i++){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().x)){
            P->livePlayer = false;}
            }
        }
    break;

    case GLUT_KEY_LEFT:
        if(P->livePlayer){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(wallArray[P->getPlayerLoc().x - 1][P->getPlayerLoc().y] == 1 ){} // do nothing hit wall
          else{P->movePlayer("left",P->frames);} // move left
        if((P->getPlayerLoc().x == M->GetStArrwsLoc().x) && (P->getPlayerLoc().y == M->GetStArrwsLoc().y)){
            P->arrowStatus = true;
            P->shootArrow();}
       for(int i = 0; i < numEnemy; i++){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().x)){
            P->livePlayer = false;}
            }
        }
    break;

    case GLUT_KEY_RIGHT:
        if(P->livePlayer){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(wallArray[P->getPlayerLoc().x + 1][P->getPlayerLoc().y] == 1 ){} // do nothing hit wall
          else{P->movePlayer("right",P->frames);} // move right
          if((P->getPlayerLoc().x == M->GetStArrwsLoc().x) && (P->getPlayerLoc().y == M->GetStArrwsLoc().y)){
            P->arrowStatus = true;
            P->shootArrow();}
       for(int i = 0; i < numEnemy; i++){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().x)){
            P->livePlayer = false;}
            }
        }
    break;

    }
  glutPostRedisplay();
}
/* Program entry point */

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (800, 800);                //window screen
   glutInitWindowPosition (100, 100);            //window position
   glutCreateWindow ("Maze");                    //program title
   init();

   glutDisplayFunc(display);                     //callback function for display
   glutReshapeFunc(resize);                      //callback for reshape
   glutKeyboardFunc(key);                        //callback function for keyboard
   glutSpecialFunc(Specialkeys);
   glutMouseFunc(mouse);
   glutIdleFunc(idle);
   glutMainLoop();

   return EXIT_SUCCESS;
}
