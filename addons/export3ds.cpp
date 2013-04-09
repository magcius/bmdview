#include "export3ds.h"

#include <map>
#include <string.h> // memcmp
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>

#include <unistd.h>
#include <lib3ds.h>

#include "../common.h"
#include "exportTexture.h"

#include "../transformtools.h"

//one bmd packet is one 3ds mesh

//3ds supports
//- vertex positions
//- tex coords
//- basic hierarchy
//- textures (references to external files)
//- animations (in a strange way)

//does not support
//- vertex colors
//- vertex normals (but face normals...oh well)
//- vertex weighting/skinning
//- tristrips/trifans (converted to triangles)

//currently working
//- vertex positions
//- tex coords
//- textures

//issues: levels are lit with vertex colors, which are not exported.
//additionally, levels have no normals, so I can't decide which
//objects need smoothend normals and which not. so level export
//needs some manual tweaking

static int
countTriangles(const Primitive& curr)
{
  switch(curr.type)
  {
    case GX_TRIANGLE_STRIP:
    case GX_TRIANGLE_FAN:
      return curr.points.size() - 2;
      break;

    default: //other primitives not supported atm
      warn("countTriangles(): unsupported primitive type %x", curr.type);
      return 0;
  }
}

static int
countTriangles(const Packet& p)
{
  int triCount = 0;
  for(size_t i = 0; i < p.primitives.size(); ++i)
    triCount += countTriangles(p.primitives[i]);
  return triCount;
}

static int
countTriangles(const Batch& b)
{
  int triCount = 0;
  for(size_t j = 0; j < b.packets.size(); ++j)
    triCount += countTriangles(b.packets[j]);
  return triCount;
}

struct Vert {
  Index point;
  int idx;
};

static int
internVert(std::map<int, Vert> &usedVerts,
           Index point)
{
  std::map<int, Vert>::iterator it = usedVerts.find(point.posIndex);

  if (it == usedVerts.end())
  {
    Vert v;
    v.point = point;
    v.idx = usedVerts.size();
    usedVerts[point.posIndex] = v;
  }

  return usedVerts[point.posIndex].idx;
}

static Lib3dsMesh*
batchToMesh(const Batch& b, const BModel& bmd, const std::string& name,
            int material)
{
  const Vtx1& vtx = bmd.vtx1;
  const Attributes& attribs = b.attribs;

  if(!attribs.hasPositions)
    return NULL;

  Lib3dsMesh* mesh = lib3ds_mesh_new(name.c_str());

  int triCount = countTriangles(b);

  std::map<int, Vert> usedVerts;
  Matrix44f matrixTable[10];

  //tris
  lib3ds_mesh_resize_faces(mesh, triCount);
  int triIndex = 0;
  size_t i, j, k;

  for(j = 0; j < b.packets.size(); ++j)
  {
    const Packet& p = b.packets[j];

    for(i = 0; i < p.primitives.size(); ++i)
    {
      const Primitive& curr = p.primitives[i];
      if(curr.points.size() < 2) continue;

      int a = 0;
      int b = 1;

      bool flip = true;
      for(k = 2; k < curr.points.size(); ++k)
      {
        int c = k;

        mesh->faces[triIndex].index[0] = internVert(usedVerts, curr.points[a]);
        mesh->faces[triIndex].index[1] = internVert(usedVerts, curr.points[b]);
        mesh->faces[triIndex].index[2] = internVert(usedVerts, curr.points[c]);

        if(attribs.hasTexCoords[0])
          mesh->faces[triIndex].material = material;

        if(flip)
          std::swap(mesh->faces[triIndex].index[2],
                    mesh->faces[triIndex].index[0]);

        ++triIndex;

        switch(curr.type)
        {
          case GX_TRIANGLE_STRIP:
            flip = !flip;
            a = b;
            b = c;
            break;
          case GX_TRIANGLE_FAN:
            b = c;
            break;
        }
      }
    }
  }

  lib3ds_mesh_resize_vertices(mesh, usedVerts.size(), 1, 0);

  for(j = 0; j < b.packets.size(); ++j)
  {
    const Packet& p = b.packets[j];

    updateMatrixTable(bmd, p, matrixTable);

    for(i = 0; i < p.primitives.size(); ++i)
    {
      const Primitive& curr = p.primitives[i];
      for(k = 0; k < curr.points.size(); ++k)
      {
        const Index &point = curr.points[k];

        int idx = usedVerts[point.posIndex].idx;

        Vector3f v = vtx.positions[point.posIndex];

        Matrix44f mat = matrixTable[0];
        if(attribs.hasMatrixIndices)
          mat = matrixTable[point.matrixIndex/3];
        else
          mat = Matrix44f::IDENTITY;

        v = mat * v;

        mesh->vertices[idx][0] = v.x();
        mesh->vertices[idx][1] = -v.z();
        mesh->vertices[idx][2] = v.y();

        if(attribs.hasTexCoords[0])
        {
          float u = vtx.texCoords[0][point.texCoordIndex[0]].s;
          float v = vtx.texCoords[0][point.texCoordIndex[0]].t;

          // store v flipped - 3ds max has v texcoord axis inverted
          mesh->texcos[idx][0] = u;
          mesh->texcos[idx][1] = 1.f - v;
        }
      }
    }
  }

  return mesh;
}

