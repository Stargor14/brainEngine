#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std; 
int SCREEN_WIDTH=900,SCREEN_HEIGHT=900;	
class line{
public:
    float verts[6];
    float width;
    line(){
    }
    line(float x1, float x2, float y1, float y2, float w){
        verts[0]=x1;
        verts[1]=y1;
        verts[2]=0.0;
        verts[3]=x2;
        verts[4]=y2;
        verts[5]=0.0; 
        width=w;
    } 
};
void init(){//misc things i dont understand yet
   	glViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
   	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 1);
    	glMatrixMode(GL_MODELVIEW);
    	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    	glLoadIdentity();
}
void drawLine(line l, float o){
    glColor3f(o,o,o);
    glEnable( GL_LINE_SMOOTH );
    //glEnable( GL_LINE_STIPPLE );
    //glPushAttrib( GL_LINE_BIT );
    glLineWidth( l.width );
    glLineStipple( 1, 0x00FF );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, l.verts);
    glDrawArrays( GL_LINES, 0, 2 );
    glDisableClientState( GL_VERTEX_ARRAY );
    //glPopAttrib( );
    //glDisable( GL_LINE_STIPPLE );
    glDisable( GL_LINE_SMOOTH );
}
void drawPoint(float x, float y){
	glBegin(GL_POINTS);
 	glPointSize(50.0);
    	glColor3f(0.0,1.0,0.5);
 	glVertex3f(x, y,0.0f);
 	glEnd();	
}
struct network{
	float neuronMutationRate,connectionMutationRate,neuronMutationStrength,paramMutationRate;
	int layers,connectionNum,neuronNum,id;
	int* layerNeuronCount;//number of neurons per layer, max 2^28  
	float* neuronWeights;//array of floats for weights and values of each node, maxvalue floatmax
	float* neuronValues;
	u_long* connections;//64 bit data type, 28 bits for id, 4 bits for layer number, 32 for input 32 for output
};

network* initNet(const char* file){
	network* n=(network*)malloc(sizeof(network));	
	FILE* f;
	f=fopen(file,"rb");
	fread(&n->neuronMutationRate,sizeof(float),1,f);
	fread(&n->connectionMutationRate,sizeof(float),1,f);
	fread(&n->neuronMutationStrength,sizeof(float),1,f);
	fread(&n->paramMutationRate,sizeof(float),1,f);
	fread(&n->layers,sizeof(int),1,f);
	fread(&n->neuronNum,sizeof(int),1,f);
	fread(&n->connectionNum,sizeof(int),1,f);
	fread(&n->id,sizeof(int),1,f);
	n->layerNeuronCount=(int*)malloc(sizeof(int)*n->layers);//allocate memory for the neurons and connections 
	n->neuronWeights=(float*)malloc(sizeof(float)*n->neuronNum);//allocate memory for the neurons and connections 
	n->neuronValues=(float*)malloc(sizeof(float)*n->neuronNum);//allocate memory for the neurons and connections 
	n->connections=(u_long*)malloc(sizeof(long)*n->connectionNum);//allocate memory for the neurons and connections 
	fread(n->layerNeuronCount,sizeof(int),n->layers,f);
	fread(n->neuronWeights,sizeof(float),n->neuronNum,f);
	//neuronValues not required, since they are always initialized at 0
	fread(n->connections,sizeof(long),n->connectionNum,f);
	fclose(f);
//	clear(n);//make sure the neuronValues are set at 0, might be redundant?
	return n;	
}
/*
for(int i=0;i<n->connectionNum;i++){
		std::cout<<"connection "<<i<<" "<<"to_layer: "<<(n->connections[i]&0xF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"to_address: "<<((n->connections[i]>>4)&0xFFFFFFF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"from_layer: "<<((n->connections[i]>>32)&0xF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"from_address: "<<((n->connections[i]>>36))<<'\n'<<'\n';
	}
	*/
line* createLines(network* n){
	line* ans=(line*)malloc(sizeof(line)*n->connectionNum);
	for(int i=0;i<n->connectionNum;i++){
		ans[i]=line(
				SCREEN_WIDTH/n->layers*((n->connections[i]>>32)&0xF)+50,
				SCREEN_WIDTH/n->layers*(n->connections[i]&0xF)+50,
				904/SCREEN_HEIGHT*((n->connections[i]>>36))+(904-n->layerNeuronCount[((n->connections[i]>>32)&0xF)])/2,
				904/SCREEN_HEIGHT*((n->connections[i]>>4)&0xFFFFFFF)+(904-n->layerNeuronCount[(n->connections[i]&0xF)])/2
				,0.1);
	}
	return ans;
}
int main(int argc, char* argv[] ){
    GLFWwindow *window;
    // Initialize the library
    network* n=initNet(argv[1]);
    if (!glfwInit())return -1;
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "network visualizer", NULL, NULL);
    if (!window){//if window failed to init
        glfwTerminate( );
        return -1;
    }
    glfwMakeContextCurrent(window);//making "context" current? dont understand
    init();
    glfwSetWindowPos(window, 0, 0);
    glfwFocusWindow(window);
    
    double xpos, ypos;
   line* lines=createLines(n); 
   int c=0;
   //from layer:((n->connections[i]>>32)&0xF) 
   //from address((n->connections[i]>>36))
   //to layer(n->connections[i]&0xF)
   //to address((n->connections[i]>>4)&0xFFFFFFF)
   map<pair<int,int>,vector<line>> groupedConnections;//conncections grouped by neuron
	for(int i=0;i<n->connectionNum;i++){
		pair<int,int> temp=make_pair(((n->connections[i]>>32)&0xF),((n->connections[i]>>36)));
		if(groupedConnections.count(temp)==0){
			//groupedConnections.insert(vector<line>());
		}
		groupedConnections[temp].push_back(lines[i]);
	}
map<pair<int,int>,vector<line>>::iterator it=groupedConnections.begin();
while (!glfwWindowShouldClose(window)){//while the window is not closed
        glClear( GL_COLOR_BUFFER_BIT );//clear the screen
        glfwGetCursorPos(window, &xpos, &ypos);
	for(int i=0;i<n->connectionNum;i++){
		drawLine(lines[i],0.2);
	}
	for(int i=0;i<n->layers;i++){
		for(int p=0;p<n->layerNeuronCount[i];p++){
			drawPoint(SCREEN_WIDTH/n->layers*i+50,p+(904-n->layerNeuronCount[i])/2);
		}	
	}
	//iteratre through map and draw lines of each neuron
	if(it==groupedConnections.end())it=groupedConnections.begin();
		
	for(int i=0;i<it->second.size();i++)drawLine(it->second[i],1);
		it++;
	if(c>n->connectionNum)c=0;
	glfwSwapBuffers(window);//swap the buffer being shown and the buffer that was being drawn to
        glfwPollEvents();//check for screen inputs
    }
    glfwTerminate();
    return 0;
}

