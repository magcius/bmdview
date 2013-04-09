#include "drawbmd.h"

#include "bmdread.h"

#include "simple_gl.h"
#include <cassert>
#include <sstream>

#include "transformtools.h"
#include "camera.h"

#include "oglblock.h"

using namespace std;

void drawCoordFrame();

void drawBatch(BModel& bmd, int index, const Matrix44f& def);

Vector3f normTrans(const Matrix44f& m, const Vector3f& v)
{
  //TODO: use inverse transpose
  return Vector3f(
    m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2],
    m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2],
    m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2]
    );
}

Matrix44f operator*(float f, const Matrix44f& m)
{
  Matrix44f s;
  s.loadScale(f);
  return s*m; //doesn't change 4th row (intended)
}

Matrix44f updateMatrix(const Frame& f, Matrix44f effP)
{
  if(false) //f.unknown == 2)
  {
    //effP[3][0] = effP[3][1] = effP[3][2] = 0.f;
    //effP[0][3] = effP[1][3] = effP[2][3] = 0.f;
    /*
    for(int j = 0; j < 3; ++j)
      for(int k = 0; k < 3; ++k)
        effP[j][k] = j == k ? 1.f : 0.f;
    //*/

    //*
    Matrix44f ret = effP*frameMatrix(f);
    for(int j = 0; j < 3; ++j)
      for(int k = 0; k < 3; ++k)
        ret[j][k] = frameMatrix(f)[j][k];
        //ret[j][k] = j == k ? 1.f : 0.f;
    return ret;
    //*/
  }
  return effP*frameMatrix(f);
}


void drawBox(Vector3f bbMin, Vector3f bbMax, const Matrix44f& mat)
{
  bbMin = bbMin;
  bbMax = bbMax;

  int i;
  Vector3f d = bbMax - bbMin;
  Vector3f verts[8];
  verts[0] = bbMin;
  verts[1] = bbMin + Vector3f(d.x(), 0, 0);
  verts[2] = bbMin + Vector3f(d.x(), d.y(), 0);
  verts[3] = bbMin + Vector3f(0, d.y(), 0);

  verts[4] = bbMin + Vector3f(0, 0, d.z());
  verts[5] = bbMin + Vector3f(d.x(), 0, d.z());
  verts[6] = bbMin + Vector3f(d.x(), d.y(), d.z());
  verts[7] = bbMin + Vector3f(0, d.y(), d.z());

  for(i = 0; i < 8; ++i)
    verts[i] = mat*verts[i];

  //glColor3f(1, 0, 0);
  glBegin(GL_LINE_LOOP);
    for(i = 0; i < 4; ++i)
      glVertex3fv(verts[i]);
  glEnd();

  //glColor3f(0, 1, 0);
  glBegin(GL_LINES);
    for(i = 0; i < 4; ++i)
    {
      glVertex3fv(verts[i]);
      glVertex3fv(verts[i + 4]);
    }
  glEnd();

  //glColor3f(0, 0, 1);
  glBegin(GL_LINE_LOOP);
    for(i = 0; i < 4; ++i)
      glVertex3fv(verts[i + 4]);
  glEnd();
}

void drawSkeleton(BModel& bmd, const SceneGraph& s, Matrix44f p = Matrix44f::IDENTITY)
{
  if(s.type == 0x10)
  {
    const Frame& f = bmd.jnt1.frames[s.index];

    glBegin(GL_LINES);
      glColor3f(1, 0, 0);
      glVertex3fv(p*Vector3f(0, 0, 0));
      glColor3f(0, 0, 1);
      glVertex3fv(p*f.t);
    glEnd();
    glColor3f(1, 1, 1);

    p = updateMatrix(f, p);

    /*
    //draw coord frames at joints
    glPushMatrix();
    prepareCamera();
    glMultMatrixf(effP.transpose());
    glScalef(100, 100, 100);
    drawCoordFrame();
    glPopMatrix();
    //*/
  }

  for(size_t i = 0; i < s.children.size(); ++i)
    drawSkeleton(bmd, s.children[i], p);
}

GLenum compareMode(u8 gxMode)
{
  switch(gxMode)
  {
    case 0: //GX_NEVER
      return GL_NEVER;
      break;

    case 1: //GX_LESS
      return GL_LESS;
      break;

    case 2: //GX_EQUAL
      return GL_EQUAL;
      break;

    case 3: //GX_LEQUAL
      return GL_LEQUAL;
      break;

    case 4: //GX_GREATER
      return GL_GREATER;
      break;

    case 5: //GX_NEQUAL
      return GL_NOTEQUAL;
      break;

    case 6: //GX_GEQUAL
      return GL_GEQUAL;
      break;

    case 7: //GX_ALWAYS
      return GL_ALWAYS;
      break;

    default:
      fprintf (stderr, "unknown compare mode %d\n", gxMode);
      return GL_ALWAYS;
  }
}

