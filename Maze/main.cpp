#include <cstring>
#include <string.h>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <Timer.h>
#include <player.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <math.h>
#include <ctime>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <Enemies.h>
#include <wall.h>
#include <math.h>

using namespace std;
const int n = 20; // horizontal size of the map
const int m = 20; // vertical size size of the map
static int map[n][m];
static int closed_nodes_map[n][m]; // map of closed (tried-out) nodes
static int open_nodes_map[n][m]; // map of open (not-yet-tried) nodes
static int dir_map[n][m]; // map of directions
const int dir = 4; // number of possible directions to go at any position
static int dx[dir]={1, 0, -1, 0};
static int dy[dir]={0, 1, 0, -1};
Maze *M = new Maze(20);                         // Set Maze grid size
Player *P = new Player();                       // create player
wall W[1000];                                  // wall with number of bricks
Enemies E[100];                                  // create number of enemies
Timer *T0 = new Timer();
string line, obj;
int x, y;
int numWall = 0;
int numEnemy = 0;
int choice;
float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping

class node
{
	// current position
	int xPos;
	int yPos;
	// total distance already travelled to reach the node
	int level;
	// priority=level+remaining distance estimate
	int priority;  // smaller: higher priority

public:
	node(int xp, int yp, int d, int p)
	{
		xPos = xp; yPos = yp; level = d; priority = p;
	}

	int getxPos() const { return xPos; }
	int getyPos() const { return yPos; }
	int getLevel() const { return level; }
	int getPriority() const { return priority; }

	void updatePriority(const int & xDest, const int & yDest)
	{
		priority = level + estimate(xDest, yDest) * 10; //A*
	}

	// give better priority to going strait instead of diagonally
	void nextLevel(const int & i) // i: direction
	{
		level += (dir == 8 ? (i % 2 == 0 ? 10 : 14) : 10);
	}

	// Estimation function for the remaining distance to the goal.
	const int & estimate(const int & xDest, const int & yDest) const
	{
		static int xd, yd, d;
		xd = xDest - xPos;
		yd = yDest - yPos;

		// Euclidian Distance
		//d = static_cast<int>(sqrt(xd*xd + yd*yd));

		// Manhattan distance
		d=abs(xd)+abs(yd);

		// Chebyshev distance
		//d=max(abs(xd), abs(yd));

		return(d);
	}
};

// Determine priority (in the priority queue)
bool operator<(const node & a, const node & b)
{
	return a.getPriority() > b.getPriority();
}

