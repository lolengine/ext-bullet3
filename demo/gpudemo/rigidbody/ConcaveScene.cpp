#include "ConcaveScene.h"
#include "GpuRigidBodyDemo.h"
#include "BulletCommon/btQuickprof.h"
#include "OpenGLWindow/ShapeData.h"

#include "OpenGLWindow/GLInstancingRenderer.h"
#include "BulletCommon/btQuaternion.h"
#include "OpenGLWindow/btgWindowInterface.h"
#include "gpu_broadphase/host/btGpuSapBroadphase.h"
#include "../GpuDemoInternalData.h"
#include "basic_initialize/btOpenCLUtils.h"
#include "OpenGLWindow/OpenGLInclude.h"
#include "OpenGLWindow/GLInstanceRendererInternalData.h"
#include "parallel_primitives/host/btLauncherCL.h"
#include "gpu_rigidbody/host/btGpuRigidBodyPipeline.h"
#include "gpu_rigidbody/host/btGpuNarrowPhase.h"
#include "gpu_rigidbody/host/btConfig.h"
#include "GpuRigidBodyDemoInternalData.h"
#include"../../ObjLoader/objLoader.h"


struct GraphicsVertex
{
	float xyzw[4];
	float normal[3];
	float uv[2];
};
struct GraphicsShape
{
	btAlignedObjectArray<GraphicsVertex>*	m_vertices;
	int				m_numvertices;
	btAlignedObjectArray<int>* 		m_indices;
	int				m_numIndices;
	float			m_scaling[4];
};

GraphicsShape* createGraphicsShapeFromWavefrontObj(objLoader* obj)
{
	btAlignedObjectArray<GraphicsVertex>* vertices = new btAlignedObjectArray<GraphicsVertex>;
	{
//		int numVertices = obj->vertexCount;
	//	int numIndices = 0;
		btAlignedObjectArray<int>* indicesPtr = new btAlignedObjectArray<int>;
			/*
		for (int v=0;v<obj->vertexCount;v++)
		{
			vtx.xyzw[0] = obj->vertexList[v]->e[0];
			vtx.xyzw[1] = obj->vertexList[v]->e[1];
			vtx.xyzw[2] = obj->vertexList[v]->e[2];
			btVector3 n(vtx.xyzw[0],vtx.xyzw[1],vtx.xyzw[2]);
			if (n.length2()>SIMD_EPSILON)
			{
				n.normalize();
				vtx.normal[0] = n[0];
				vtx.normal[1] = n[1];
				vtx.normal[2] = n[2];

			} else
			{
				vtx.normal[0] = 0; //todo
				vtx.normal[1] = 1;
				vtx.normal[2] = 0;
			}
			vtx.uv[0] = 0.5f;vtx.uv[1] = 0.5f;	//todo
			vertices->push_back(vtx);
		}
		*/

		for (int f=0;f<obj->faceCount;f++)
		{
			obj_face* face = obj->faceList[f];
			//btVector3 normal(face.m_plane[0],face.m_plane[1],face.m_plane[2]);
			if (face->vertex_count>=3)
			{
				btVector3 normal(0,1,0);
				int vtxBaseIndex = vertices->size();

				if (face->vertex_count<=4)
				{
					indicesPtr->push_back(vtxBaseIndex);
					indicesPtr->push_back(vtxBaseIndex+1);
					indicesPtr->push_back(vtxBaseIndex+2);
					
					GraphicsVertex vtx0;
					vtx0.xyzw[0] = obj->vertexList[face->vertex_index[0]]->e[0];
					vtx0.xyzw[1] = obj->vertexList[face->vertex_index[0]]->e[1];
					vtx0.xyzw[2] = obj->vertexList[face->vertex_index[0]]->e[2];
					vtx0.xyzw[3] = 0.f;//obj->vertexList[face->vertex_index[0]]->e[2];

					vtx0.uv[0] = 0.5f;//obj->textureList[face->vertex_index[0]]->e[0];
					vtx0.uv[1] = 0.5f;//obj->textureList[face->vertex_index[0]]->e[1];

					GraphicsVertex vtx1;
					vtx1.xyzw[0] = obj->vertexList[face->vertex_index[1]]->e[0];
					vtx1.xyzw[1] = obj->vertexList[face->vertex_index[1]]->e[1];
					vtx1.xyzw[2] = obj->vertexList[face->vertex_index[1]]->e[2];
					vtx1.xyzw[3]= 0.f;
					vtx1.uv[0] = 0.5f;//obj->textureList[face->vertex_index[1]]->e[0];
					vtx1.uv[1] = 0.5f;//obj->textureList[face->vertex_index[1]]->e[1];

					GraphicsVertex vtx2;
					vtx2.xyzw[0] = obj->vertexList[face->vertex_index[2]]->e[0];
					vtx2.xyzw[1] = obj->vertexList[face->vertex_index[2]]->e[1];
					vtx2.xyzw[2] = obj->vertexList[face->vertex_index[2]]->e[2];
					vtx2.xyzw[3] = 0.f;
					vtx2.uv[0] = 0.5f;obj->textureList[face->vertex_index[2]]->e[0];
					vtx2.uv[1] = 0.5f;obj->textureList[face->vertex_index[2]]->e[1];


					btVector3 v0(vtx0.xyzw[0],vtx0.xyzw[1],vtx0.xyzw[2]);
					btVector3 v1(vtx1.xyzw[0],vtx1.xyzw[1],vtx1.xyzw[2]);
					btVector3 v2(vtx2.xyzw[0],vtx2.xyzw[1],vtx2.xyzw[2]);

					normal = (v1-v0).cross(v2-v0);
					normal.normalize();
					vtx0.normal[0] = normal[0];
					vtx0.normal[1] = normal[1];
					vtx0.normal[2] = normal[2];
					vtx1.normal[0] = normal[0];
					vtx1.normal[1] = normal[1];
					vtx1.normal[2] = normal[2];
					vtx2.normal[0] = normal[0];
					vtx2.normal[1] = normal[1];
					vtx2.normal[2] = normal[2];
					vertices->push_back(vtx0);
					vertices->push_back(vtx1);
					vertices->push_back(vtx2);
				}
				if (face->vertex_count==4)
				{

					indicesPtr->push_back(vtxBaseIndex);
					indicesPtr->push_back(vtxBaseIndex+1);
					indicesPtr->push_back(vtxBaseIndex+2);
					indicesPtr->push_back(vtxBaseIndex+3);
//
					GraphicsVertex vtx3;
					vtx3.xyzw[0] = obj->vertexList[face->vertex_index[3]]->e[0];
					vtx3.xyzw[1] = obj->vertexList[face->vertex_index[3]]->e[1];
					vtx3.xyzw[2] = obj->vertexList[face->vertex_index[3]]->e[2];
					vtx3.uv[0] = 0.5;
					vtx3.uv[1] = 0.5;

					vtx3.normal[0] = normal[0];
					vtx3.normal[1] = normal[1];
					vtx3.normal[2] = normal[2];

					vertices->push_back(vtx3);

				}
			}
		}
		
		
		GraphicsShape* gfxShape = new GraphicsShape;
		gfxShape->m_vertices = vertices;
		gfxShape->m_numvertices = vertices->size();
		gfxShape->m_indices = indicesPtr;
		gfxShape->m_numIndices = indicesPtr->size();
		for (int i=0;i<4;i++)
			gfxShape->m_scaling[i] = 1;//bake the scaling into the vertices 
		return gfxShape;
	}
}

