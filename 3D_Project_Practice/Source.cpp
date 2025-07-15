#include<GLEW/glew.h>
#include<GLFW/glfw3.h>


#include<glm/glm/glm.hpp>
#include<glm/glm/gtc/matrix_transform.hpp>
#include<glm/glm/gtc/type_ptr.hpp>

#include<SOIL2/SOIL2.h>

#include<iostream>
#include<vector>



//initialize functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void initCamera();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float camX, camY, camZ, triLocX, triLocY, triLocZ;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;

//initialize timing variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float cameraSpeedMultiplier = 1.0f;

//Sphere Settings
const int sectorCount = 12;
const int stackCount = 12;
const float radius = 1.0f;
const float M_PI = 3.14159f;

//Cylinder for Lamp Top Settings
float coneRadius = 1.5f, height = 3.0f, segments = 12;


struct Vertex {
	float x, y, z;
	float s, t;
	float nx, ny, nz;
};


bool isPerspective = true;

//Camera Vectors
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

//Lighting
const int NUM_LIGHTS = 2;

glm::vec3 lightPositions[] = {
	glm::vec3(0.0f, 2.0f, 15.0f),
	glm::vec3(5.0f, 2.0f, 10.0f)
};
glm::vec3 lightColors[] = {
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f)
};
glm::vec3 objectColors[] = {
	glm::vec3(0.41f, 0.41f, 0.40f),
	glm::vec3(0.52f, 0.43f, 0.34f)
};

// Create and Compile Shaders
static GLuint CompileShader(const std::string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const std::string& vertexShader, const std::string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}




glm::float32 triRotations[] = {
	0.0f, 30.0f, 60.0f, 90.0f, 120.0f, 150.0f,
	180.0f, 210.0f, 240.0f, 270.0f, 300.0f, 330.0f

};


