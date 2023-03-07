#ifndef VISUAL_H
#define VISUAL_H
#include "neuro.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
namespace visual{
extern int SCREEN_WIDTH,SCREEN_HEIGHT;	
void init();
class line;
void drawLine(line l);
void drawPoint(float x, float y);
void drawNetwork(neuro::network* n);
};

#endif 