void ConcaveScene::setupScene(const ConstructionInfo& ci)
{
	objLoader* objData = new objLoader();
	//char* fileName = "data/plane.obj";
	char* fileName = "data/teddy.obj";//"plane.obj";
	//char* fileName = "data/sponza_closed.obj";//"plane.obj";

	FILE* f = 0;

	char relativeFileName[1024];
	{
		const char* prefix[]={"../","../../","../../../","../../../../"};
		int numPrefixes = sizeof(prefix)/sizeof(char*);

		for (int i=0;i<numPrefixes;i++)
		{
			
			sprintf(relativeFileName,"%s%s",prefix[i],fileName);
			f = fopen(relativeFileName,"r");
			if (f)
			{
				fclose(f);
				break;
			}
		}
	}

	if (f)
		fclose(f);
	else
		return;

	objData->load(relativeFileName);
	int index=10;

	{
		GraphicsShape* shape = createGraphicsShapeFromWavefrontObj(objData);
		btVector4 scaling(1,1,1,1);

		btAlignedObjectArray<btVector3> verts;
		for (int i=0;i<shape->m_numvertices;i++)
		{
			btVector3 vtx = (btVector3&)shape->m_vertices->at(i).xyzw;
			verts.push_back(vtx);
		}
	
		int colIndex = m_data->m_np->registerConcaveMesh(&verts,shape->m_indices,scaling);
		
		{
			int strideInBytes = 9*sizeof(float);
			int numVertices = sizeof(cube_vertices)/strideInBytes;
			int numIndices = sizeof(cube_vertices)/sizeof(int);
			//int shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
			//int shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);


			int shapeId = ci.m_instancingRenderer->registerShape(&shape->m_vertices->at(0).xyzw[0], shape->m_numvertices, &shape->m_indices->at(0), shape->m_numIndices);
			btQuaternion orn(0,0,0,1);
				
			btVector4 color(0,1,0,1.f);//0.5);
	
				
			{
				float mass = 0.f;
				btVector3 position(0,0,0);
				int id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId,position,orn,color,scaling);
				int pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass,position,orn,colIndex,index);
				index++;
			}

		

		}
	}

	int strideInBytes = 9*sizeof(float);
	int numVertices = sizeof(cube_vertices)/strideInBytes;
	int numIndices = sizeof(cube_vertices)/sizeof(int);
	//int shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	int shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	int group=1;
	int mask=1;
	
	

	

	if (1)
	{
		btVector4 scaling(1,1,1,1);
		int colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
		for (int i=0;i<ci.arraySizeX;i++)
		{
			for (int j=0;j<ci.arraySizeY;j++)
			{
				for (int k=0;k<ci.arraySizeZ;k++)
				{
					float mass = 1;

					btVector3 position(-2*ci.gapX+i*ci.gapX,25+j*ci.gapY,-2*ci.gapZ+k*ci.gapZ);
					btQuaternion orn(1,0,0,0);
				
					btVector4 color(0,1,0,1);
				
					int id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId,position,orn,color,scaling);
					int pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass,position,orn,colIndex,index);
				
					index++;
				}
			}
		}
	}
	float camPos[4]={0,0,0,0};//65.5,4.5,65.5,0};
	//float camPos[4]={1,12.5,1.5,0};
	m_instancingRenderer->setCameraTargetPosition(camPos);
	m_instancingRenderer->setCameraDistance(50);

}