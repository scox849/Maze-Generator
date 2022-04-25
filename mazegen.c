/*********************/
/* Samuel Cox        */
/* CS-241-002        */
/* Generateds random */
/* mazes, solves and */
/* prints them.      */
/*********************/






#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mazegen.h"

#define MIN_SIZE 3
#define DIR_SIZE 4
#define EMPTY 0
#define NO_DIR -1
#define PIPLIST_INDEX 15
#define MAX_END_PIPE 8
#define CROSS 15
#define MIN_WAYPOINT 1
#define STRAIGHT_P_MOD 10

//Used for my testing please ignore.

const int DIRECTION_LIST[] = {NORTH, EAST, SOUTH, WEST};
const int DIRECTION_DX[]   = {    0,    1,     0,   -1};
const int DIRECTION_DY[]   = {   -1,    0,     1,    0};


const unsigned char pipeList[] =
{
  219, 208, 198, 200, 210, 186, 201, 204, 
  181, 188, 205, 202, 187, 185, 203, 206
};

// Functon prototypes and global variables
static unsigned char **maze;
int carveMaze(int x, int y, int dir);
int pickDir(int x, int y);
void fillMaze();
int getRand();
int flipDir(int dir);
int solveMaze(int x, int y);
void carveWaypointAllies(int x, int y, int distance);
int isEndPipe(char pipe);
int getNextAlleyEnd();
void shuffleEndPoints();
void copyIntToAddress(int n, unsigned char bytes[]);

int solving = FALSE;
int mazeWidth;
int mazeHeight;
int cleared = 1;
int clearedEmpties;
int startX;
int startY;
int foundBeg = FALSE;
int foundEnd = FALSE;
int wayPointXf;
int wayPointYf;
int exitX;
int exitY;
int maxCellsToCarve = 0;
int allyLength;
int endsOfMazeX[DIR_SIZE];
int endsOfMazeY[DIR_SIZE];
int endsOfMazeIndex = 0;
int endsCleared = FALSE;
int printAs = FALSE;
double straightP;
double wayPointDp;
int cellsCarved = 0;
int bmp = TRUE;


/* Allocates the memory for the maze
   check if all parameters are within
   the limits and calls carveMaze. 
   returns an int. 0 for all is well
   1 for an error. Takes width, height
   wayPointx, wayPointy, waypointalleyLength,
   straightProbability and printAlgorithm steps
   as argruments.                               */

int mazeGenerate(int width, int height,
    int wayPointX, int wayPointY,      
    int wayPointAlleyLength,           
    double wayPointDirectionPercent,  
    double straightProbability,    
    int printAlgorithmSteps)
{
    int dir;
    //checks if mazeFree has been previosly called
    if(cleared == 0)
    {
        mazeFree();
    }

    //Assigns or reassigns global variables to make sure
    //new calls to mazeGenerate work correctly.
    mazeWidth = width;
    mazeHeight = height;
    wayPointXf = wayPointX - 1;
    wayPointYf = wayPointY - 1;
    clearedEmpties = width * height;
    allyLength = wayPointAlleyLength;
    endsOfMazeIndex = 0;
    straightP = straightProbability;
    cellsCarved = 0;
    endsCleared = FALSE;
    //Error Checking
    if(wayPointDirectionPercent > 0.0)
    {
        wayPointDp = wayPointDirectionPercent;
        maxCellsToCarve = clearedEmpties * wayPointDirectionPercent;    
    }
    else
    {
        printf("ERROR: wayPointDirectionPercent is too small.\n");
        return 1;
    }
    if(width < MIN_SIZE || height < MIN_SIZE)
    {
        printf("ERROR: Dimensions too small.\n");
        return 1;
    }
    if((wayPointX-1) >= width || (wayPointY-1) >= height ||
       (wayPointX - 1) <= 0 || (wayPointY - 1) <= 0)
    {
        printf("ERROR: Waypoint out of bounds.\n");
        return 1;
    }

    //Checks if printAlgorithmSteps is set to true.
    if(printAlgorithmSteps == TRUE)
    {
        printAs = TRUE;
    }
    else
    {
        printAs = FALSE;
    }

    // Memory Allocation
    maze = malloc(width * sizeof(*maze));

    for(int i = 0; i < width; i++)
    {
        maze[i] = malloc(height * sizeof(int));
    }

    cleared = 0;

    
    //Fills the maze with empty characters
    fillMaze();

    //Begins creating maze based on wayPointAlleyLength.
    if(wayPointAlleyLength > 0)
    {
        carveWaypointAllies(wayPointXf, wayPointYf, wayPointAlleyLength);
        foundBeg = FALSE;
        foundEnd = FALSE;    
    }
    else
    {
        dir = pickDir(wayPointXf,wayPointYf);
        carveMaze(wayPointXf, wayPointYf, dir);
        foundBeg = FALSE;
        foundEnd = FALSE;
                        
    }
    
    //retunrs 0 because all is well.
    return 0;

}

