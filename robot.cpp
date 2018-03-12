
#include "Angel.h"

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

GLuint vaoSphere;
GLuint vaoRobot;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
const int NumSpherePoints = 684;
const GLfloat Pi = 3.1415926535898;
const GLfloat step = 0.1745329251994; // 1 / 18 * Pi;
const GLfloat R = 0.25;

point4 points[NumVertices];
color4 colors[NumVertices];
point4 spherePoints[NumSpherePoints];
color4 sphereColors[NumSpherePoints];
point4 center;
float old_x, old_y, old_z, new_x, new_y, new_z;

point4 vertices[8] = {
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 1.0, 1.0, 1.0),  // white
    color4(0.0, 1.0, 1.0, 1.0)   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// Shader transformation matrices
mat4  model, view;
GLuint Model, View, Projection;
bool topView;
int phase;
GLfloat old_base_angle, old_lower_arm_angle, old_upper_arm_angle;
GLfloat new_base_angle, new_lower_arm_angle, new_upper_arm_angle;
GLfloat restore_base_angle, restore_lower_arm_angle, restore_upper_arm_angle;
GLfloat base_angle, lower_arm_angle, upper_arm_angle;

// Array of rotation angles (in degrees) for each rotation axis
enum { 
	Base = 0, 
	LowerArm = 1, 
	UpperArm = 2, 
	NumAngles = 3 
};
int Axis = Base;
GLfloat Theta[NumAngles] = { 0.0 };

// Menu option values
const int Quit = 4;


//----------------------------------------------------------------------------

int Index = 0;

void quad(int a, int b, int c, int d)
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

void colorsphere() 
{
	int sphereIndex = 0;

    for (float angle1 = 0; angle1 < 2 * Pi; angle1 += step) {
    	for (float angle2 = 0; angle2 < Pi; angle2 += step) {
    		if (sphereIndex >= NumSpherePoints) {
    			break;
    		}

    		spherePoints[sphereIndex] = vec4(R * cos(angle1) * sin(angle2), R * sin(angle1) * sin(angle2), R * cos(angle2), 1.0);
    		sphereColors[sphereIndex] = vec4(1.0, 0.0, 0.0, 1.0);
    		sphereIndex++;
    	}
    }
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void sphere()
{
	if (phase == 1) {
		//attached
		mat4 instance = Translate( 0.0, UPPER_ARM_HEIGHT, 0.0 ) * Translate( 0.0, R, 0.0 );
		glUniformMatrix4fv( Model, 1, GL_TRUE, model * instance);
	}

	else {
		//not attached
		mat4 instance = Translate(center.x, center.y, center.z);
		glUniformMatrix4fv(Model, 1, GL_TRUE, instance);
	}
	
	glBindVertexArray(vaoSphere);
	glDrawArrays(GL_POINTS, 0, NumSpherePoints);
}


void base()
{
    mat4 instance = (Translate(0.0, 0.5 * BASE_HEIGHT, 0.0) * Scale(BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH));
    glUniformMatrix4fv(Model, 1, GL_TRUE, model * instance);
    glBindVertexArray(vaoRobot);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) * Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH));
    glUniformMatrix4fv(Model, 1, GL_TRUE, model * instance);
    glBindVertexArray(vaoRobot);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = (Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) * Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH));
    glUniformMatrix4fv(Model, 1, GL_TRUE, model * instance);
    glBindVertexArray(vaoRobot);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Accumulate ModelView Matrix as we traverse the tree
    if (topView) {
    	view = LookAt(vec4(1, 12, 4, 1), vec4(1, 0, 4, 1), vec4(0, 0, -1, 0));
    	glUniformMatrix4fv(View, 1, GL_TRUE, view);
    }

    else {
    	view = mat4(1.0);
    	glUniformMatrix4fv(View, 1, GL_TRUE, view);
    }

    if (phase == 1) {
    	model = RotateY(Theta[Base]);
    	base();
   		model *= (Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LowerArm]));
   		lower_arm();
		model *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0) * RotateZ(Theta[UpperArm]));
    	upper_arm();
    	sphere();
    }

    else {
    	sphere();
    	model = RotateY(Theta[Base]);
    	base();
   		model *= (Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LowerArm]));
   		lower_arm();
   		model *= (Translate(0.0, LOWER_ARM_HEIGHT, 0.0) * RotateZ(Theta[UpperArm]));
    	upper_arm();
	}

    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void initSphere() 
{
	colorsphere();

	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);

	GLuint sphereBuffer;
	glGenBuffers(1, &sphereBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors), sphereColors);
    
    // Load shaders and use the resulting shader program
    
    GLuint sphereProgram = InitShader("vshader81.glsl", "fshader81.glsl");
    glUseProgram(sphereProgram);
    
    GLuint vSpherePosition = glGetAttribLocation(sphereProgram, "vPosition");
    glEnableVertexAttribArray(vSpherePosition);
    glVertexAttribPointer(vSpherePosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vSphereColor = glGetAttribLocation( sphereProgram, "vColor" );
    glEnableVertexAttribArray(vSphereColor);
    glVertexAttribPointer(vSphereColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(spherePoints)));
    
    Model = glGetUniformLocation(sphereProgram, "Model");
    View = glGetUniformLocation(sphereProgram, "View");
    Projection = glGetUniformLocation(sphereProgram, "Projection");
}