int main() {

	//initialize and configure glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Nate Lemons", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//Callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//load all OpenGL function pointers with glew
	if (glewInit() != GLEW_OK) {
		std::cout << "Error!" << std::endl;
	}

	//vertex information
	GLfloat candleVertices[] = {

		// Vertices             //Texture	  //Normals
		0.0f, 0.0f, 0.0f,       0.5f, 1.0f,	  0.0f, 0.0f, -1.0f,
		0.866f, 0.5f, 0.0f,     1.0f, 0.5f,	  0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,       0.0f, 0.5f,	  0.0f, 0.0f, -1.0f,

		0.866f, 0.5f, 0.0f,     1.0f, 0.5f,   0.0f, 0.0f, -1.0f,
		0.866f, 0.5f, -2.0f,    1.0f, 0.0f,   0.866f, 0.5f, 0.0f,
		1.0f, 0.0f, 0.0f,       0.0f, 0.5f,   0.0f, 0.0f, -1.0f,

		1.0f, 0.0f, 0.0f,       0.0f, 0.5f,	  0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -2.0f,      0.0f, 0.0f,	  0.866f, 0.5f, 0.0f,
		0.866f, 0.5f, -2.0f,    1.0f, 0.0f,	  0.866f, 0.5f, 0.0f

	};



	GLfloat counterVertices[] = {

		-5.0f, -5.0f, -5.0f,   0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		 5.0f, -5.0f, -5.0f,   1.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		 5.0f, -5.0f,  5.0f,   1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		 5.0f, -5.0f,  5.0f,   1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-5.0f, -5.0f,  5.0f,   0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-5.0f, -5.0f, -5.0f,   0.0f, 1.0f,	0.0f, 1.0f, 0.0f
	};

	GLfloat vaseVertices[] = {

		0.866f, 0.5f, 0.0f,   1.0f, 0.5f,	0.0f, 0.0f, -1.0f,
		0.866f, 0.5f, -2.0f,  1.0f, 0.0f,	0.866f, 0.5f, 0.0f,
		1.0f, 0.0f, 0.0f,     0.0f, 0.5f,	0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,     0.0f, 0.5f,	0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -2.0f,    0.0f, 0.0f,	0.866f, 0.5f, 0.0f,
		0.866f, 0.5f, -2.0f,  1.0f, 0.0f,	0.866f, 0.5f, 0.0f


	};

	GLfloat lampTopVertices[] = {

		0.0f, 0.0f, -2.0f,    0.5f, 1.0f,	0.0f, 0.0f, -1.0f,
		0.866f, 0.5f, -2.0f,  1.0f, 0.5f,	0.866f, 0.5f, 0.0f,
		1.0f, 0.0f, -2.0f,    0.0f, 0.5f,	0.866f, 0.5f, 0.0f,

		0.433f, 0.25f, 0.0f,  1.0f, 0.5f,	0.0f, 0.0f, -0.5f,
		0.866f, 0.5f, -2.0f,  1.0f, 0.0f,	0.866f, 0.5f, 0.0f,
		0.5f, 0.0f, 0.0f,     0.0f, 0.5f,	0.0f, 0.0f, -0.5f,


		0.866f, 0.5f, -2.0f,  1.0f, 0.0f,	0.866f, 0.5f, 0.0f,
		0.5f, 0.0f, 0.0f,     0.0f, 0.5f,	0.0f, 0.0f, -0.5f,
		1.0f, 0.0f, -2.0f,    0.0f, 0.0f,	0.866f, 0.5f, 0.0f,
	
	};




	GLfloat mushroomBaseVertices[] = {

		// Front
		-0.1f, -0.5f, -0.1f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.1f,  0.5f, -0.1f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.1f, -0.5f, -0.1f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// Back
		-0.1f, -0.5f, 0.1f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,		0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-0.1f,  0.5f, 0.1f, 0.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-0.1f, -0.5f, 0.1f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f,

		// Left
		-0.1f,  0.5f,  0.1f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.1f,  0.5f, -0.1f, 1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.1f, -0.5f, -0.1f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.1f, -0.5f, -0.1f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.1f, -0.5f,  0.1f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.1f,  0.5f,  0.1f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// Right
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		0.5f, -0.5f,  0.5f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,

		// Bottom
		-0.1f, -0.5f, -0.1f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-0.1f, -0.5f,  0.1f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-0.1f, -0.5f, -0.1f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,

		// Top
		-0.1f,  0.5f, -0.1f, 0.0f,  1.0f,	0.0f, -1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f,  1.0f,	0.0f, -1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f,	0.0f, -1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f,	0.0f, -1.0f, 0.0f,
		-0.1f,  0.5f,  0.1f, 0.0f,  0.0f,	0.0f, -1.0f, 0.0f,
		-0.1f,  0.5f, -0.1f, 0.0f,  1.0f,	0.0f, -1.0f, 0.0f
	};



	std::vector<Vertex> sphereVertices;
	std::vector<unsigned int> sphereIndices;

	// Generate sphere vertices and indices
	for (int i = 0; i <= stackCount; ++i) {
		float stackAngle = M_PI / 2 - i * M_PI / stackCount;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j) {
			float sectorAngle = j * 2 * M_PI / sectorCount;
			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);

			// Vertex
			Vertex vertex;
			vertex.x = x;
			vertex.y = y;
			vertex.z = z;

			// Texture coordinates
			vertex.s = (float)j / sectorCount;
			vertex.t = (float)i / stackCount;

			// Normal vectors (same as vertex coordinates for a sphere)
			vertex.nx = x;
			vertex.ny = y;
			vertex.nz = z;

			sphereVertices.push_back(vertex);

			// Indices
			if (i > 0 && j > 0) {
				int first = (i - 1) * (sectorCount + 1) + j - 1;
				int second = i * (sectorCount + 1) + j - 1;
				int third = i * (sectorCount + 1) + j;
				int fourth = (i - 1) * (sectorCount + 1) + j;

				// First triangle
				sphereIndices.push_back(first);
				sphereIndices.push_back(second);
				sphereIndices.push_back(third);

				// Second triangle
				sphereIndices.push_back(third);
				sphereIndices.push_back(fourth);
				sphereIndices.push_back(first);
			}
		}
	}

	std::vector<Vertex> capVertices;
	std::vector<unsigned int> capIndices;

	// Generate sphere vertices and indices for the upper hemisphere
	for (int i = 0; i <= stackCount / 2; ++i) { // Adjusted loop range for half of the stacks
		float stackAngle = M_PI / 2 - i * M_PI / stackCount;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j) {
			float sectorAngle = j * 2 * M_PI / sectorCount;
			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);

			// Vertex
			Vertex capVertex;
			capVertex.x = x;
			capVertex.y = y;
			capVertex.z = z;

			// Texture coordinates
			capVertex.s = (float)j / sectorCount;
			capVertex.t = (float)i / (stackCount / 2); // Adjusted texture coordinate range

			// Normal vectors (same as vertex coordinates for a sphere)
			capVertex.nx = x;
			capVertex.ny = y;
			capVertex.nz = z;

			capVertices.push_back(capVertex);

			// Indices
			if (i > 0 && j > 0) {
				int first = (i - 1) * (sectorCount + 1) + j - 1;
				int second = i * (sectorCount + 1) + j - 1;
				int third = i * (sectorCount + 1) + j;
				int fourth = (i - 1) * (sectorCount + 1) + j;

				// First triangle
				capIndices.push_back(first);
				capIndices.push_back(second);
				capIndices.push_back(third);

				// Second triangle
				capIndices.push_back(third);
				capIndices.push_back(fourth);
				capIndices.push_back(first);
			}
		}
	}


	
	//allow OpenGL to test for depth of vertices/indices
	glEnable(GL_DEPTH_TEST);

	//Variables for object rendering
	unsigned int candleVBO, candleVAO, counterVBO, counterVAO, vaseVBO, vaseVAO, sphereVBO, sphereVAO, sphereEBO;
	unsigned int  lampSphereVBO, lampSphereVAO, lampSphereEBO, lampMiddleVBO, lampMiddleVAO, lampTopVBO, lampTopVAO, lampTopEBO;
	unsigned int mushroomBaseVAO, mushroomBaseVBO, mushStemOneVAO, mushStemOneVBO, mushStemTwoVAO, mushStemTwoVBO, mushCapOneVAO, 
		mushCapOneVBO, mushCapOneEBO, mushCapTwoVAO, mushCapTwoVBO, mushCapTwoEBO;
	unsigned int lampVAO, lampVBO;

	//Information for candle 
	glGenVertexArrays(1, &candleVAO);
	glGenBuffers(1, &candleVBO);
	glBindVertexArray(candleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, candleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(candleVertices), candleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);


	//information for counter 
	glGenVertexArrays(1, &counterVAO);
	glGenBuffers(1, &counterVBO);
	glBindVertexArray(counterVAO);
	glBindBuffer(GL_ARRAY_BUFFER, counterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(counterVertices), counterVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);



	//information for vase cylinder
	glGenVertexArrays(1, &vaseVAO);
	glGenBuffers(1, &vaseVBO);
	glBindVertexArray(vaseVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vaseVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vaseVertices), vaseVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);



	//information for vase sphere
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, s));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	//information for lamp sphere (Base)
	glGenVertexArrays(1, &lampSphereVAO);
	glGenBuffers(1, &lampSphereVBO);
	glGenBuffers(1, &lampSphereEBO);
	glBindVertexArray(lampSphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampSphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampSphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, s));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//lamp (middle)
	glGenVertexArrays(1, &lampMiddleVAO);
	glGenBuffers(1, &lampMiddleVBO);
	glBindVertexArray(lampMiddleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampMiddleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vaseVertices), vaseVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	//lamp (top)
	glGenVertexArrays(1, &lampTopVAO);
	glGenBuffers(1, &lampTopVBO);
	glBindVertexArray(lampTopVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampTopVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampTopVertices), lampTopVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	
	//mushroom sculpture information (base)
	glGenVertexArrays(1, &mushroomBaseVAO);
	glGenBuffers(1, &mushroomBaseVBO);
	glBindVertexArray(mushroomBaseVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mushroomBaseVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mushroomBaseVertices), mushroomBaseVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	//mushroom sculpture stem one
	glGenVertexArrays(1, &mushStemOneVAO);
	glGenBuffers(1, &mushStemOneVBO);
	glBindVertexArray(mushStemOneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mushStemOneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(candleVertices), candleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	//mushroom sculpture stem two
	glGenVertexArrays(1, &mushStemTwoVAO);
	glGenBuffers(1, &mushStemTwoVBO);
	glBindVertexArray(mushStemTwoVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mushStemTwoVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(candleVertices), candleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	//Mush cap one
	glGenVertexArrays(1, &mushCapOneVAO);
	glGenBuffers(1, &mushCapOneVBO);
	glGenBuffers(1, &mushCapOneEBO);
	glBindVertexArray(mushCapOneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mushCapOneVBO);
	glBufferData(GL_ARRAY_BUFFER, capVertices.size() * sizeof(Vertex), capVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mushCapOneEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, capIndices.size() * sizeof(unsigned int), capIndices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, s));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Mush cap two
	glGenVertexArrays(1, &mushCapTwoVAO);
	glGenBuffers(1, &mushCapTwoVBO);
	glGenBuffers(1, &mushCapTwoEBO);
	glBindVertexArray(mushCapTwoVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mushCapTwoVBO);
	glBufferData(GL_ARRAY_BUFFER, capVertices.size() * sizeof(Vertex), capVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mushCapTwoEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, capIndices.size() * sizeof(unsigned int), capIndices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, s));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//Load textures
	GLint candleTexWidth, candleTexHeight, floorTexWidth, floorTexHeight, vaseTexWidth, vaseTexHeight, lampTexWidth, lampTexHeight, mushTexWidth, mushTexHeight;
	unsigned char* candleImage = SOIL_load_image("candle.png", &candleTexWidth, &candleTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* floorImage = SOIL_load_image("counter.jpg", &floorTexWidth, &floorTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* vaseImage = SOIL_load_image("vase2.png", &vaseTexWidth, &vaseTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* lampImage = SOIL_load_image("brownGlass.png", &lampTexWidth, &lampTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* mushImage = SOIL_load_image("wood.png", &mushTexWidth, &mushTexHeight, 0, SOIL_LOAD_RGB);

	//Generate Candle Texture
	GLuint candleTexture;
	glGenTextures(1, &candleTexture);
	glBindTexture(GL_TEXTURE_2D, candleTexture);
	GLfloat maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, candleTexWidth, candleTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, candleImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(candleImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Generate Floor texture
	GLuint floorTexture;
	glGenTextures(1, &floorTexture);
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, floorTexWidth, floorTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, floorImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(floorImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Vase Texture
	GLuint vaseTexture;
	glGenTextures(1, &vaseTexture);
	glBindTexture(GL_TEXTURE_2D, vaseTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vaseTexWidth, vaseTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, vaseImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(vaseImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Lamp Texture
	GLuint lampTexture;
	glGenTextures(1, &lampTexture);
	glBindTexture(GL_TEXTURE_2D, lampTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lampTexWidth, lampTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, lampImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(lampImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Mushroom Sculpture Texture
	GLuint mushTexture;
	glGenTextures(1, &mushTexture);
	glBindTexture(GL_TEXTURE_2D, mushTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mushTexWidth, mushTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mushImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(mushImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Vertex shader source code
	std::string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec2 texCoord;"
		"layout(location = 2) in vec3 normal;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 fragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oTexCoord = texCoord;"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"fragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	std::string fragmentShaderSource =
		"#version 330 core\n"
		"#define NUM_LIGHTS 2\n"
		"in vec3 fragPos;"
		"in vec3 oNormal;"
		"in vec2 oTexCoord;"
		"out vec4 fragColor;"
		// Material Properties
		"uniform vec3 objectColors[NUM_LIGHTS];" // Array of object colors
		"uniform sampler2D myTexture;"
		// Light Properties
		"uniform vec3 lightPositions[NUM_LIGHTS];"
		"uniform vec3 lightColors[NUM_LIGHTS];"
		"void main()\n"
		"{\n"
		// Ambient Light
		"float ambientStrength = 0.3;"
		"vec3 ambient = vec3(0.0);"
		"for (int i = 0; i < NUM_LIGHTS; ++i) {" // Loop through lights
		"   ambient += ambientStrength * objectColors[i];" // Accumulate ambient light for each object color
		"}"
		// Diffuse Light
		"vec3 norm = normalize(oNormal);"
		"vec3 result = vec3(0.0);"
		"for (int i = 0; i < NUM_LIGHTS; ++i) {"
		"   vec3 lightDir = normalize(lightPositions[i] - fragPos);"
		"   float diff = max(dot(norm, lightDir), 0.0);"
		"   vec3 diffuse = diff * lightColors[i] * objectColors[i];" // Multiply diffuse light by object color
		"   result += diffuse;"
		"}"
		// Combine Ambient and Diffuse Lights
		"vec3 lighting = ambient + result;"
		"vec3 finalColor = texture(myTexture, oTexCoord).rgb * lighting;"
		"fragColor = vec4(finalColor, 1.0);"
		"}\n";

	//Initialize shader progrogram
	unsigned int shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	

	//render loop
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//input
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		//define and initialize view and projection matrices
		glm::mat4 view;
		glm::mat4 projection;

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);//set where camera looks

		if (isPerspective) {
			projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		}
		else {
			projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.f);
		}

		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

		//Get Light and Object Color and Light Position location
		unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColors");
		unsigned int lightPositionsLoc = glGetUniformLocation(shaderProgram, "lightPositions");
		unsigned int lightColorsLoc = glGetUniformLocation(shaderProgram, "lightColors");
		unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		glUniform3fv(objectColorLoc, NUM_LIGHTS, glm::value_ptr(objectColors[0]));
		glUniform3fv(lightPositionsLoc, NUM_LIGHTS, glm::value_ptr(lightPositions[0]));
		glUniform3fv(lightColorsLoc, NUM_LIGHTS, glm::value_ptr(lightColors[0]));

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//Render Candle Cylinder
		glBindTexture(GL_TEXTURE_2D, candleTexture);
		glBindVertexArray(candleVAO);

			for (int i = 0; i < 12; i++) {
				glm::mat4 candleModel;

				candleModel = glm::translate(candleModel, glm::vec3(-1.0f, -0.99f, 3.5f));
				candleModel = glm::scale(candleModel, glm::vec3(0.8f, 0.8f, 0.8f));
				candleModel = glm::rotate(candleModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				candleModel = glm::rotate(candleModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(candleModel));

				glDrawArrays(GL_TRIANGLES, 0, 9);
			}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//Render plane 
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glBindVertexArray(counterVAO);
			glm::mat4 counterModel;
			counterModel = glm::mat4(1.0);
			counterModel = glm::translate(counterModel, glm::vec3(0.0f, 4.0f, 0.0f));
			modelLoc = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(counterModel));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);



		//Render vase top
		glBindTexture(GL_TEXTURE_2D, vaseTexture);
		glBindVertexArray(vaseVAO);
			for (int i = 0; i < 12; i++) {
				glm::mat4 vaseCylinderModel;

				vaseCylinderModel = glm::translate(vaseCylinderModel, glm::vec3(2.7f, 0.2f, 2.5f));
				vaseCylinderModel = glm::scale(vaseCylinderModel, glm::vec3(0.4f, 0.55f, 0.4f));
				vaseCylinderModel = glm::rotate(vaseCylinderModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				vaseCylinderModel = glm::rotate(vaseCylinderModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(vaseCylinderModel));

				glDrawArrays(GL_TRIANGLES, 0, 9);
			}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		// Render vase base
		glBindTexture(GL_TEXTURE_2D, vaseTexture);
		glBindVertexArray(sphereVAO);
			glm::mat4 vaseSphereModel = glm::mat4(1.0f);
			vaseSphereModel = glm::translate(vaseSphereModel, glm::vec3(2.7f, -0.3f, 2.5f));
			vaseSphereModel = glm::rotate(vaseSphereModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			vaseSphereModel = glm::scale(vaseSphereModel, glm::vec3(0.8f, 0.8f, 0.8f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(vaseSphereModel));
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		
		
		//Render Lamp Base
		glBindVertexArray(lampSphereVAO);
		glBindTexture(GL_TEXTURE_2D, lampTexture);
			glm::mat4 lampSphereModel = glm::mat4(1.0f);
			lampSphereModel = glm::translate(lampSphereModel, glm::vec3(1.7f,-0.1f, -1.9f));
			lampSphereModel = glm::rotate(lampSphereModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			lampSphereModel = glm::scale(lampSphereModel, glm::vec3(1.5f, 1.5f, 1.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lampSphereModel));
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//Render Lamp Middle
		glBindVertexArray(lampMiddleVAO);
		glBindTexture(GL_TEXTURE_2D, lampTexture);
			for (int i = 0; i < 12; i++) {
				glm::mat4 lampMiddleModel;

				lampMiddleModel = glm::translate(lampMiddleModel, glm::vec3(1.7f, 0.65f, -1.9f));
				lampMiddleModel = glm::scale(lampMiddleModel, glm::vec3(0.9f, 0.3f, 0.9f));
				lampMiddleModel = glm::rotate(lampMiddleModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				lampMiddleModel = glm::rotate(lampMiddleModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lampMiddleModel));

				glDrawArrays(GL_TRIANGLES, 0, 9);
			}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		
		//Lamp (Top)
		glBindVertexArray(lampTopVAO);
		glBindTexture(GL_TEXTURE_2D, lampTexture);
			for (int i = 0; i < 12; i++) {
				glm::mat4 lampTopModel;

				lampTopModel = glm::translate(lampTopModel, glm::vec3(1.7f, 2.75f, -1.9f));
				lampTopModel = glm::scale(lampTopModel, glm::vec3(1.3f, 0.8f, 1.3f));
				lampTopModel = glm::rotate(lampTopModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				lampTopModel = glm::rotate(lampTopModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lampTopModel));

				glDrawArrays(GL_TRIANGLES, 0, 9);

			}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);


		//Render Mushroom Sculpture (Base)
		glBindTexture(GL_TEXTURE_2D, mushTexture);
		glBindVertexArray(mushroomBaseVAO);

			glm::mat4 mushroomBaseModel;
			mushroomBaseModel = glm::translate(mushroomBaseModel, glm::vec3(-3.3f, -0.85f, 0.5f));
			mushroomBaseModel = glm::rotate(mushroomBaseModel, glm::radians(55.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			mushroomBaseModel = glm::scale(mushroomBaseModel, glm::vec3(5.0f, 0.3f, 2.5f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mushroomBaseModel));

			glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		//Mushroom Stem One
		glBindTexture(GL_TEXTURE_2D, mushTexture);
		glBindVertexArray(mushStemOneVAO);
		for (int i = 0; i < 12; i++) {
			glm::mat4 mushStemOneModel;

			mushStemOneModel = glm::translate(mushStemOneModel, glm::vec3(-3.15f, -0.7f, 0.3f));
			mushStemOneModel = glm::scale(mushStemOneModel, glm::vec3(0.10, 0.7f, 0.10f));
			mushStemOneModel = glm::rotate(mushStemOneModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			mushStemOneModel = glm::rotate(mushStemOneModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mushStemOneModel));

			glDrawArrays(GL_TRIANGLES, 0, 9);
		}

		glBindVertexArray(0);

		//mushroom stem two
		glBindTexture(GL_TEXTURE_2D, mushTexture);
		glBindVertexArray(mushStemTwoVAO);
		for (int i = 0; i < 12; i++) {
			glm::mat4 mushStemTwoModel;

			mushStemTwoModel = glm::translate(mushStemTwoModel, glm::vec3(-2.3f, -0.7f, -1.0f));
			mushStemTwoModel = glm::scale(mushStemTwoModel, glm::vec3(0.10f, 1.2f, 0.10f));
			mushStemTwoModel = glm::rotate(mushStemTwoModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			mushStemTwoModel = glm::rotate(mushStemTwoModel, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mushStemTwoModel));

			glDrawArrays(GL_TRIANGLES, 0, 9);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		//mushroom cap one
		glBindVertexArray(mushCapOneVAO);
		glBindTexture(GL_TEXTURE_2D, mushTexture);
			glm::mat4 mushCapOneModel;
			mushCapOneModel = glm::translate(mushCapOneModel, glm::vec3(-3.15f, 0.5f, 0.3f));
			mushCapOneModel = glm::rotate(mushCapOneModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			mushCapOneModel = glm::scale(mushCapOneModel, glm::vec3(0.4f, 0.4f, 0.4f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mushCapOneModel));
			glDrawElements(GL_TRIANGLES, capIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//mushroom cap two
		glBindVertexArray(mushCapTwoVAO);
		glBindTexture(GL_TEXTURE_2D, mushTexture);
			glm::mat4 mushCapTwoModel;
			mushCapTwoModel = glm::translate(mushCapTwoModel, glm::vec3(-2.3f, 1.3f, -1.0f));
			mushCapTwoModel = glm::rotate(mushCapTwoModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			mushCapTwoModel = glm::scale(mushCapTwoModel, glm::vec3(0.8f, 0.8f, 0.6f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mushCapTwoModel));
			glDrawElements(GL_TRIANGLES, capIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//swap buffers and poll IO events with glfw
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

//Define process input function
void processInput(GLFWwindow* window) {

	//query glfw whether relevant keys are pressed/released
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	//calculate camera movement speed
	float cameraSpeed = static_cast<float>(2.5 * deltaTime) * cameraSpeedMultiplier;

	//WASD Movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	//QE Movement (Up and Down)
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		cameraPos += cameraUp * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		cameraPos -= cameraUp * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		initCamera();
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		isPerspective = true;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		isPerspective = false;
	}



}

//function for changing viewport with window resize (by OS or user resize)
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

	glViewport(0, 0, width, height);
}

//Define Mouse Callback function
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	const float MAX_PITCH = 89.0f;
	const float MIN_PITCH = -89.0f;

	//Set initial place of cursor
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//update where cursor is
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	//set sensitivity of mouse
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > MAX_PITCH) {
		pitch = MAX_PITCH;
	}
	if (pitch < MIN_PITCH) {
		pitch = MIN_PITCH;
	}

	//refactor vector for cameraFront
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

}


//Define Scroll Calback function
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

	//Alter Camera Speed with scroll wheel
	const float scrollSensitivity = 0.1f;
	cameraSpeedMultiplier += yoffset * 0.1f;

	//Clamp camera speed
	const float minSpeed = 0.1f;
	const float maxSpeed = 5.0f;
	cameraSpeedMultiplier = glm::clamp(cameraSpeedMultiplier, minSpeed, maxSpeed);
}


void initCamera() {
	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
}