// A-star algorithm.
// The route returned is a string of direction digits.
string pathFind(const int & xStart, const int & yStart,
	const int & xFinish, const int & yFinish)
{
	static priority_queue<node> pq[2]; // list of open (not-yet-tried) nodes
	static int pqi; // pq index
	static node* n0;
	static node* m0;
	static int i, j, x, y, xdx, ydy;
	static char c;
	pqi = 0;

	// reset the node maps
	for (y = 0; y<m; y++)
	{
		for (x = 0; x<n; x++)
		{
			closed_nodes_map[x][y] = 0;
			open_nodes_map[x][y] = 0;
		}
	}

	// create the start node and push into list of open nodes
	n0 = new node(xStart, yStart, 0, 0);
	n0->updatePriority(xFinish, yFinish);
	pq[pqi].push(*n0);
	open_nodes_map[x][y] = n0->getPriority(); // mark it on the open nodes map

											  // A* search
	while (!pq[pqi].empty())
	{
		// get the current node w/ the highest priority
		// from the list of open nodes
		n0 = new node(pq[pqi].top().getxPos(), pq[pqi].top().getyPos(),
			pq[pqi].top().getLevel(), pq[pqi].top().getPriority());

		x = n0->getxPos(); y = n0->getyPos();

		pq[pqi].pop(); // remove the node from the open list
		open_nodes_map[x][y] = 0;
		// mark it on the closed nodes map
		closed_nodes_map[x][y] = 1;

		// quit searching when the goal state is reached
		//if((*n0).estimate(xFinish, yFinish) == 0)
		if (x == xFinish && y == yFinish)
		{
			// generate the path from finish to start
			// by following the directions
			string path = "";
			while (!(x == xStart && y == yStart))
			{
				j = dir_map[x][y];
				c = '0' + (j + dir / 2) % dir;
				path = c + path;
				x += dx[j];
				y += dy[j];
			}

			// garbage collection
			delete n0;
			// empty the leftover nodes
			while (!pq[pqi].empty()) pq[pqi].pop();
			return path;
		}

		// generate moves (child nodes) in all possible directions
		for (i = 0; i<dir; i++)
		{
			xdx = x + dx[i]; ydy = y + dy[i];

			if (!(xdx<0 || xdx>n - 1 || ydy<0 || ydy>m - 1 || map[xdx][ydy] == 1
				|| closed_nodes_map[xdx][ydy] == 1))
			{
				// generate a child node
				m0 = new node(xdx, ydy, n0->getLevel(),
					n0->getPriority());
				m0->nextLevel(i);
				m0->updatePriority(xFinish, yFinish);

				// if it is not in the open list then add into that
				if (open_nodes_map[xdx][ydy] == 0)
				{
					open_nodes_map[xdx][ydy] = m0->getPriority();
					pq[pqi].push(*m0);
					// mark its parent node direction
					dir_map[xdx][ydy] = (i + dir / 2) % dir;
				}
				else if (open_nodes_map[xdx][ydy]>m0->getPriority())
				{
					// update the priority info
					open_nodes_map[xdx][ydy] = m0->getPriority();
					// update the parent direction info
					dir_map[xdx][ydy] = (i + dir / 2) % dir;

					// replace the node
					// by emptying one pq to the other one
					// except the node to be replaced will be ignored
					// and the new node will be pushed in instead
					while (!(pq[pqi].top().getxPos() == xdx &&
						pq[pqi].top().getyPos() == ydy))
					{
						pq[1 - pqi].push(pq[pqi].top());
						pq[pqi].pop();
					}
					pq[pqi].pop(); // remove the wanted node

								   // empty the larger size pq to the smaller one
					if (pq[pqi].size()>pq[1 - pqi].size()) pqi = 1 - pqi;
					while (!pq[pqi].empty())
					{
						pq[1 - pqi].push(pq[pqi].top());
						pq[pqi].pop();
					}
					pqi = 1 - pqi;
					pq[pqi].push(*m0); // add the better node instead
				}
				else delete m0; // garbage collection
			}
		}
		delete n0; // garbage collection
	}
	return ""; // no route found
}

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
    glutFullScreen();
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);
   // int choice;
    fstream myfile;
    while(choice != 1 && choice != 2){
    cout << "What Level Would You like to play on? " << endl;
    cout << "1. Lava" << endl;
    cout << "2. Forest" << endl;
    cout << "Please enter 1 or 2" << endl;
    cin >> choice;
    }
    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(choice == 1){
     M->loadBackgroundImage("images/lava.jpg");           // Load maze background image
     myfile.open("lava.txt");
   }
    else{
    M->loadBackgroundImage("images/bak.jpg");           // Load maze background image
     myfile.open("forest.txt");
    }
