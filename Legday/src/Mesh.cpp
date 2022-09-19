#include "Mesh.h"

void Mesh::addNormal(GLuint vert, glm::vec3 vec) {
	glm::vec3 curVertNormal = glm::vec3(vertex_normal_data[vert * 4], vertex_normal_data[vert * 4 + 1], vertex_normal_data[vert * 4 + 2]);
	curVertNormal = glm::normalize(curVertNormal + vec);
	vertex_normal_data[vert * 4] = curVertNormal.x;
	vertex_normal_data[vert * 4 + 1] = curVertNormal.y;
	vertex_normal_data[vert * 4 + 2] = curVertNormal.z;
	vertex_normal_data[vert * 4 + 3] = 0.0;
}

void Mesh::computeNormals() {
	vertex_normal_data = (GLfloat*)malloc(sizeof(GLfloat) * n_vert * 4);
	memset(vertex_normal_data, 0, sizeof(GLfloat) * n_vert * 4);

	for (int i = 0; i < n_idx / 3; i++) {
		GLuint va = index_data[i * 3];
		GLuint vb = index_data[i * 3 + 1];
		GLuint vc = index_data[i * 3 + 2];

		glm::vec3 pa = glm::vec3(vertex_position_data[va * 3], vertex_position_data[va * 3 + 1], vertex_position_data[va * 3 + 2]);
		glm::vec3 pb = glm::vec3(vertex_position_data[vb * 3], vertex_position_data[vb * 3 + 1], vertex_position_data[vb * 3 + 2]);
		glm::vec3 pc = glm::vec3(vertex_position_data[vc * 3], vertex_position_data[vc * 3 + 1], vertex_position_data[vc * 3 + 2]);

		glm::vec3 faceNormal = glm::normalize(glm::cross((pa - pb), (pc - pa)));

		addNormal(va, faceNormal);
		addNormal(vb, faceNormal);
		addNormal(vc, faceNormal);
	}
}

Mesh::Mesh(string path) :
	vertex_position_data(0),
	vertex_color_data(0),
	index_data(0),
	n_idx(0),
	n_vert(0) 
{
	ifstream file;
	file.open(path, std::fstream::in);

	if (!file.is_open()) {
		cout << "couldn't open mesh file\n";
		exit(1);
	}

	unsigned int n_vertex = 0;
	unsigned int n_faces = 0;

	// count how many verts and how many faces
	string flag;
	while (file >> flag) {
		if (flag == "v") n_vertex++;
		if (flag == "f") n_faces++;
		file.ignore(1000, '\n');
	}

	// back to the start
	file.clear();
	file.seekg(0, file.beg);

	vertex_position_data = (GLfloat*)malloc(sizeof(GLfloat) * n_vertex * 3);
	vertex_color_data = (GLfloat*)malloc(sizeof(GLfloat) * n_vertex * 4);
	index_data = (GLuint*)malloc(sizeof(GLuint) * n_faces * 3);

	bool idx_from_zero = false;

	unsigned int v_index = 0;
	unsigned int f_index = 0;
	while (file >> flag) {
		if (flag == "v") {
			GLfloat pos[3];
			file >> pos[0] >> pos[1] >> pos[2];
			memcpy(vertex_position_data + v_index, pos, sizeof(pos));
			v_index += 3;
		}
		else if (flag == "f") {
			GLuint idx[3];
			file >> idx[0] >> idx[1] >> idx[2];
			memcpy(index_data + f_index, idx, sizeof(idx));
			f_index += 3;
			if (idx[0] == 0 or idx[1] == 0 or idx[2] == 0) idx_from_zero = true;
		}
		else file.ignore(1000, '\n');
	}

	if (!idx_from_zero) {
		for (int i = 0; i < n_faces * 3; i++) {
			index_data[i]--;
		}
	}
	
	n_idx = n_faces * 3;
	n_vert = n_vertex;

	file.close();

	computeNormals();
}