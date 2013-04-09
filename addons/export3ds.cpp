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

struct Vert
{
  Vert(int a, int b, int c)
    : posIndex(a), tex0Index(b), matIndex(c)
  {
  }

  int posIndex;
  int tex0Index;
  int matIndex;

  bool operator<(const Vert& b) const
  {
    return memcmp(this, &b, sizeof(Vert)) < 0;
  }
};

typedef std::map<Vert, int> PairMap;

static int
getIndex(const Index& index, const Attributes& attribs, const PairMap& pairMap)
{
  int posIndex = index.posIndex;
  int texIndex = 0, matIndex = 0;
  if(attribs.hasTexCoords[0])
    texIndex = index.texCoordIndex[0];
  if(attribs.hasMatrixIndices)
    matIndex = index.matrixIndex;

  PairMap::const_iterator it;
  it = pairMap.find(Vert(posIndex, texIndex, matIndex));
  assert(it != pairMap.end());

  return it->second;
}

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

//it's better if one batch is one mesh, this makes hierarchy
//handling easier
static Lib3dsMesh*
batchToMesh(const Batch& b, const BModel& bmd, const std::string& name,
            int matIndex)
{
  const Vtx1& vtx = bmd.vtx1;
  const Attributes& attribs = b.attribs;

  if(!attribs.hasPositions)
    return NULL;

  size_t i, j;

  //count number of triangles
  //(3ds doesn't support tristrips and trifans, so we have to convert
  //everything to normal triangles)
  int triCount = countTriangles(b);

  //this is a bit tricky: bmd files store a position index
  //and a tex coord index for each point. 3ds has only
  //one index that is used as index for both the position array
  //and the tex coords array. So we have to check how many
  //(posIndex, texCoords0Index) pairs are in the bmd file,
  //and use that many "unified" indices in the 3ds file.

  //count needed vertices/texcoords
  PairMap pairMap; //holds (posIndex, texCoords0Index) pairs

  //decides which normals are smoothend
  std::map<int, std::set<int> > normalMapSet;

  for(j = 0; j < b.packets.size(); ++j)
  {
    const Packet& p = b.packets[j];
    for(i = 0; i < p.primitives.size(); ++i)
    {
      const Primitive& curr = p.primitives[i];
      for(size_t k = 0; k < curr.points.size(); ++k)
      {
        int texIndex = 0, matIndex = 0;

        if(attribs.hasTexCoords[0])
          texIndex = curr.points[k].texCoordIndex[0];

        if(attribs.hasMatrixIndices)
          matIndex = curr.points[k].matrixIndex;

        if(attribs.hasNormals)
          normalMapSet[curr.points[k].posIndex].insert(curr.points[k].normalIndex);

        pairMap[Vert(curr.points[k].posIndex, texIndex, matIndex)] = -1;
      }
    }
  }

  //copy data
  Lib3dsMesh* mesh = lib3ds_mesh_new(name.c_str());

  //vertices and texcoords
  lib3ds_mesh_resize_vertices(mesh, pairMap.size(), 1, 0);

  PairMap::iterator it, end = pairMap.end();
  for(i = 0, it = pairMap.begin(); it != end; ++it, ++i)
  {
    Vert curr = it->first;

    mesh->vertices[i][0] = vtx.positions[curr.posIndex].x();
    mesh->vertices[i][1] = vtx.positions[curr.posIndex].y();
    mesh->vertices[i][2] = vtx.positions[curr.posIndex].z();

    if(attribs.hasTexCoords[0])
    {
      float u = vtx.texCoords[0][curr.tex0Index].s;
      float v = vtx.texCoords[0][curr.tex0Index].t;

      //store v flipped - 3ds max has v texcoord axis inverted
      mesh->texcos[i][0] = u;
      mesh->texcos[i][1] = 1.f - v;
    }

    it->second = i;
  }

  //transform positions to global space
  Matrix44f matrixTable[10];
  std::set<Vert> alreadyTransformed;
  for(j = 0; j < b.packets.size(); ++j)
  {
    const Packet& p = b.packets[j];

    updateMatrixTable(bmd, p, matrixTable);
    Matrix44f mat = matrixTable[0];

    for(i = 0; i < p.primitives.size(); ++i)
    {
      const Primitive& curr = p.primitives[i];
      for(size_t j = 0; j < curr.points.size(); ++j)
      {
        int tex0Index = 0, matIndex = 0;
        if(attribs.hasTexCoords[0])
          tex0Index = curr.points[j].texCoordIndex[0];
        if(attribs.hasMatrixIndices)
          matIndex = curr.points[j].matrixIndex;

        if(alreadyTransformed.find(Vert(curr.points[j].posIndex,
          tex0Index, matIndex)) != alreadyTransformed.end())
          continue;
        alreadyTransformed.insert(Vert(curr.points[j].posIndex,
          tex0Index, matIndex));

        if(attribs.hasMatrixIndices)
          mat = matrixTable[curr.points[j].matrixIndex/3];
        //mat = Matrix44f::IDENTITY;

        //bmd has x right, y up, z in
        //3ds has x right, z up, y out
        //so swap y and z, change sign of y
        int index = getIndex(curr.points[j], attribs, pairMap);

        Vector3f v(mesh->vertices[index][0],
                   mesh->vertices[index][1],
                   mesh->vertices[index][2]);
        v = mat*v;
        mesh->vertices[index][0] = v.x();
        mesh->vertices[index][1] = -v.z();
        mesh->vertices[index][2] = v.y();
      }
    }
  }


  //tris
  lib3ds_mesh_resize_faces(mesh, triCount);
  int triIndex = 0;
  for(j = 0; j < b.packets.size(); ++j)
  {
    const Packet& p = b.packets[j];

    //set up matrix table
    updateMatrixTable(bmd, p, matrixTable);

    for(i = 0; i < p.primitives.size(); ++i)
    {
      const Primitive& curr = p.primitives[i];
      if(curr.points.size() < 2) continue;

      int a = 0;
      int b = 1;

      bool flip = true;
      for(size_t k = 2; k < curr.points.size(); ++k)
      {
        int c = k;

        //set all edges visibility to true
        mesh->faces[triIndex].flags = LIB3DS_FACE_VIS_AC | LIB3DS_FACE_VIS_BC | LIB3DS_FACE_VIS_AB;

        //get "unified" index
        int ia, ib, ic;
        ia = mesh->faces[triIndex].index[0] =
          getIndex(curr.points[a], attribs, pairMap);
        ib = mesh->faces[triIndex].index[1] =
          getIndex(curr.points[b], attribs, pairMap);
        ic = mesh->faces[triIndex].index[2] =
          getIndex(curr.points[c], attribs, pairMap);

        //if at least one vertex of the current triangle has more than
        //one normal, the triangle contains a "hard" edge, don't put
        //it into a smoothing group. Otherwise, smooth it.

        //this didn't work well with levels, which contain no normal data,
        //but might have antiparallel triangles whose normals would add up
        //zero - so use this only if normals were present.

        //*
        if(attribs.hasNormals
          && !(normalMapSet[ia].size() > 1 || normalMapSet[ib].size() > 1
               || normalMapSet[ic].size() > 1))
          mesh->faces[triIndex].smoothing_group = 1; // XXX -- bad?
        //*/

        //set material name
        if(attribs.hasTexCoords[0])
          mesh->faces[triIndex].material = matIndex;

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
                   Lib3dsNode *parent, int material)
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
  traverseSceneGraph(file, bmd, sg, NULL, -1);

  lib3ds_file_save(file, filename.c_str());
  lib3ds_file_free(file);
}
