#pragma once

#include "Mesh3DStorage.hpp"
#include <iostream>
#include <fstream>

namespace poly_fem{
	namespace MeshProcessing3D{
#define Jacobian_Precision 1.e-7
		const int hex_face_table[6][4] =
		{
			{ 0,1,2,3 },
			{ 4,7,6,5 },
			{ 0,4,5,1 },
			{ 0,3,7,4 },
			{ 3,2,6,7 },
			{ 1,5,6,2 },
		};
		const int hex_tetra_table[8][4] =
		{
			{ 0,3,4,1 },
			{ 1,0,5,2 },
			{ 2,1,6,3 },
			{ 3,2,7,0 },
			{ 4,7,5,0 },
			{ 5,4,6,1 },
			{ 6,5,7,2 },
			{ 7,6,4,3 },
		};

		void build_connectivity(Mesh3DStorage &hmi);
		void reorder_hex_mesh_propogation(Mesh3DStorage &hmi);
		bool scaled_jacobian(Mesh3DStorage &hmi, Mesh_Quality &mq);
		double a_jacobian(Vector3d &v0, Vector3d &v1, Vector3d &v2, Vector3d &v3);

		void global_orientation_hexes(Mesh3DStorage &hmi);
		void refine_catmul_clark_polar(Mesh3DStorage &M, int iter);

		void  orient_surface_mesh(Mesh3DStorage &hmi);

	} // namespace Navigation3D
} // namespace poly_fem