void init(void)
{
    colorcube();
    topView = false;
    phase = 0;
    
    // Create a vertex array object
    glGenVertexArrays(1, &vaoRobot);
    glBindVertexArray(vaoRobot);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader81.glsl", "fshader81.glsl");
    glUseProgram(program);
    
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    Model = glGetUniformLocation(program, "Model");
    View = glGetUniformLocation(program, "View");
    Projection = glGetUniformLocation(program, "Projection");

    glEnable(GL_DEPTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.0, 0.0, 0.0, 1.0); 
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
	// Incrase the joint angle
		Theta[Axis] += 5.0;
		if (Theta[Axis] > 360.0) 
			{ 
				Theta[Axis] -= 360.0; 
			}
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
	// Decrase the joint angle
		Theta[Axis] -= 5.0;
		if (Theta[Axis] < 0.0) { 
			Theta[Axis] += 360.0; 
		}
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu(int option)
{
    if (option == Quit) {
		exit(EXIT_SUCCESS);
    }
    else {
        printf("%i\n",option);
		Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width)/height;

    if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
    }

    else {
		bottom /= aspect;
		top /= aspect;
    }

    mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    model = mat4(1.0);  // An Identity matrix
    view = mat4(1.0);
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit(EXIT_SUCCESS);
	    break;
	case 't': case 'T':
		//glMatrixMode(GL_MODELVIEW);
		topView = true;
	    printf("Top\n");
	    break;
	case 's': case 'S':
		//glMatrixMode(GL_MODELVIEW);
		topView = false;
	    printf("Side\n");
	    break;
	case '0':
		Axis = 0;
		break;
	case '1':
		Axis = 1;
		break;
	case '2':
		Axis = 2;
		break;
    }

    glutPostRedisplay();
}

bool equalToZero(GLfloat x) 
{
	if (x > -0.01 && x < 0.01) {
		return true;
	}
	else {
		return false;
	}
}

bool isAbove(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1)
{
	// Return true if (x0, y0) is above the line linking (x1, y1) and (0, 0)
	// before calling this function, make sure x1 > 0
	GLfloat k = y1 / x1; // k is the slope, y = kx

	if (equalToZero(k * x0 - y0) || (y0 < k * x0)) {
		return false;
	} 
	else {
		return true;
	}

}