if (myfile.is_open()){
    cout << "Able to open file" << endl;
        for (int y = 0; y<m; y++){
		for (int x = 0; x<n; x++){
            map[x][y] = 0;
            }
        }
    while ( getline (myfile,line) ){
        stringstream ss(line);
        ss >> obj >> x >> y;
        if(obj == "enemy"){
            if(choice == 1){
            E[numEnemy].initEnm(M->getGridSize(),4,"images/Dragon.png"); //Load enemy image
        } //Load enemy image
            else{E[numEnemy].initEnm(M->getGridSize(),4,"images/e.png");}
            E[numEnemy].placeEnemy(float(x%(M->getGridSize())),float(y%(M->getGridSize())));
            numEnemy++;
            }
       else if(obj == "wall"){
            if(choice == 1){
            W[numWall].wallInit(M->getGridSize(),"images/wallrock.jpg"); // Load walls
            }
            else{ W[numWall].wallInit(M->getGridSize(),"images/bush.png");}
            W[numWall].placeWall(x,y);
            //wallArray[x][y] = 1;
            numWall++;
            map[x][y] = 1;// place each brick
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
    else{
        cout << "Unable to open file" << endl;
    }
}//end file open

void display(void)
{
  glClear (GL_COLOR_BUFFER_BIT);        // clear display screen

        glPushMatrix();
         M->drawBackground();
        glPopMatrix();


        for(int i=0; i<400;i++)
        {
           W[i].drawWall();
        }

        glPushMatrix();
            M->drawGrid();
        glPopMatrix();

        glPushMatrix();
            P->drawplayer();
        glPopMatrix();


        for(int i=0; i<10;i++)
        {
            if(E[i].live){
            E[i].drawEnemy();
            }
            //else E[i].live = false;
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
           for(int i = 0; i < numEnemy; i++){
            for(int j = 0; j < 20; j++){
            if((strcmp(P->playerDir, "right")==0) && (P->getArrowLoc().x + j == E[i].getEnemyLoc().x && P->getArrowLoc().y == E[i].getEnemyLoc().y)){
                E[i].live = false;
            }
            else if((strcmp(P->playerDir, "left")==0) && (P->getArrowLoc().x - j == E[i].getEnemyLoc().x && P->getArrowLoc().y == E[i].getEnemyLoc().y)){
                E[i].live = false;
                }
            else if((strcmp(P->playerDir, "up")==0) && (P->getArrowLoc().x == E[i].getEnemyLoc().x && P->getArrowLoc().y + j == E[i].getEnemyLoc().y)){
                E[i].live = false;
                }
           else if((strcmp(P->playerDir, "down")==0) && (P->getArrowLoc().x == E[i].getEnemyLoc().x && P->getArrowLoc().y - j == E[i].getEnemyLoc().y)){
                E[i].live = false;
            }
            }
           }
        break;
        case 27 :                       // esc key to exit
        case 'q':
            exit(0);
            break;
       // case 'n': // to add menu and toggle fire and move

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

void Specialkeys(int key, int x, int y){
    switch(key)
    {
    case GLUT_KEY_UP:
        if(P->livePlayer && M->liveChest){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
         if(map[P->getPlayerLoc().x][P->getPlayerLoc().y + 1] == 1 ){} // do nothing, hit wall
         else{P->movePlayer("up",P->frames);} // move up
         if((P->getPlayerLoc().x == M->GetChestLoc().x) && (P->getPlayerLoc().y == M->GetChestLoc().y)){
            M->liveChest = false;
            P->livePlayer = false;
       }
         if((P->getPlayerLoc().x == M->GetStArrwsLoc().x && P->getPlayerLoc().y == M->GetStArrwsLoc().y) &&  M->liveSetOfArrws != false ){
            P->arrowStatus = true;
            P->numArrows = P->numArrows + 5;
            M->liveSetOfArrws = false;
           // cout << "Number of Arrows: " << P->numArrows << endl;
            }

            for(int i = 0; i < numEnemy; i++){
            if(E[i].live){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().y)){
            P->livePlayer = false;}
        else{
            string route = pathFind( E[i].getEnemyLoc().x, E[i].getEnemyLoc().y , P->getPlayerLoc().x, P->getPlayerLoc().y);
           if(route[0] == '0'){E[i].moveEnemy("right");}
            else if(route[0] == '1'){E[i].moveEnemy("up");}
             else if(route[0] == '2'){E[i].moveEnemy("left");}
            else{E[i].moveEnemy("down");}
        }
            }
            }
        }
    break;

    case GLUT_KEY_DOWN:
        if(P->livePlayer && M->liveChest){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(map[P->getPlayerLoc().x][P->getPlayerLoc().y - 1] == 1 ){} // do nothing hit wall
          else{P->movePlayer("down",P->frames);} // move down
          if((P->getPlayerLoc().x == M->GetChestLoc().x) && (P->getPlayerLoc().y == M->GetChestLoc().y)){
            M->liveChest = false;
            P->livePlayer = false;
       }
         if((P->getPlayerLoc().x == M->GetStArrwsLoc().x && P->getPlayerLoc().y == M->GetStArrwsLoc().y) &&  M->liveSetOfArrws != false ){
            P->arrowStatus = true;
             P->numArrows = P->numArrows + 5;
             M->liveSetOfArrws = false;
              cout << "Number of Arrows: " << P->numArrows << endl;
            }
         for(int i = 0; i < numEnemy; i++){
                 if(E[i].live){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().y)){
            P->livePlayer = false;}
              else{
            string route = pathFind( E[i].getEnemyLoc().x, E[i].getEnemyLoc().y , P->getPlayerLoc().x, P->getPlayerLoc().y);
          if(route[0] == '0'){E[i].moveEnemy("right");}
            else if(route[0] == '1'){E[i].moveEnemy("up");}
             else if(route[0] == '2'){E[i].moveEnemy("left");}
            else{E[i].moveEnemy("down");}
        }
            }
         }
        }
    break;

    case GLUT_KEY_LEFT:
        if(P->livePlayer && M->liveChest){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(map[P->getPlayerLoc().x - 1][P->getPlayerLoc().y] == 1 ){} // do nothing hit wall
          else{P->movePlayer("left",P->frames);} // move left
          if((P->getPlayerLoc().x == M->GetChestLoc().x) && (P->getPlayerLoc().y == M->GetChestLoc().y)){
            M->liveChest = false;
            P->livePlayer = false;
       }
        if((P->getPlayerLoc().x == M->GetStArrwsLoc().x && P->getPlayerLoc().y == M->GetStArrwsLoc().y) &&  M->liveSetOfArrws != false ){
            P->arrowStatus = true;
             P->numArrows = P->numArrows + 5;
             M->liveSetOfArrws = false;
            //  cout << "Number of Arrows: " << P->numArrows << endl;
            }
       for(int i = 0; i < numEnemy; i++){
             if(E[i].live){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().y)){
            P->livePlayer = false;}
              else{
            string route = pathFind( E[i].getEnemyLoc().x, E[i].getEnemyLoc().y , P->getPlayerLoc().x, P->getPlayerLoc().y);
            if(route[0] == '0'){E[i].moveEnemy("right");}
            else if(route[0] == '1'){E[i].moveEnemy("up");}
             else if(route[0] == '2'){E[i].moveEnemy("left");}
            else{E[i].moveEnemy("down");}
        }
            }
       }
        }
    break;

    case GLUT_KEY_RIGHT:
        if(P->livePlayer && M->liveChest){
         cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
          if(map[P->getPlayerLoc().x + 1][P->getPlayerLoc().y] == 1 ){} // do nothing hit wall
          else{P->movePlayer("right",P->frames);} // move right
          if((P->getPlayerLoc().x == M->GetChestLoc().x) && (P->getPlayerLoc().y == M->GetChestLoc().y)){
            M->liveChest = false;
            P->livePlayer = false;
       }
          if((P->getPlayerLoc().x == M->GetStArrwsLoc().x && P->getPlayerLoc().y == M->GetStArrwsLoc().y) &&  M->liveSetOfArrws != false ){
            P->arrowStatus = true;
             P->numArrows = P->numArrows + 5;
            M->liveSetOfArrws = false;
            // cout << "Number of Arrows: " << P->numArrows << endl;
            }
       for(int i = 0; i < numEnemy; i++){
            if(E[i].live){
         if((P->getPlayerLoc().x == E[i].getEnemyLoc().x) && (P->getPlayerLoc().y == E[i].getEnemyLoc().y)){
            P->livePlayer = false;}
              else{
            string route = pathFind( E[i].getEnemyLoc().x, E[i].getEnemyLoc().y , P->getPlayerLoc().x, P->getPlayerLoc().y);
           if(route[0] == '0'){E[i].moveEnemy("right");}
            else if(route[0] == '1'){E[i].moveEnemy("up");}
             else if(route[0] == '2'){E[i].moveEnemy("left");}
            else{E[i].moveEnemy("down");}
        }
            }
       }
        }
    break;

    }
  glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (1280, 720);                //window screen
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