/* Recursively creates the maze. Takes and 
   x, y , and direction it came from as args.
   Returns an int.                           */
int carveMaze(int x, int y, int dir)
{

    int cameFrom = DIRECTION_LIST[dir];
    int newDir;
    int goingTo;

    //loop until all maze locations are filled
    while(clearedEmpties != EMPTY)
    {

        
        //Checks straightProbablilty and gets the correct direction
        //accordingly.
        if(straightP > 0.0)
        {
            int straightPInt = straightP * STRAIGHT_P_MOD;
            int randomCheck = rand() % STRAIGHT_P_MOD;
            if(randomCheck < straightPInt)
            {
                dir = flipDir(dir);
                if((x + DIRECTION_DX[dir]) >= 0 && (x + DIRECTION_DX[dir]) < mazeWidth
                    && (y + DIRECTION_DY[dir]) >= 0 && (y + DIRECTION_DY[dir]) < mazeHeight)
                {
                    if(maze[x + DIRECTION_DX[dir]][y + DIRECTION_DY[dir]] == EMPTY)
                    {
                        newDir = dir;
                    }
                    else
                    {
                        newDir = pickDir(x,y);
                    }
                }
                else
                {
                    newDir = pickDir(x,y);
                }
            }
            else
            {
                newDir = pickDir(x,y);
            }
        }
        else
        {
            newDir = pickDir(x,y);    
        } 
        goingTo = DIRECTION_LIST[newDir];

        //Checks if new cell is empty.
        if(maze[x][y] == EMPTY)
        {
            clearedEmpties--;
            //Counter is wayPointDirectionPercent is < 1.0
            if(wayPointDp < 1.0)
            {
                cellsCarved++;  
            }          
        }

        //Used for wayPointDirectionPercent to check if the number of
        //cells it is allowed to carve has been reached.
        if(cellsCarved == maxCellsToCarve)
        {
            maze[x][y] = cameFrom;
            cellsCarved = EMPTY;
            //Gets end of next ally.
            if(allyLength > EMPTY)
            {
                int indexOfEnd = NO_DIR;

                indexOfEnd = getNextAlleyEnd();
                
                if(indexOfEnd != NO_DIR)
                {
                    newDir = pickDir(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd]);
                    carveMaze(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd], newDir);    
                }
      
            }
            else
            {
                //Used if alley length is 0
                newDir = pickDir(wayPointXf,wayPointYf);
                if(newDir == NO_DIR)
                {
                    return 0;
                }
                else
                {
                    carveMaze(wayPointXf,wayPointYf, newDir);    
                }       
            }
            return 0;
        }
        //Checks if the cell its carving from is the waypoint.
        if(x == wayPointXf && y == wayPointYf)
        {
            maze[x][y] += goingTo;
        }
        else if((y == (mazeHeight - 1) || y == 0) && x != wayPointXf) //Chooses begining and end of maze.
        {
            if(maze[x][y] == EMPTY)
            {
                //Breaks open a hole for the beginning.
                if(foundBeg == FALSE)
                {
                    if(y == 0)
                    {
                        goingTo = NORTH;    
                    }
                    else
                    {
                        goingTo = SOUTH;
                    }
                    maze[x][y] = (cameFrom + goingTo);
                    startX = x;
                    startY = y;
                    foundBeg = TRUE;

                    if(wayPointDp < 1.0)
                    {
                        cellsCarved = EMPTY;
                    }            
                    if(printAs == TRUE)
                    {
                        mazePrint();
                    }

                    if(allyLength > EMPTY)
                    {
                        int indexOfEnd = NO_DIR;

                        indexOfEnd = getNextAlleyEnd();
                        if(indexOfEnd != NO_DIR)
                        {
                            newDir = pickDir(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd]);
                            carveMaze(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd], newDir);    
                        }     
                    }
                    else
                    {
                        newDir = pickDir(wayPointXf,wayPointYf);
                        carveMaze(wayPointXf,wayPointYf, newDir);
                    }

                    return 0;
                }
                else if(foundEnd == FALSE)//Breaks open hole for end of maze.
                {
                    if(y == 0)
                    {
                        goingTo = NORTH;    
                    }
                    else
                    {
                        goingTo = SOUTH;
                    }
                    maze[x][y] = (cameFrom + goingTo);
                    exitX = x;
                    exitY = y;
                    foundEnd = TRUE;
                    if(wayPointDp < 1.0)
                    {
                        cellsCarved = 0;
                    }
                    if(printAs == TRUE)
                    {
                        mazePrint();
                    }
                    if(allyLength > 0 && endsCleared == FALSE)
                    {
                        int indexOfEnd = NO_DIR;

                        indexOfEnd = getNextAlleyEnd();
                        
                        if(indexOfEnd != NO_DIR)
                        {
                            newDir = pickDir(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd]);
                            carveMaze(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd], newDir);    
                        }
                              
                    }
                    return 0;   
                }
            }
        }
        //if no directions to go return or go to next alley end
        //else fill the maze with the proper pipes
        if(newDir == NO_DIR)
        {
            if(maze[x][y] == EMPTY)
            {
                maze[x][y] = cameFrom;
                if(printAs == TRUE)
                {
                    mazePrint();
                }
            }
            if(allyLength > EMPTY && endsCleared == FALSE)
            {
                int indexOfEnd = NO_DIR;

                indexOfEnd = getNextAlleyEnd();
                
                if(indexOfEnd != NO_DIR)
                {
                    newDir = pickDir(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd]);
                    carveMaze(endsOfMazeX[indexOfEnd], endsOfMazeY[indexOfEnd], newDir);    
                }             
            }

            return 0;
        }
        else if(maze[x][y] == EMPTY)
        {
            maze[x][y] = (cameFrom + goingTo);
        }
        else if(maze[x][y] != EMPTY)
        {
            if(x == wayPointXf && y == wayPointYf)
            {
                //catch to ensure waypoint remains the correct pipe
                //I want nothing to be done to the waypoint here.    
            }
            else
            {
                maze[x][y] += goingTo;
            }
        }
        //increment x and y flip where it is going to where it came from 
        //call carvemaze again.
        x += DIRECTION_DX[newDir];
        y += DIRECTION_DY[newDir];
        newDir = flipDir(newDir);
        carveMaze(x, y, newDir);
    }
    
}