void calculate(GLfloat x, GLfloat y, GLfloat z) 
{
	// calculate the degrees (not radian) that the base, lower arm and upper arm should rotate from the original position
	// 0 <= degrees < 360
	// calculate the degrees the base should rotate
	if (x <= 0 && equalToZero(z) || equalToZero(x) && equalToZero(z)) {
		base_angle = 0.0;
	}
	else if (x > 0 && equalToZero(z)) {
		base_angle = 180.0;
	}
	else if (equalToZero(x) && z < 0) {
		base_angle = 270.0;
	}
	else if (equalToZero(x) && z > 0) {
		base_angle = 90.0;
	}
	else if (x < 0 && z < 0) {
		base_angle = atan(z / (-1.0 * x)) * 180.0 / Pi + 360;
	}
	else if (x < 0 && z > 0) {
		base_angle = atan(z / (-1.0 * x)) * 180.0 / Pi;
	}
	else if (x > 0 && z < 0 || x > 0 && z > 0) {
		base_angle = atan(z / (-1.0 * x)) * 180.0 / Pi + 180;
	}

	// calculate the degrees the lower arm should rotate
	GLfloat x0 = sqrt(x * x + z * z);
	GLfloat y0 = y - 2;
	GLfloat a = 4 * (x0 * x0 + y0 * y0); //a = 4(x0^2 + y0 ^2)
	GLfloat b = -4 * y0 * (x0 * x0 + y0 * y0); //b = -4y0(x^2 + y^2)
	GLfloat c = (x0 * x0 + y0 * y0) * (x0 * x0 + y0 * y0) - 100 * x0 * x0; //c = (x0^2 + y0^2)^2 - 100x0^2

	GLfloat x1, y1;

	if (equalToZero(x0)) {
		if (equalToZero(y0)) {
			lower_arm_angle = 0.0;
			upper_arm_angle = 180.0;
			return;
		}
		else if (equalToZero(y0 - 10.0)) {
			lower_arm_angle = 0.0;
			upper_arm_angle = 0.0;
			return;
		}
		else if (equalToZero(y0 + 10.0)) {
			lower_arm_angle = 180.0;
			upper_arm_angle = 0.0;
			return;
		}
		else {
			y1 = y0 / 2.0;
			if (25 - (y0 * y0) / 4.0 < 0.0) {
				x1 = 0.0;
			}
			else{
				x1 = sqrt(25 - (y0 * y0) / 4.0);
			}
		}
	}

	else {
		if (b * b - 4 * a * c < 0.0) {
			printf("I cannot reach the sphere!\n");
			y1 = -b / (2 * a);
		}
		else {
			y1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		}

		x1 = (-2 * y0 * y1 + x0 * x0 + y0 * y0) / (2 * x0);
	}

	if (!equalToZero(x1) && x1 < 0.0) { //make sure that x1 >= 0
		if (b * b - 4 * a * c < 0.0) {
			printf("I cannot reach the sphere!\n");
			y1 = -b / (2 * a);
		}
		else {
			y1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
		}
		x1 = (-2 * y0 * y1 + x0 * x0 + y0 * y0) / (2 * x0);
	}

	if (equalToZero(x1) && y1 > 0.0) {
		lower_arm_angle = 0.0;
	} 
	else if (equalToZero(x1) && y1 < 0.0) {
		lower_arm_angle = 180.0;
	}
	else {
		lower_arm_angle = 90.0 - atan(y1 / x1) * 180.0 / Pi;
	}

	// calculate the degrees the upper arm should rotate
	GLfloat cos_theta = (x1 * (x0 - x1) + y1 * (y0 - y1)) / (sqrt(x1 * x1 + y1 * y1) * sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1)));
	if (equalToZero(cos_theta - 1.0)) {
		upper_arm_angle = 0.0;
	}
	else if (equalToZero(cos_theta + 1.0)) {
		upper_arm_angle = 180.0;
	}
	else if (equalToZero(lower_arm_angle - 180.0)) {
		upper_arm_angle = 360.0 - acos(cos_theta) * 180.0 / Pi;
	}
	else if (equalToZero(lower_arm_angle) || !isAbove(x0, y0, x1, y1)) {
		upper_arm_angle = acos(cos_theta) * 180.0 / Pi;
	}
	else {
		upper_arm_angle = 360.0 - acos(cos_theta) * 180.0 / Pi;
	}
}

void move(int axis, GLfloat angle)
{
	Theta[axis] += angle;
	if (Theta[axis] > 360.0) {
		Theta[axis] -= 360.0;
	}
	if (Theta[axis] < 0.0) {
		Theta[axis] += 360.0;
	}
}