GLenum blendFunc(u8 blendMode)
{
  switch(blendMode)
  {
    case 0: //GX_BL_ZERO
      return GL_ZERO;

    case 1: //GX_BL_ONE
      return GL_ONE;

    case 2: //GX_BL_SRCCLR / GX_BL_DSTCLR
      return GL_SRC_COLOR;

    case 3: //GX_BL_INVSRCCLOR / GX_BL_INVDSTCLR
      return GL_ONE_MINUS_SRC_COLOR;

    case 4: //GX_BL_SRCALPHA
      return GL_SRC_ALPHA;

    case 5: //GX_BL_INVSRCALPHA
      return GL_ONE_MINUS_SRC_ALPHA;

    case 6: //GX_DSTALPHA
      //return GL_DST_ALPHA;
      return GL_SRC_ALPHA;

    case 7: //GX_INVDSTALPHA
      //return GL_ONE_MINUS_DST_ALPHA;
      return GL_ONE_MINUS_SRC_ALPHA;
      
    default:
      warn("Unknown blendMode %d", (u32)blendMode);
      return GL_ONE;
  }
}

void applyMaterial(int index, Model& m, const OglBlock& oglBlock)
{
  BModel& bmd = *m.bmd;

  //material (TODO: this is very hacky right now)

  Material& mat = bmd.mat3.materials[bmd.mat3.indexToMatIndex[index]];
  string name = bmd.mat3.stringtable[index];

  if(!isKeyPressed('Q'))
  {
    //blending
    const BlendInfo& bi = bmd.mat3.blendInfos[mat.blendIndex];

    if(bi.blendMode != 0 && bi.blendMode != 1 //support only none and blend for now
      //|| bi.srcFactor >= 6 || bi.dstFactor >= 6 //don't support destination alpha for now
      )
    {
      fprintf(stderr, "%s: Unsupported BlendInfo %d %d %d %d\n",
        name.c_str(), bi.blendMode, bi.srcFactor, bi.dstFactor, bi.logicOp);
      glDisable(GL_BLEND);
    }
    else
    {
      switch(bi.blendMode)
      {
        case 0: //TODO: this should mean "don't blend", but links eyes don't
                //work without this
        case 1: //blend
           //TODO: check for destination alpha etc
          if(bi.srcFactor == 1 && bi.dstFactor == 0)
            glDisable(GL_BLEND);
          else
          {
            glBlendFunc(blendFunc(bi.srcFactor), blendFunc(bi.dstFactor));
            glEnable(GL_BLEND);
          }
          break;
      }
    }

  }
  else
    glDisable(GL_BLEND);

  //cull mode
  switch(bmd.mat3.cullModes[mat.cullIndex])
  {
    case 0: //GX_CULL_NONE
      glDisable(GL_CULL_FACE);
      break;

    case 1: //GX_CULL_FRONT
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      break;

    case 2: //GX_CULL_BACK
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      break;
  }

  if(bmd.mat3.zModes[mat.zModeIndex].enable)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);

  glDepthFunc(compareMode(bmd.mat3.zModes[mat.zModeIndex].zFunc));
  glDepthMask(bmd.mat3.zModes[mat.zModeIndex].enableUpdate);

  //texture
  if(m.oglBlock != NULL)
    setMaterial(index, *m.oglBlock, bmd);
}

void drawScenegraph(Model& m, const SceneGraph& s, const Matrix44f& p = Matrix44f::IDENTITY, int matIndex = 0)
{
  BModel& bmd = *m.bmd;

  Matrix44f effP = p;

  if(s.type == 0x10)
  {
    //joint
    const Frame& f = bmd.jnt1.frames[s.index];

    //this has to happen in the old frame (see butterflya.bmd)
    if(isKeyPressed('0'))
      drawBox(f.bbMin, f.bbMax, effP);

    bmd.jnt1.matrices[s.index] = updateMatrix(f, effP);
    effP = bmd.jnt1.matrices[s.index];
  }
  else if(s.type == 0x11)
  {
    //material
    //applyMaterial(s.index, bmd, *g_oglBlock);
    matIndex = s.index;
  }
  else if(s.type == 0x12)
  {
    //geometry

    //TODO: it's not sure that all matrices required by this call
    //are already calculated...
    applyMaterial(matIndex, m, *m.oglBlock);
    drawBatch(bmd, s.index, effP);
  }

  for(size_t i = 0; i < s.children.size(); ++i)
    drawScenegraph(m, s.children[i], effP, matIndex);
}