static Lib3dsMaterial*
materialToMaterial(const BModel& bmd, const Material& mat,
                   const std::vector<std::string>& texNames,
                   const std::string& name)
{
  Lib3dsMaterial* mat3ds = lib3ds_material_new(name.c_str());

  u16 stage = mat.texStages[0];
  if(stage != 0xffff)
  {
    u16 v2 = bmd.mat3.texStageIndexToTextureIndex[stage];
    std::string texName = texNames[v2];
    strcpy(mat3ds->texture1_map.name, texName.c_str());

    const ImageHeader& ih = bmd.tex1.imageHeaders[v2];
    switch(ih.wrapS)
    {
      case 0: //clamp
        //16: clamp texture outside of [0, 1]x[0, 1]
        mat3ds->texture1_map.flags = LIB3DS_TEXTURE_DECALE | LIB3DS_TEXTURE_NO_TILE;
        break;
      case 1: //tile
        mat3ds->texture1_map.flags = 0; //tile textures
        break;
      case 2: //mirror
        mat3ds->texture1_map.flags = LIB3DS_TEXTURE_MIRROR;
        break;
    }

    mat3ds->opacity_map = mat3ds->texture1_map;
    if(ih.data->format != 1)
      mat3ds->opacity_map.flags |= LIB3DS_TEXTURE_ALPHA_SOURCE;
  }

  if(mat.cullIndex != 0xff)
    mat3ds->two_sided = bmd.mat3.cullModes[mat.cullIndex] == 0;

  return mat3ds;
}

static void
traverseSceneGraph(Lib3dsFile* file, const BModel& bmd, const SceneGraph& sg,
                   Lib3dsNode *&parent, int &material)
{
  switch(sg.type)
  {
    case 0x10: // joint
      // XXX - joints
      break;
    case 0x11: // material
      material = bmd.mat3.indexToMatIndex[sg.index];
      break;
    case 0x12: // batch
      const Batch& currBatch = bmd.shp1.batches[sg.index];
      char buf[8] = { 0 };
      snprintf(buf, sizeof(buf)-1, "n%d", sg.index);

      Lib3dsMesh* m = batchToMesh(currBatch, bmd, buf, material);

      if(m != NULL)
      {
        Lib3dsNode *node = (Lib3dsNode *) lib3ds_node_new_mesh_instance(m, buf, NULL, NULL, NULL);
        lib3ds_file_append_node(file, node, parent);
        lib3ds_file_insert_mesh(file, m, -1);
        parent = node;
      }

      break;
  }

  for(size_t i = 0; i < sg.children.size(); ++i)
    traverseSceneGraph(file, bmd, sg.children[i], parent, material);
}

static void
traverseSceneGraph(Lib3dsFile* file, const BModel& bmd, const SceneGraph& sg)
{
  int material = 0;
  Lib3dsNode *parent = NULL;
  traverseSceneGraph(file, bmd, sg, parent, material);
}

//exports a BModel to a .3ds file
void exportAs3ds(const BModel& bmd, const std::string& filename)
{
  std::string folder, basename;

  //strip folders
  splitPath(filename, folder, basename);

  size_t i;

  //export textures
  std::vector<std::string> imageNames;

  for(i = 0; i < bmd.tex1.imageHeaders.size(); ++i)
  {
    char name[9] = { 0 }; // 8.3 format
    snprintf(name, sizeof(name), "%.*s%02d", 6, basename.c_str(), i);
    std::string texName = name;

    //replace spaces
    std::string::size_type pos = texName.find(' ');
    while(pos != std::string::npos)
    {
      texName[pos] = '_';
      pos = texName.find(' ', pos + 1);
    }

    saveTexture(TGA, *bmd.tex1.imageHeaders[i].data, folder + texName);
    imageNames.push_back(texName + ".tga");
  }


  Lib3dsFile* file = lib3ds_file_new();

  //generate materials
  for(i = 0; i < bmd.mat3.materials.size(); ++i)
  {
    std::ostringstream str;
    str << "m" << i;
    Lib3dsMaterial* mat = materialToMaterial(bmd, bmd.mat3.materials[i], imageNames, str.str());
    lib3ds_file_insert_material(file, mat, i);
  }

  //geometry
  SceneGraph sg;
  buildSceneGraph(bmd.inf1, sg);
  traverseSceneGraph(file, bmd, sg);

  lib3ds_file_save(file, filename.c_str());
  lib3ds_file_free(file);
}
