#ifndef PRIOTYPES_H
#define PRIOTYPES_H

namespace PrioEngine
{
	// Defines 3 floats for use with 3D coordinates.
	typedef struct Coords
	{
		float x;
		float y;
		float z;
	};

	// Defines 3 floats for describing red, green and blue values.
	typedef struct RGB
	{
		float r;
		float g;
		float b;
	};

	// Defines 4 floats for describing red, green, blue and alpha values.
	typedef struct RGBA
	{
		float r;
		float g;
		float b;
		float a;
	};

	// An enumeration of types of shapes which we should be able to create.
	enum Primitives
	{
		triangle,
		square,
		cube,
		sphere
	};

	namespace Cube
	{
		// Store the number of vertices for a cube.
		const int kNumOfVertices = 8;
		// Store the number of indices for a cube.
		const int kNumOfIndices = 36;

		// Store the position of each vertex.
		const Coords kCubeVerticesCoords[kNumOfVertices] = {
			{-0.5f, 0.5f, -0.5f},
			{0.5f, 0.5f, -0.5f},
			{-0.5f, -0.5f, -0.5f},

			{0.5f, -0.5f, -0.5f},

			{-0.5f, 0.5f, 0.5f},

			{0.5f, 0.5f, 0.5f},

			{-0.5f, -0.5f, 0.5f},

			{0.5f, -0.5f, 0.5f}
		};

		// Indice array which will describe which vertex's connect to make triangles.
		const unsigned long indices[kNumOfIndices] =
		{
			0, 1, 2,
			2, 1, 3,
			4, 0, 6,
			6, 0, 2,
			7, 5, 6,
			6, 5, 4,
			3, 1, 7,
			7, 1, 5,
			4, 5, 0,
			0, 5, 1,
			3, 7, 2,
			2, 7, 6
		};
	}

	namespace Triangle
	{
		// Define how many points (vertices) there will be in a triangle.
		const int kNumOfVertices = 3;
		// Define how many points will need to be connected to create triangles to form this shape.
		const int kNumOfIndices = 3;

		// The position of the points.
		const Coords vertices[kNumOfVertices] =
		{
			{-0.5f, -0.5f, 0.0f},
			{0.0f, 0.5f, 0.0f},
			{0.5f, -0.5f, 0.0f}
		};

		// The points which need to be connected together to create triangles.
		const unsigned long indices[kNumOfIndices] =
		{
			1, 2, 0
		};

	}

	// Store all default colours that can be used in the engine in this namespace so they can be accessed by PrioEngine::Colours::
	namespace Colours
	{
		// Define the colour to be used as diffuse lighting.
		const RGBA white = { 1.0f, 1.0f, 1.0f, 1.0f};
		const RGBA red = { 1.0f, 0.0f, 0.0f, 1.0f };
		const RGBA green = { 0.0f, 1.0f, 0.0f, 1.0f };
		const RGBA blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	// Defines the types of vertex arrays we have, these should reflect the structures in the vertex shader.
	enum VertexType
	{
		Colour,
		Texture,
		Diffuse
	};
}
#endif