void adjustMatrix(Matrix44f& mat, u8 matrixType)
{
  if(isKeyPressed('X'))
    return;

  switch(matrixType)
  {
    case 1: //billboard
    {
      //get camera matrix to kill rotation from
      //the inverse camera transform in GL_MODELVIEW

      //slowww - each call computes a matrix inverse
      //TODO: why is transpose() needed?
      Matrix44f cam = getCameraMatrix().transpose();
      /*
      cam[3][0] = 0.f;
      cam[3][1] = 0.f;
      cam[3][2] = 0.f;
      /*/
      cam[0][3] = 0.f;
      cam[1][3] = 0.f;
      cam[2][3] = 0.f;
      //*/

      mat[0][0] = 1.f;
      mat[0][1] = 0.f;
      mat[0][2] = 0.f;

      mat[1][0] = 0.f;
      mat[1][1] = 1.f;
      mat[1][2] = 0.f;

      mat[2][0] = 0.f;
      mat[2][1] = 0.f;
      mat[2][2] = 1.f;

      mat = mat*cam;
    }break;

    case 2: //y billboard
    {
      Matrix44f cam = getCameraMatrix().transpose();

      Vector3f camPos(cam[0][3], cam[1][3], cam[2][3]);

      cam[0][3] = 0.f;
      cam[1][3] = 0.f;
      cam[2][3] = 0.f;

      //Vector3f up(cam[1][0], cam[1][1], cam[1][2]);
      //Vector3f up(cam[0][1], cam[1][1], cam[2][1]);
      Vector3f up(0, 1, 0);

      Vector3f billPos(mat[3][0], mat[3][1], mat[3][2]);
      Vector3f view = camPos - billPos;

      Vector3f front = view - up*view.dot(up);
      front.normalize();
      Vector3f right = -front.cross(up);

      /*
      mat[0][0] = right[0];
      mat[0][1] = right[1];
      mat[0][2] = right[2];

      mat[1][0] = up[0];
      mat[1][1] = up[1];
      mat[1][2] = up[2];

      mat[2][0] = front[0];
      mat[2][1] = front[1];
      mat[2][2] = front[2];
      /*/
      mat[0][0] = right[0];
      mat[1][0] = right[1];
      mat[2][0] = right[2];

      mat[0][1] = up[0];
      mat[1][1] = up[1];
      mat[2][1] = up[2];

      mat[0][2] = front[0];
      mat[1][2] = front[1];
      mat[2][2] = front[2];
      //*/

      //mat = mat*cam;
    }break;
  }
}

void drawBatch(BModel& bmd, int index, const Matrix44f& def)
{
  //if(index != 8 && index != 10) return;
  //if(index != 3 && index != 27) return;

  Batch& currBatch = bmd.shp1.batches[index];

  if(isKeyPressed('9'))
      drawBox(currBatch.bbMin, currBatch.bbMax, def);

  if(!currBatch.attribs.hasPositions)
  {
    fprintf(stderr, "found batch without positions\n");
    return; //not visible
  }

  if(currBatch.attribs.hasTexCoords[0] && !isKeyPressed('E')
    && !isKeyPressed('1')) //vertex colors only
  {
    glEnable(GL_TEXTURE_2D);
  }
  else
  {
    glDisable(GL_TEXTURE_2D);
  }

  Matrix44f matrixTable[10]; //10 "normal" matrices - see gx.h

  vector<bool> isMatrixWeighted(10, false); //used for debug output

  size_t j;
  for(j = 0; j < currBatch.packets.size(); ++j)
  {
    Packet& currPacket = currBatch.packets[j];

    //set up matrix table
    updateMatrixTable(bmd, currPacket, matrixTable, &isMatrixWeighted);
    

    //if no matrix index is given per vertex, 0 is the default.
    //otherwise, mat is overwritten later.
    Matrix44f mat = matrixTable[0];

    adjustMatrix(mat, currBatch.matrixType);

    for(size_t k = 0; k < currPacket.primitives.size(); ++k)
    {
      Primitive& currPrimitive = currPacket.primitives[k];

      switch(currPrimitive.type)
      {
        case 0x98:
          glBegin(GL_TRIANGLE_STRIP); break;

        case 0xa0:
          glBegin(GL_TRIANGLE_FAN); break;

        default:
          fprintf(stderr, "unknown primitive type %x\n", currPrimitive.type);
          continue;
      }

      glColor3f(1, 1, 1);
      for(size_t m = 0; m < currPrimitive.points.size(); ++m)
      {
        if(currBatch.attribs.hasMatrixIndices) // && currBatch.matrixType != 0)
        {
          mat = matrixTable[currPrimitive.points[m].matrixIndex/3];
          adjustMatrix(mat, currBatch.matrixType);
        }

        if(currBatch.attribs.hasNormals)
          glNormal3fv(normTrans(mat, bmd.vtx1.normals[currPrimitive.points[m].normalIndex]));

        if(currBatch.attribs.hasColors[0])
          glColor4ubv((GLubyte*)&bmd.vtx1.colors[0][currPrimitive.points[m].colorIndex[0]]);

        //multitexturing only supported via shaders
        if(!hasShaderHardware())
        {
          if(currBatch.attribs.hasTexCoords[0])
            glTexCoord2fv((float*)&bmd.vtx1.texCoords[0][currPrimitive.points[m].texCoordIndex[0]]);
        }
        else
          for(int b = 0; b < 8; ++b)
            if(currBatch.attribs.hasTexCoords[b])
              glMultiTexCoord2fv(GL_TEXTURE0 + b,
                (float*)&bmd.vtx1.texCoords[b][currPrimitive.points[m].texCoordIndex[b]]);

        glVertex3fv((mat)*bmd.vtx1.positions[currPrimitive.points[m].posIndex]);
      }

      glEnd();
    }
  }

}

void drawBmd(Model& m, const SceneGraph& sg)
{
  drawScenegraph(m, sg);
  glUseProgramObjectARB(0);
}