/* Returns the next end of the waypoint alley*/
int getNextAlleyEnd()
{

    int tempDir = NO_DIR;
    for(int i = 0; i < endsOfMazeIndex; i++)
    {
        if(isEndPipe(maze[endsOfMazeX[i]][endsOfMazeY[i]]))
        {
            if((tempDir = pickDir(endsOfMazeX[i], endsOfMazeY[i])) != NO_DIR)
            {
                return i;
            }
        }
    }
    endsCleared = TRUE;
    return NO_DIR;   
     
    
}

/* Takes a char as an argument. Returns TRUE or FALSE.
   Checks to see if the given pipe character is an end 
   cap.                                                */
int isEndPipe(char pipe)
{
    switch(pipe)
    {
        case NORTH:
            return TRUE;
            break;
        case SOUTH:
            return TRUE;
            break;
        case EAST:
            return TRUE;
            break;
        case WEST: 
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

/* Carves allies in 4 directions of the given length from the given waypoint 
   Takes and x index y index and alley distance as arguments                */
void carveWaypointAllies(int x, int y, int distance)
{
    int goingTo;
    int comingFrom;
    int tempx = x;
    int tempy = y;
    int reverse;

    
    for(int i = 0; i < DIR_SIZE; i++)
    {
        
        goingTo = DIRECTION_LIST[i];
        reverse = flipDir(i);
        comingFrom = DIRECTION_LIST[reverse];
        for(int j = 0; (j < distance + 1); j++)
        {
            //Bounds Check
            if((tempx + DIRECTION_DX[i]) >= 0 && (tempx + DIRECTION_DX[i]) < mazeWidth
                && (tempy + DIRECTION_DY[i]) >= 0 && (tempy + DIRECTION_DY[i]) < mazeHeight)
            {
                //Places proper pipe.
                if(tempx == x && tempy == y)
                {
                    maze[tempx][tempy] += goingTo;
                    tempx += DIRECTION_DX[i];
                    tempy += DIRECTION_DY[i];
                    if(clearedEmpties == (mazeWidth * mazeHeight))
                    {
                        clearedEmpties--;
                    }
                }
                else if(j == distance)
                {
                    maze[tempx][tempy] = comingFrom;
                    endsOfMazeX[endsOfMazeIndex] = tempx;
                    endsOfMazeY[endsOfMazeIndex] = tempy;
                    endsOfMazeIndex++;
                    tempx += DIRECTION_DX[i];
                    tempy += DIRECTION_DY[i];
                    clearedEmpties--;
                    if(printAs == TRUE)
                    {
                        mazePrint();
                    }
                }
                else
                {
                    maze[tempx][tempy] = (comingFrom + goingTo);
                    tempx += DIRECTION_DX[i];
                    tempy += DIRECTION_DY[i];
                    clearedEmpties--;   
                }   
            }
            else if(tempx == 0 || tempy == 0 || tempx == (mazeWidth - 1) || 
                    tempy == (mazeHeight - 1)) // Check if alleylength goes beyond the edge of the maze.
            {
                
                maze[tempx][tempy] = comingFrom;
                endsOfMazeX[endsOfMazeIndex] = tempx;
                endsOfMazeY[endsOfMazeIndex] = tempy;
                endsOfMazeIndex++;
                clearedEmpties--;
                if(printAs == TRUE)
                {
                    mazePrint();
                }
                break;        
            }
        }
        tempx = x;
        tempy = y;
    }

    //Shuffles the end points so that they are random.
    shuffleEndPoints();
    int randomIndex;
    //Carves maze starting from the end of a random alley.
    while(clearedEmpties != EMPTY)
    {
        randomIndex = rand() % endsOfMazeIndex;
        carveMaze(endsOfMazeX[randomIndex],endsOfMazeY[randomIndex],
                  pickDir(endsOfMazeX[randomIndex],endsOfMazeY[randomIndex]));    
    }
    
}



/* Randomizes the end points if alley length  > 0 */
void shuffleEndPoints()
{
    int tempEndX;
    int tempEndY;
    int randomIndex;
    for(int i = 0; i < endsOfMazeIndex; i++)
    {
        randomIndex = rand() % endsOfMazeIndex;
        tempEndX = endsOfMazeX[randomIndex];
        tempEndY = endsOfMazeY[randomIndex];
        endsOfMazeX[randomIndex] = endsOfMazeX[i];
        endsOfMazeY[randomIndex] = endsOfMazeY[i];
        endsOfMazeX[i] = tempEndX;
        endsOfMazeY[i] = tempEndY;

    }
}

/* Filps the given direction to its opposite
   takes an index direction as an argument and returns 
   the index to the opposite direction */
int flipDir(int dir)
{
    int north = 0;
    int south = 2;
    int east = 1;
    int west = 3;
    int oldDir = DIRECTION_LIST[dir];
    if(oldDir == NORTH)
    {
        return south;
    }
    else if(oldDir == SOUTH)
    {
        return north;
    }
    else if(oldDir == EAST)
    {
        return west;
    }
    else if(oldDir == WEST)
    {
        return east;
    }

}

/*fills the maze with empty blocks*/
void fillMaze()
{
    for(int i = 0; i < mazeWidth; i++)
    {
        for(int j = 0; j < mazeHeight; j++)
        {
            maze[i][j] = EMPTY;
        }    
    }
}


/* Finds the possible directions for the maze to go in.
   take x and y as an argument and returns index to a 
   valid direction.                                   
   Used for solving as well.                           */
int pickDir(int x, int y)
{
    int posDir[DIR_SIZE];
    int arrIndex = 0;
    int randIndex;
    int picked = FALSE;
    for(int i = 0; i < DIR_SIZE; i++)
    {
        if((x + DIRECTION_DX[i]) >= 0 && (x + DIRECTION_DX[i]) < mazeWidth
            && (y + DIRECTION_DY[i]) >= 0 && (y + DIRECTION_DY[i]) < mazeHeight)
        {
            //if used for carvemaze. else if used for solvemaze.
            if(maze[x+DIRECTION_DX[i]][y+DIRECTION_DY[i]] == EMPTY && solving == FALSE)
            {    
                posDir[arrIndex] = i;
                arrIndex++;
                picked = TRUE;         
            }
            else if(solving && (maze[x][y] & DIRECTION_LIST[i]) > 0 && 
                    (maze[x + DIRECTION_DX[i]][y + DIRECTION_DY[i]] < GOAL))
            {
                posDir[arrIndex] = i;
                arrIndex++;
                picked = TRUE;
            }
        }
    }
    if(picked == TRUE)
    {
        randIndex = rand() % arrIndex;
        return posDir[randIndex];
    }
    else
    {
        return NO_DIR;
    }
}


/* Starts solving of the maze.*/
void mazeSolve()
{
    solving = TRUE;
    solveMaze(startX, startY);
    solving = FALSE;
  
}

/* Solves the maze by checking random directions and marking
   the pipes with GOAL for part of the solution or 
   VISITED for not part of the solution 
   until the end is found. Returns and int and 
   takes a X and Y as parameters.        */
int solveMaze(int x, int y)
{
    int nextDir;

    while(solving == TRUE)
    {
        //Pick whitch direction to check
        nextDir = pickDir(x,y);
        //printf("MAZE VALUE %d\n",maze[x][y]);
        maze[x][y] |= GOAL;
        //check for if end was found
        if(x == exitX && y == exitY)
        {
            solving = FALSE;
            return 0;
        }
        //return if no valid directions
        if(nextDir == NO_DIR)
        {
            maze[x][y] |= VISITED;
            return 0;
        }
        
        solveMaze(x + DIRECTION_DX[nextDir], y + DIRECTION_DY[nextDir]);

        
    }

}
/* Copies an integer to address. */
void copyIntToAddress(int n, unsigned char bytes[])
{
    bytes[0] = n & 0xFF;
    bytes[1] = (n >>  8) & 0xFF;
    bytes[2] = (n >> 16) & 0xFF;
    bytes[3] = (n >> 24) & 0xFF;
}

/* Skipes the header of the bmp file read in. */
void skipHeader(FILE* currentPipe)
{
    int headerSize = 54;
    unsigned char ignore[headerSize];
    for(int i = 0; i < headerSize; i++)
    {
        fscanf(currentPipe,"%c", ignore);

    }
}

/* Transfers the bmp file read in into the main img array. */

int transferPipe(unsigned char *img, unsigned char *pipeBlock, int rowSize, 
                 int imgIndex, int offset, int verticalOffset)
{
    int pipeIndex = 0;
    int endOfFirstRow = 0;;
    int tempImgIndex = imgIndex;
    for(int j = 0; j < 8; j++)
    {
        for(int i = 0; i < 24; i++)
        {
            img[imgIndex] = pipeBlock[pipeIndex];
            pipeIndex++;
            if(i == 23)
            {
                if(j == 0)
                {
                    
                    endOfFirstRow = (imgIndex +1);
                }

                if(verticalOffset > 0)
                {
                    imgIndex = (tempImgIndex + (rowSize * (j +1)));

                }
                else
                {
                    imgIndex = ((rowSize * (j +1) + (24 * offset)));    
                }
                
            }
            else
            {
                imgIndex++;
            }
            if(imgIndex == ((rowSize * 8 * (verticalOffset + 1)) - 1))
            {
                endOfFirstRow = ((rowSize * 8 * (verticalOffset + 1)));
            }
        }
    }
    return endOfFirstRow;
}



/* Prints the maze using the proper
   colors based on printAlgorithmSteps, and solvemaze.*/
void mazePrint()
{

    //Error checking to make sure a nonexistant maze is not accessed.
    if(mazeWidth < MIN_SIZE || mazeHeight < MIN_SIZE)
    {
        return;
    }
    if(wayPointXf >= mazeWidth || wayPointYf >= mazeHeight ||
       wayPointXf <= 0 || wayPointYf <= 0)
    {
        return;
    }
    if(wayPointDp == 0.0)
    {
        return;
    }

    printf("\n\n");
    printf("========================\n");
    printf("Maze(%d x %d): (%d, %d)\n",
           mazeWidth, mazeHeight, (wayPointXf +1), (wayPointYf+1));
    printf("========================\n");

    if(bmp == TRUE)
    {
        int pixelWidth = (8 * mazeWidth);
        int pixelHeight = (8 * mazeHeight);
        int rowSize = pixelWidth * 3;
        int rowPadding = 0;
        rowPadding = (4 - (rowSize % 4)) % 4;
        rowSize += rowPadding;
        int pixelDataSize = rowSize*pixelHeight;
        int fileSize = 54 + pixelDataSize;
        int headerSize = 54;

        printf("rowPadding=%d bytes\n", rowPadding);
        printf("rowSize   =%d bytes\n", rowSize);

        unsigned char header[54] =
        {
            'B','M',  // magic number
            0,0,0,0,  // size in bytes (set below)
            0,0,0,0,  // reserved
            54,0,0,0, // offset to start of pixel data
            40,0,0,0, // info hd size
            0,0,0,0,  // image width (set below)
            0,0,0,0,  // image heigth (set below)
            1,0,      // number color planes
            24,0,      // bits per pixel
            0,0,0,0,  // compression is none
            0,0,0,0,  // image bits size
            0x13,0x0B,0,0, // horz resoluition in pixel / m
            0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
            0,0,0,0,  // #colors in pallete
            0,0,0,0,  // #important colors
        };
        copyIntToAddress(fileSize, &header[2]);
        copyIntToAddress(pixelWidth, &header[18]);
        copyIntToAddress(pixelHeight, &header[22]);
        copyIntToAddress(pixelDataSize, &header[34]);

        unsigned char img[pixelDataSize];
        memset(img,0xFF,sizeof(img));
        
	
        FILE* bmMaze = fopen("maze.bmp","w+");
        FILE* currentPipe = fopen("0.bmp", "r+");

        int pipeBlockSize = 64 * 3;
        unsigned char pipeBlock[pipeBlockSize];
        int imgIndex = 0;
        int tempIndex = 0;
        int verticalOffset = 0;

        

        
        for(int i = (mazeHeight -1); i >= 0; i--)
        {
            for(int j = 0; j < mazeWidth; j++)
            {
                fclose(currentPipe);
                switch(maze[j][i] & PIPLIST_INDEX)
                {
                    case 1:
                        currentPipe = fopen("1.bmp", "r+");
                        break;
                    case 2:
                        currentPipe = fopen("2.bmp", "r+");
                        break;
                    case 3:
                        currentPipe = fopen("3.bmp", "r+");
                        break;
                    case 4:
                        currentPipe = fopen("4.bmp", "r+");
                        break;
                    case 5:
                        currentPipe = fopen("5.bmp", "r+");
                        break;
                    case 6:
                        currentPipe = fopen("6.bmp", "r+");
                        break;
                    case 7:
                        currentPipe = fopen("7.bmp", "r+");
                        break;
                    case 8:
                        currentPipe = fopen("8.bmp", "r+");
                        break;
                    case 9:
                        currentPipe = fopen("9.bmp", "r+");
                        break;
                    case 10:
                        currentPipe = fopen("10.bmp", "r+");
                        break;
                    case 11:
                        currentPipe = fopen("11.bmp", "r+");
                        break;
                    case 12:
                        currentPipe = fopen("12.bmp", "r+");
                        break;
                    case 13:
                        currentPipe = fopen("13.bmp", "r+");
                        break;
                    case 14:
                        currentPipe = fopen("14.bmp", "r+");
                        break;
                    case 15:
                        currentPipe = fopen("15.bmp", "r+");
                        break;
                    default:
                        currentPipe = fopen("0.bmp", "r+");
                        break;
                }
                skipHeader(currentPipe);
                fread(pipeBlock, 3, 64, currentPipe);
                tempIndex = transferPipe(img,pipeBlock,rowSize,imgIndex, j,verticalOffset);
                if(tempIndex < pixelDataSize)
                {
                    imgIndex = tempIndex;
                }
                

            }
            verticalOffset++;

            
        }

        fwrite(header, 1, sizeof(header), bmMaze);
        fwrite(img,1,sizeof(img),bmMaze);
        fclose(currentPipe);
        fclose(bmMaze);   


            
    

    }
    if(bmp == TRUE || bmp == FALSE)
    {
        for(int i = 0; i < mazeHeight; i++)
        {
            for(int j = 0; j < mazeWidth; j++)
            {
                if(maze[j][i] == EMPTY)
                {
                    textcolor(TEXTCOLOR_WHITE);
                    printf("%c", pipeList[maze[j][i]&PIPLIST_INDEX]);
                }
                else if(j == wayPointXf && i == wayPointYf && maze[j][i] <= PIPLIST_INDEX)
                {
                    textcolor(TEXTCOLOR_RED);
                    printf("%c", pipeList[maze[j][i]&PIPLIST_INDEX]);
                    textcolor(TEXTCOLOR_WHITE);   
                }
                else if(maze[j][i] & VISITED) //Check if index has been VISITED during solveMaze
                {
                    if(printAs == TRUE)
                    {
                        textcolor(TEXTCOLOR_RED);
                    }
                    else
                    {
                        textcolor(TEXTCOLOR_WHITE);
                        if(j == wayPointXf && i == wayPointYf)
                        {
                            textcolor(TEXTCOLOR_RED);
                        }    
                    } 
                    printf("%c",pipeList[maze[j][i] & PIPLIST_INDEX]);    
                }
                else if(maze[j][i] & GOAL)//Check if index is part of the solution.
                {
                    if(j == wayPointXf && i == wayPointYf)
                    {
                        if(printAs == TRUE)
                        {
                            textcolor(TEXTCOLOR_GREEN);
                        }
                        else
                        {
                            textcolor(TEXTCOLOR_RED);    
                        }
                        printf("%c",pipeList[maze[j][i] & PIPLIST_INDEX]);
                        textcolor(TEXTCOLOR_WHITE);
                    }
                    else
                    {
                        textcolor(TEXTCOLOR_GREEN);
                        printf("%c",pipeList[maze[j][i] & PIPLIST_INDEX]);
                        if(printAs == TRUE)
                        {
                            textcolor(TEXTCOLOR_RED);
                        }
                        else
                        {
                            textcolor(TEXTCOLOR_WHITE);    
                        }   
                    }
                }
                else
                {
                    if(printAs == TRUE)//Prints pipes in red if printAlgorithmSteps is true.
                    {
                        textcolor(TEXTCOLOR_RED);
                    }
                    printf("%c", pipeList[maze[j][i] & PIPLIST_INDEX]);
                    textcolor(TEXTCOLOR_WHITE);    
                }
                
            }
            printf("\n");
            textcolor(TEXTCOLOR_WHITE);
        }    
    }
}

/*Clears the allocated memory */
void mazeFree()
{
    if(cleared == 1)
    {
        return;
    }
    for(int i = 0; i < mazeWidth; i++)
    {
        free(maze[i]);
    }

    free(maze);
    cleared = 1;    
}

/*This portion is for my own testing please ignore.*/

/* Changes text color of terminal */
void textcolor(int color)
{
    printf("%c[%d;%d;%dm", 0x1B, 0, color, 40); 
}

void main()
{
    //(unsigned long) time(NULL)
    srand((unsigned long) time(NULL));
    
    mazeGenerate(20,20,   10,10,3,  0.4,  0.3, FALSE); 
    mazePrint();
    mazeSolve();
    mazePrint();
    mazeFree();
    
}

