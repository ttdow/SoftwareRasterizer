#include "tgaimage.h"
#include "Model.h"
#include "ModelLoader.h"

#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue	 = TGAColor(0, 0, 255, 255);
const TGAColor cyan  = TGAColor(0, 255, 255, 255);

const int width = 800;
const int height = 800;

void Line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;

	// Determine if line is taller than it is wide.
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		// Transpose image. Now the image is wider than it is tall.
		std::swap(x0, y0);
		std::swap(x1, y1);
		
		steep = true;
	}

	// Force lines to go from left to right.
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	// Eject repeated calculations from the loop.
	int dx = x1 - x0;
	int dy = y1 - y0;

	int derror = std::abs(dy) * 2;
	int error = 0;
	int y = y0;
	
	for (int x = x0; x <= x1; x++)
	{
		// Un-transpose if necessary.
		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}

		error += derror;
		if (error > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}

void Line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
	Line(p0.x, p0.y, p1.x, p1.y, image, color);
}

void Triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	// Return immediately if triangle is just a point.
	if (t0.y == t1.y && t0.y == t2.y)
	{
		return;
	}

	// Sort the vertices, t0, t1, t2 lower-to-upper.
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	// Determine total height of triangle.
	int totalHeight = t2.y - t0.y;

	for (int i = 0; i < totalHeight; i++)
	{
		bool secondHalf = i > t1.y - t0.y || t1.y == t0.y;
		int segmentHeight = secondHalf ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / totalHeight;
		float beta = (float)(i - (secondHalf ? t1.y - t0.y : 0)) / segmentHeight;

		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = secondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		
		if (A.x > B.x) std::swap(A, B);

		for (int j = A.x; j <= B.x; j++)
		{
			image.set(j, t0.y + i, color);
		}
	}
}

void Triangle(Vec3f t0, Vec3f t1, Vec3f t2, TGAImage& image, TGAColor color)
{
	// Convert from 3D world coords to 2D screen coords.
	Vec2i v0 = Vec2i((t0.x + 1.0f) * width / 2.0f, (t0.y + 1.0f) * height / 2.0f);
	Vec2i v1 = Vec2i((t1.x + 1.0f) * width / 2.0f, (t1.y + 1.0f) * height / 2.0f);
	Vec2i v2 = Vec2i((t2.x + 1.0f) * width / 2.0f, (t2.y + 1.0f) * height / 2.0f);

	Triangle(v0, v1, v2, image, color);
}

Vec3f Barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
	Vec3f D = Vec3f(C.x - A.x, B.x - A.x, A.x - P.x);
	Vec3f E = Vec3f(C.y - A.y, B.y - A.y, A.y - P.y);
	
	Vec3f u = D ^ E;

	if (std::abs(u.z) > 1e-2)
	{
		return Vec3f((1.0f - (u.x + u.y) / u.z), (u.y / u.z), (u.x / u.z));
	}

	return Vec3f(-1.0f, 1.0f, 1.0f);
}

void Triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color)
{
	// Initialize a 2D bounding box.
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	// Determine the maximum values for bounding boxes based on the output image dimensions.
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

	// Get the min x, y and max x, y values from the triangle's vertices to create the
	// bounding box around it.
	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pts[i].x));
		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));

		bboxmin.y = std::max(0.0f, std::min(bboxmin.y, pts[i].y));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}

	// Iterate through every pixel in the bounding box and call the Barycentric
	// function to determine if it is inside or outside the triangle.
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bcScreen = Barycentric(pts[0], pts[1], pts[2], P);
			if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0)
			{
				continue;
			}

			// Determine depth value of the current pixel.
			P.z = 0;
			P.z += pts[0].z * bcScreen.x;
			P.z += pts[1].z * bcScreen.y;
			P.z += pts[2].z * bcScreen.z;

			// Update the pixel if the current value is closer than any
			// previous value.
			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}

Vec3f World2Screen(Vec3f v)
{
	return Vec3f(int((v.x + 1.0f) * width / 2.0f + 0.5f), int((v.y + 1.0f) * height / 2.0f + 0.5f), v.z);
}

int main(int argc, char** argv)
{
	// Create model loader singleton.
	ModelLoader* modelLoader = ModelLoader::GetInstance();

	// Load model file from disk.
	Model* model = modelLoader->Load("african_head.obj");
	
	// Initialize image class.
	TGAImage* image = new TGAImage(width, height, TGAImage::RGB);

	// Set scene light direction.
	Vec3f lightDir(0.0f, 0.0f, -1.0f);

	// Initialize zbuffer array.
	float* zbuffer = new float[width * height];
	for (int i = width * height; i >= 0; i--)
	{
		zbuffer[i] = -std::numeric_limits<float>::max();
	}

	// Draw model.
	for (int i = 0; i < model->GetNumInds(); i += 3)
	{
		// Get indices.
		unsigned int i0 = model->GetInd(i);
		unsigned int i1 = model->GetInd(i + 1);
		unsigned int i2 = model->GetInd(i + 2);

		// Get the vertices using the indices.
		Vec3f v0 = model->GetVert(i0);
		Vec3f v1 = model->GetVert(i1);
		Vec3f v2 = model->GetVert(i2);

		Vec3f pts[3] = { World2Screen(v0), World2Screen(v1), World2Screen(v2) };

		// Calculate normal using cross product of two vectors in the plane.
		Vec3f normal = ((v2 - v0) ^ (v1 - v0)).normalize();

		// Calc intensity using dot product of normal and light direction.
		float intensity = normal * lightDir;

		// Only draw if light is hitting the surface.
		if (intensity > 0)
		{
			// Determine color from light intensity.
			TGAColor color(intensity * 255 * 0, intensity * 255, intensity * 255, 255);

			// Draw the current triangle.
			Triangle(pts, zbuffer, *image, color);
		}
	}

	// Flips image coords so that bottom-left corner is origin (needed for TGA format?)
	image->flip_vertically();
	
	// Output TGA file.
	image->write_tga_file("output.tga");

	std::cin.get();
	
	// Cleanup.
	delete(image);
	delete(model);
	delete(modelLoader);

	return 0;
}