void process(int)
{
	if (phase == 0) {
		if (old_base_angle > 1.0) {
			move(0, 1.0);
			old_base_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (old_base_angle > 0.0 && old_base_angle <= 1.0) {
			move(0, old_base_angle);
			old_base_angle -= old_base_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (old_lower_arm_angle > 1.0) {
			move(1, 1.0);
			old_lower_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (old_lower_arm_angle > 0.0 && old_lower_arm_angle <= 1.0) {
			move(1, old_lower_arm_angle);
			old_lower_arm_angle -= old_lower_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (old_upper_arm_angle > 1.0) {
			move(2, 1.0);
			old_upper_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (old_upper_arm_angle > 0.0 && old_upper_arm_angle <= 1.0) {
			move(2, old_upper_arm_angle);
			old_upper_arm_angle -= old_upper_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		if (old_base_angle == 0.0 && old_lower_arm_angle == 0.0 && old_upper_arm_angle == 0.0) {
			phase = 1;
		}
	}

	else if (phase == 1) {
		if (new_base_angle > 1.0) {
			move(0, 1.0);
			new_base_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (new_base_angle > 0.0 && new_base_angle <= 1.0) {
			move(0, new_base_angle);
			new_base_angle -= new_base_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (new_lower_arm_angle > 1.0) {
			move(1, 1.0);
			new_lower_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (new_lower_arm_angle > 0.0 && new_lower_arm_angle <= 1.0) {
			move(1, new_lower_arm_angle);
			new_lower_arm_angle -= new_lower_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (new_upper_arm_angle > 1.0) {
			move(2, 1.0);
			new_upper_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (new_upper_arm_angle > 0.0 && new_upper_arm_angle <= 1.0) {
			move(2, new_upper_arm_angle);
			new_upper_arm_angle -= new_upper_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		if (new_base_angle == 0.0 && new_lower_arm_angle == 0.0 && new_upper_arm_angle == 0.0) {
			phase = 2;
			center.x = new_x;
			center.y = new_y;
			center.z = new_z;
		}
	}

	else {
		if (restore_base_angle > 1.0) {
			move(0, -1.0);
			restore_base_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (restore_base_angle > 0.0 && restore_base_angle <= 1.0) {
			move(0, -restore_base_angle);
			restore_base_angle -= restore_base_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (restore_lower_arm_angle > 1.0) {
			move(1, -1.0);
			restore_lower_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (restore_lower_arm_angle > 0.0 && restore_lower_arm_angle <= 1.0) {
			move(1, -restore_lower_arm_angle);
			restore_lower_arm_angle -= restore_lower_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (restore_upper_arm_angle > 1.0) {
			move(2, -1.0);
			restore_upper_arm_angle -= 1.0;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}

		else if (restore_upper_arm_angle > 0.0 && restore_upper_arm_angle <= 1.0) {
			move(2, -restore_upper_arm_angle);
			restore_upper_arm_angle -= restore_upper_arm_angle;
			glutPostRedisplay();
			glutTimerFunc(50, process, 0);
		}
	}
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("robot");

    // Iff you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE; 
    glewInit();
    
    init();
    initSphere();
    old_x = atof(argv[1]);
    old_y = atof(argv[2]);
    old_z = atof(argv[3]);
    new_x = atof(argv[4]);
    new_y = atof(argv[5]);
    new_z = atof(argv[6]);
    center = vec4(old_x, old_y, old_z, 1.0);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glutCreateMenu(menu);
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry("base", Base);
    glutAddMenuEntry("lower arm", LowerArm);
    glutAddMenuEntry("upper arm", UpperArm);
    glutAddMenuEntry("quit", Quit);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    calculate(old_x, old_y, old_z);
    old_base_angle = base_angle;
    old_lower_arm_angle = lower_arm_angle;
    old_upper_arm_angle = upper_arm_angle;
    calculate(new_x, new_y, new_z);
    new_base_angle = base_angle - old_base_angle;
    new_lower_arm_angle = lower_arm_angle - old_lower_arm_angle;
    new_upper_arm_angle = upper_arm_angle - old_upper_arm_angle;
    if (new_base_angle < 0.0) {
    	new_base_angle += 360.0;
    }

    if (new_base_angle >= 360.0) {
    	new_base_angle -= 360.0;
    }

    if (new_lower_arm_angle < 0.0) {
    	new_lower_arm_angle += 360.0;
    }

    if (new_lower_arm_angle >= 360.0) {
    	new_lower_arm_angle -= 360.0;
    }

    if (new_upper_arm_angle < 0.0) {
    	new_upper_arm_angle += 360.0;
    }

    if (new_upper_arm_angle >= 360.0) {
    	new_upper_arm_angle -= 360.0;
    }

    restore_base_angle = base_angle;
    restore_lower_arm_angle = lower_arm_angle;
    restore_upper_arm_angle = upper_arm_angle;

    glutTimerFunc(50, process, 0);

    glutMainLoop();
    return 0;
}
