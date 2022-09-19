#include "MeshRenderer.h"

void MeshRenderer::initComponent() {

	//objectMesh = new Mesh("meshes/d20.ms");					/// these should not be here 
	//shaderProgram = new Shader("shader.vs", "shader.fs");	//
	VAO = Renderer::BuildTrianglesVAO(objectMesh->vertex_position_data, objectMesh->vertex_normal_data, objectMesh->index_data, objectMesh->n_vert, objectMesh->n_idx);
	n_idx = objectMesh->n_idx;
}

void MeshRenderer::execute() {
	RenderObject ro(material, VAO, n_idx, getTrueTransform());
	Renderer::renderQ.push(ro);
}

void MeshRenderer::endComponent() {
	return;
}
