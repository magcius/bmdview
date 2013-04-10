#include "oglblock.h"

#include <sstream>
#include <fstream>

static GLenum
texFilter(u8 filter)
{
  switch(filter)
  {
    case 0:
      return GL_NEAREST;

    case 1:
      return GL_LINEAR;
      
    case 2:
      return GL_NEAREST_MIPMAP_NEAREST;
      
    case 3:
      return GL_LINEAR_MIPMAP_NEAREST;
      
    case 4:
      return GL_NEAREST_MIPMAP_LINEAR;
      
    case 5:
      return GL_LINEAR_MIPMAP_LINEAR;
    
    default:
      warn("Unknown tex filter %d", (u32)filter);
      return GL_LINEAR;
  }
}

static void
setFilters(int magFilter, int minFilter, int mipCount)
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  texFilter(magFilter));

  if(minFilter < 2)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    texFilter(minFilter));
  else
  {
    if(GLEW_VERSION_1_2)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                      mipCount - 1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      texFilter(minFilter));
    }
    else
    {
      fprintf(stderr, "Texture LOD not supported, using linear filter\n");
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR);
    }
  }
}

static void
setTexWrapModeST(GLenum wrapST, u8 mode)
{
  switch(mode)
  {
    case 0: //TODO: check if this is supported (?)
      glTexParameterf(GL_TEXTURE_2D, wrapST, GL_CLAMP_TO_EDGE);
      break;
    case 1:
      glTexParameterf(GL_TEXTURE_2D, wrapST, GL_REPEAT);
      break;
    case 2:
      glTexParameterf(GL_TEXTURE_2D, wrapST, GL_MIRRORED_REPEAT);
      break;
  }
}

static void
setTexWrapMode(u8 sMode, u8 tMode)
{
  setTexWrapModeST(GL_TEXTURE_WRAP_S, sMode);
  setTexWrapModeST(GL_TEXTURE_WRAP_T, tMode);
}

static std::string
getLog(GLhandleARB p)
{
  GLint len;
  std::string ret;
  glGetObjectParameterivARB(p, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);
  if(len > 0)
  {
    ret.resize(len);
    glGetInfoLogARB(p, len, NULL, &ret[0]);
  }
  return ret;
}

static void
writeTexGen(std::ostream& out, const TexGenInfo& texGen, int i, const Material& currMat, const Mat3& mat)
{
  out << std::hex << "\n  //GX_SetTexCoordGen(0x" << i << ", 0x"
    << (int)texGen.texGenType << ", 0x" << (int)texGen.texGenSrc << ", 0x"
    << (int)texGen.matrix << "):\n" << std::dec;

  std::ostringstream tempOut;
  tempOut << "gl_TexCoord[" << i << "]";
  std::string dest = tempOut.str();

  if(texGen.texGenType == 0 || texGen.texGenType == 1)
  {
    //warn("writeTexGen() type %d: matrix support not really implemented", texGen.texGenType);

    out << "  " << dest << " = ";

    if(texGen.matrix == 0x3c)
      ; //nothing to do
    else if(texGen.matrix >= 0x1e && texGen.matrix <= 0x39)
      out << "gl_TextureMatrix[" << (texGen.matrix - 0x1e)/3 <<"]*";
    else
      warn("writeTexGen() type %d: unsupported matrix 0x%x", texGen.texGenType, texGen.matrix);

    if(texGen.texGenSrc >= 4 && texGen.texGenSrc <= 11)
      out << "gl_MultiTexCoord" << texGen.texGenSrc - 4 <<";\n";
    else if(texGen.texGenSrc == 0)
    {
      log("writeTexGen() type 0: Found src 0, might not yet work (use transformed or untransformed pos?)");
      out << "gl_Position;\n";
    }
    else if(texGen.texGenSrc == 1)
    {
      log("writeTexGen() type 0: Found src 1, might not yet work (use transformed or untransformed normal?)");
      out << "vec4(normal, 1.0);\n";
    }
    else
    {
      warn("writeTexGen() type %d: unsupported src 0x%x", texGen.texGenType, texGen.texGenSrc);
      out << "vec4(0.0, 0.0, 0.0, 0.0); //(unsupported)\n";
    }

    if(currMat.texMtxInfos[i] != 0xffff)
    {
      const TexMtxInfo& tmi = mat.texMtxInfos[currMat.texMtxInfos[i]];
      out << "  " << dest << ".st *= vec2(" << tmi.scaleU << ", " << tmi.scaleV << ");\n";
      out << "  " << dest << ".st += vec2(" << tmi.scaleCenterX*(1 - tmi.scaleU)
                          << ", " << tmi.scaleCenterY*(1 - tmi.scaleV) << ");\n";
    }
  }
  else if(texGen.texGenType == 0xa)
  {
    if(texGen.matrix != 0x3c)
      warn("writeTexGen() type 0xa: unexpected matrix 0x%x", texGen.matrix);
    if(texGen.texGenSrc != 0x13)
      warn("writeTexGen() type 0xa: unexpected src 0x%x", texGen.texGenSrc);

    log("writeTexGen(): Found type 0xa (SRTG), doesn't work right yet");
  }
  else
  {
    warn("writeTexGen: unsupported type 0x%x", texGen.texGenType);
    out << "  " << dest << " = vec4(0.0, 0.0, 0.0, 0.0); //(unsupported texgentype)\n";
  }
}

std::string createVertexShaderString(int index, const Mat3& mat)
{
  const Material& currMat = mat.materials[index];
  std::ostringstream out;
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::showpoint);
  int i;

  out << "//TODO: color chan info, lighting, tex coord gen 2\n";

  out << "void main()\n{\n";

  out << "  //transform position (and normal (TODO: do normal right))\n";
  out << "  gl_Position = ftransform();\n";
  out << "  vec3 normal = gl_NormalMatrix*gl_Normal;\n\n";
  out << "  vec4 color;\n\n";

  //TODO: hack (lighting)
  if(currMat.chanControls[0] >= mat.colorChanInfos.size())
    out << "  color = gl_Color;\n";
  else
  {
    const ColorChanInfo& chanInfo = mat.colorChanInfos[currMat.chanControls[0]];
    out << "\n  //TODO: make this right (color):\n";
    if(chanInfo.matColorSource == 1)
      out << "  color = gl_Color;\n";
    else
    {
      const MColor& c = mat.color1[currMat.color1[0]];
      out << "  color = vec4("
          << c.r/255.f << ", " << c.g/255.f << ", " << c.b/255.f << ", "
          << c.a/255.f << ");\n";
    }

    //test if this is a "light enable" flag: (seems so)
    //*
//http://kuribo64.cjb.net/?page=thread&id=532#14984
//printf("%d %d\n", chanInfo.enable, chanInfo.litMask);
    if(chanInfo.enable != 0)
    {
      MColor amb = { 0, 0, 0, 0 };
      //if(currMat.color2[0] != 0xffff)
      if(currMat.color2[0] != 0xffff && currMat.color2[0] < mat.color2.size())
        amb = mat.color2[currMat.color2[0]];

      //out << "  color.rgb *= max(1.0, normal.z);\n";
      out << "  vec4 light = gl_ModelViewMatrix*vec4(0.0, 0.0, 1.0, 0.0);\n";
      //out << "  color.rgb *= max(dot(light.xyz, normal), 0.0);\n";
      //out << "  color.rgb *= max(dot(normalize(light.xyz), normal), 0.0);\n";
      //out << "  color.rgb *= " << 1 - amb.r/255.f << "*max(0.5*dot(normalize(light.xyz), normalize(normal)), 0.0) + "
      //    << amb.r/255.f << ";\n";
      //out << "  color.rgb *= max(normal.z, 0.0);\n";
      out << "  color.rgb *= 0.5;\n";

      //out << "  color.rgb += vec3(" << amb.r/255.f << ", " << amb.g/255.f << ", "
      //    << amb.b/255.f << ");\n";
    }

  }
  //*/
  out << "  gl_FrontColor = color;\n\n\n";


  //texgen
  out << "  //TexGen\n";
  for(i = 0; i < mat.texGenCounts[currMat.texGenCountIndex]; ++i) //num TexGens == num Textures
    writeTexGen(out, mat.texGenInfos[currMat.texGenInfos[i]], i, currMat, mat);
  out << "\n\n";

  out << "}\n";
  return out.str();
}

static std::string
getRegIdName(u8 id)
{
  std::ostringstream out;
  if(id == 0)
    out << "gl_FragColor";
  else
    out << "c" << id - 1;
  return out.str();
}

static std::string
getTexAccess(const TevOrderInfo& info)
{
  std::ostringstream out;
  out << "texture2D(texture[" << (int)info.texMap << "], "
      << "gl_TexCoord[" << (int)info.texCoordId << "].st)";
  return out.str();
}

static std::string
getRasColor(const TevOrderInfo& info)
{
  //TODO:
  return "gl_Color";

  switch(info.chanId)
  {
    case 0:
      return "gl_Color.rgb";
    case 1:
      return "gl_SecondaryColor.rgb";
    case 2:
      return "gl_Color.a";
    case 3:
      return "gl_SecondaryColor.a";
    case 4:
      return "return gl_Color";
    case 5:
      return "return gl_SecondaryColor";
    case 6:
      return "vec4(0.0, 0.0, 0.0, 0.0);";
    //TODO: 7, 8
    default:
    {
      warn("getRasColor(): unknown chanId 0x%x", info.chanId);
      return "vec4(0.0, 1.0, 0.0, 1.0);";
    }
  }
}

static std::string
getColorIn(u8 op, u8 konst, const TevOrderInfo& info)
{
  const char* suffix[2] = { ".rgb", ".aaa" };

  if(op <= 7)
    return getRegIdName(op/2) + suffix[op%2];

  switch(op)
  {
    case 8:
    case 9:
      return getTexAccess(info) + suffix[op - 8];

    case 10:
    case 11:
      return getRasColor(info) + suffix[op - 10];

    case 12: return "1.0*ONE.rgb";
    case 13: return "0.5*ONE.rgb";

    case 14:
    {
      if(konst <= 7)
        switch(konst)
        {
          case 0: return "1.0*ONE.rgb";
          case 1: return "0.875*ONE.rgb";
          case 2: return "0.75*ONE.rgb";
          case 3: return "0.625*ONE.rgb";
          case 4: return "0.5*ONE.rgb";
          case 5: return "0.375*ONE.rgb";
          case 6: return "0.25*ONE.rgb";
          case 7: return "0.125*ONE.rgb";
        }
      else if(konst < 0xc)
      {
        warn("getColorOp(): unknown konst %x", konst);
        return "ERROR";
      }

      konst -= 0xc;
      const char* v1[4] = { "konst0", "konst1", "konst2", "konst3" };
      const char* v2[5] = { ".rgb", ".rrr", ".ggg", ".bbb", ".aaa" };
      return std::string(v1[konst%4]) + v2[konst/4];
    }
    case 15:
      return "0.0*ONE.rgb";
    default:
      warn("Unknown colorIn %d", (u32)op);
      return "0.0*ONE.rgb";
  }
}

static std::string
getAlphaIn(u8 op, u8 konst, const TevOrderInfo& info)
{
  if(op <= 3)
    return getRegIdName(op) + ".a";

  switch(op)
  {
    case 4: return getTexAccess(info) + ".a";
    case 5: return getRasColor(info) + ".a";

    case 6:
    {
      if(konst <= 7)
        switch(konst)
        {
          case 0: return "1.0";
          case 1: return "0.875";
          case 2: return "0.75";
          case 3: return "0.625";
          case 4: return "0.5";
          case 5: return "0.375";
          case 6: return "0.25";
          case 7: return "0.125";
        }
      else if(konst < 0x10)
      {
        warn("getColorOp(): unknown konst %x", konst);
        return "ERROR";
      }

      konst -= 0x10;
      const char* v1[4] = { "konst0", "konst1", "konst2", "konst3" };
      const char* v2[5] = { ".r", ".g", ".b", ".a" };
      return std::string(v1[konst%4]) + v2[konst/4];
    }

    case 7: return "0.0";
    
    default:
      warn("Unknown alpha input %d", (u32)op);
      return "0.0";
  }
}

static std::string
getMods(const std::string& dest, u8 bias, u8 scale, u8 clamp, int type)
{
  std::ostringstream out;

  const char* biasStr[3][2] = {
    { "", "" },
    { "+ 0.5*ONE.rgb", "+ 0.5" },
    { " - 0.5*ONE.rgb", " - 0.5"}
  };

  switch(bias)
  {
    case 0: break;

    case 1:
    case 2:
      out << "  " << dest << " = " << dest << biasStr[bias][type] << ";\n";
      break;

    default:
      warn("getMods(): unknown bias %d", bias);
  }

  const char* scaleStr[4] = { "1.0", "2.0", "4.0", "0.5" };
  switch(scale)
  {
    case 0: break;
    case 1:
    case 2:
    case 3:
      out << "  " << dest << " *= " << scaleStr[scale] << ";\n";
      break;

    default:
      warn("getMods(): unknown scale %d", scale);
  }

  if(clamp)
  {
    if(type == 0)
      out << "  " << dest << " = clamp(" << dest << ", vec3(0.0, 0.0, 0.0), ONE.rgb);\n";
    else
      out << "  " << dest << " = clamp(" << dest << ", 0.0, 1.0);\n";
  }

  return out.str();
}

static std::string
getOp(u8 op, u8 bias, u8 scale, u8 clamp, u8 regId, std::string ins[4], int type)
{
  const char* suffix0[2] = { ".rgb", ".a" };

  std::ostringstream out;
  std::string dest = getRegIdName(regId) + suffix0[type];

  switch(op)
  {
    case 0:
    case 1:
    {
      out << "  " << dest << " = "
          << "mix(" << ins[0] << ", " << ins[1] << ", "
          << ins[2] << ")";

      if(op == 0)
        out << " + ";
      else
      {
        out << " - ";
        log("getOp(): op 1 might not work");
      }
       
      out << ins[3] << ";\n";

      if(op == 1)
        out << "  " << dest << " = -" << dest << ";\n";

      out << getMods(dest, bias, scale, clamp, type);

      return out.str();
    }

    case 8:
    case 9:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
    {
      if(type == 0)
      {
        out << "  if(";

        if(op >= 0xa)
          out << "dot(";
        
        out << ins[0];

        switch(op)
        {
          case 8:
          case 9:
            out << ".r"; break;
          case 0xa:
          case 0xb:
            out << ".rg, vec2(255.0/65535.0, 255.0*256.0/65535.0))"; break;
          case 0xc:
          case 0xd:
            out << ".rgb, vec3(255.0/16777215.0, 255.0*256.0/16777215.0, 255.0*65536.0/16777215.0))"; break;
        }

        if(op%2 == 0) out << " > ";
        else out << " == ";

        if(op >= 0xa)
          out << "dot(";

        out << ins[1];

        switch(op)
        {
          case 8:
          case 9:
            out << ".r"; break;
          case 0xa:
          case 0xb:
            out << ".rg, vec2(255.0/65535.0, 255.0*256.0/65535.0))"; break;
          case 0xc:
          case 0xd:
            out << ".rgb, vec3(255.0/16777215.0, 255.0*256.0/16777215.0, 255.0*65536.0/16777215.0))"; break;
        }
        
        out << ")\n    " << dest << " = " << ins[2] << ";\n"
            << "  else\n    " << dest << " = " << ins[3] << ";\n";

        //out << getMods(dest, 0, scale, clamp, type);
        if(bias != 3 || scale != 1 || clamp != 1)
          warn("getOp() comp0: unexpected bias %d, scale %d, clamp %d", bias, scale, clamp);

        return out.str();
      }
    }break; //type == 1

    case 0xe:
    case 0xf:
    {
      if(type == 1)
      {
        out << "  if(" << ins[0];
        if(op == 0xe) out << " > ";
        else out << " == ";
        out << ins[1] << ")\n    " << dest << " = " << ins[2] << ";\n"
            << "  else\n    " << dest << " = " << ins[3] << ";\n";

        //out << getMods(dest, 0, scale, clamp, type);
        if(bias != 3 || scale != 1 || clamp != 1)
          warn("getOp() comp0: unexpected bias %d, scale %d, clamp %d", bias, scale, clamp);

        return out.str();
      }
      //TODO: gnd.bdl uses 0xe on type == 0
    }break;
  }

  warn("getOp(): unsupported op %d", op);
  if(type == 0)
    out << "  " << dest << " = vec3(0., 1., 0.); //(unsupported)";
  else
    out << "  " << dest << " = 0.5; //(unsupported)";
  return out.str();
}

static std::string
getAlphaCompare(int comp, int ref)
{
  //ATI cards can't even handle constant bools if we use any() and all(),
  //so for now use complicated expressions for true and false
  //(TODO: optimize constant bools away).

  std::ostringstream out;
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::showpoint);
  switch(comp)
  {
    case 0: //GX_NEVER
      //out << "false";
      out << "gl_FragColor.a <= -10.0";
      break;
    case 1: //GX_LESS
      out << "gl_FragColor.a" << " < " << ref/255.f;
      break;
    case 2: //GX_EQUAL
      out << "gl_FragColor.a" << " == " << ref/255.f;
      break;
    case 3: //GX_LEQUAL
      out << "gl_FragColor.a" << " <= " << ref/255.f;
      break;
    case 4: //GX_GREATER
      out << "gl_FragColor.a" << " > " << ref/255.f;
      break;
    case 5: //GX_NEQUAL
      out << "gl_FragColor.a" << " != " << ref/255.f;
      break;
    case 6: //GX_GEQUAL
      out << "gl_FragColor.a" << " >= " << ref/255.f;
      break;
    case 7: //GX_ALWAYS
      //out << "true";
      out << "gl_FragColor.a <= 10.0";
      break;
  }
  return out.str();
}

static std::string
createFragmentShaderString(int index, const Mat3& mat)
{
  const Material& currMat = mat.materials[index];
  std::ostringstream out;
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::showpoint);
  int i;


  out << "uniform sampler2D texture[8];\n";
  out << "void main()\n{\n";
  out << "  const vec4 ONE = vec4(1.0, 1.0, 1.0, 1.0);\n\n";

  //check which constant colors are used, write these:
  bool needK[4] = { false, false, false, false };
  for(i = 0; i < mat.tevCounts[currMat.tevCountIndex]; ++i)
  {
    u8 konstColor = currMat.constColorSel[i];
    u8 konstAlpha = currMat.constAlphaSel[i];
    const TevStageInfo& stage = mat.tevStageInfos[currMat.tevStageInfo[i]];

    if((konstColor > 7 && konstColor < 0xc)
      || (konstAlpha > 7 && konstAlpha < 0x10))
    {
      warn("createFragmentShaderString: Invalid color sel");
      continue; //should never happen
    }

    if(konstColor > 7
      && (stage.colorIn[0] == 0xe || stage.colorIn[1] == 0xe 
          || stage.colorIn[2] == 0xe || stage.colorIn[3] == 0xe))
      needK[(konstColor - 0xc)%4] = true;
    if(konstAlpha > 7
      && (stage.alphaIn[0] == 6 || stage.alphaIn[1] == 6 
          || stage.alphaIn[2] == 6 || stage.alphaIn[3] == 6))
      needK[(konstAlpha - 0x10)%4] = true;
  }
  for(i = 0; i < 4; ++i)
  {
    if(needK[i])
    {
      const MColor& c = mat.color3[currMat.color3[i]];
      //const Color16& c = mat.colorS10[currMat.colorS10[i]];
      out << "  const vec4 konst" << i << " = vec4("
          << c.r/255.f << ", " << c.g/255.f << ", "
          << c.b/255.f << ", " << c.a/255.f << ");\n";
    }
  }
  out << "\n";

  //check which registers are used, write these:
  out << "  //registers (use gl_FragColor as GX_TEVPREV)\n";
  bool needReg[4] = { false, false, false, false };
  for(i = 0; i < mat.tevCounts[currMat.tevCountIndex]; ++i)
  {
    const TevStageInfo& stage = mat.tevStageInfos[currMat.tevStageInfo[i]];
    needReg[stage.colorRegId] = true;
    needReg[stage.alphaRegId] = true;

    int j;
    for(j = 0; j < 4; ++j)
      if(stage.colorIn[j] <= 7)
        needReg[stage.colorIn[j]/2] = true;

    for(j = 0; j < 4; ++j)
      if(stage.alphaIn[j] <= 3)
        needReg[stage.alphaIn[j]] = true;
  }
  for(i = 1; i < 4; ++i)
  {
    //TODO: this is still somewhat broken (startline.bmd for example)

    if(needReg[i])
    //if(needReg[i==0?3:i-1])
    {
      const Color16& c = mat.colorS10[currMat.colorS10[i==0?3:i-1]]; //XXXX ????? (sunflower.bmd)
      //const Color16& c = mat.colorS10[currMat.colorS10[i]]; //XXXX ?????
      //const MColor& c = mat.color3[currMat.color3[i]]; //XXXX ?????
      if(i == 0)
        out << "  " << getRegIdName(i);
      else
        out << "  vec4 " << getRegIdName(i);
      out << " = vec4("
          << c.r/255.f << ", " << c.g/255.f << ", "
          << c.b/255.f << ", " << c.a/255.f << ");\n";
    }
  }
  out << "\n";


  out << "  //Tev stages\n";
  for(i = 0; i < mat.tevCounts[currMat.tevCountIndex]; ++i)
  {
    const TevOrderInfo& order = mat.tevOrderInfos[currMat.tevOrderInfo[i]];
    const TevStageInfo& stage = mat.tevStageInfos[currMat.tevStageInfo[i]];

    //tev order
    out << std::hex;
    out << "\n  //Tev stage " << i << "\n";
    out << "  //GX_SetTevOrder(0x" << i << ", 0x"
        << (int)order.texCoordId << ", 0x" << (int)order.texMap << ", 0x"
        << (int)order.chanId << ")\n";

    //kcolorsel, color in, color op
    /*
    out << "\n  //GX_SetTevKColorSel(0x" << i
        << ", 0x" << (int)currMat.constColorSel[i] << ")\n";
    out << "  //GX_SetTevColorIn(0x" << i
        << ", 0x" << (int)stage.colorIn[0] << ", 0x" << (int)stage.colorIn[1]
        << ", 0x" << (int)stage.colorIn[2] << ", 0x" << (int)stage.colorIn[3]
        << ")\n";
    out << "  //GX_SetTevColorOp(0x" << i
        << ", 0x" << (int)stage.colorOp << ", 0x" << (int)stage.colorBias
        << ", 0x" << (int)stage.colorScale << ", 0x" << (int)stage.colorClamp
        << ", 0x" << (int)stage.colorRegId << ")\n";
    //*/

    std::string colorIns[4];
    colorIns[0] = getColorIn(stage.colorIn[0], currMat.constColorSel[i], order);
    colorIns[1] = getColorIn(stage.colorIn[1], currMat.constColorSel[i], order);
    colorIns[2] = getColorIn(stage.colorIn[2], currMat.constColorSel[i], order);
    colorIns[3] = getColorIn(stage.colorIn[3], currMat.constColorSel[i], order);

    out << getOp(stage.colorOp, stage.colorBias, stage.colorScale,
      stage.colorClamp, stage.colorRegId, colorIns, 0) << "\n";

    //kalphasel, alpha in, alpha op
    /*
    out << "\n  //GX_SetTevKAlphaSel(0x" << i
        << ", 0x" << (int)currMat.constAlphaSel[i] << ")\n";
    out << "  //GX_SetTevAlphaIn(0x" << i
        << ", 0x" << (int)stage.alphaIn[0] << ", 0x" << (int)stage.alphaIn[1]
        << ", 0x" << (int)stage.alphaIn[2] << ", 0x" << (int)stage.alphaIn[3]
        << ")\n";
    out << "  //GX_SetTevAlphaOp(0x" << i
        << ", 0x" << (int)stage.alphaOp << ", 0x" << (int)stage.alphaBias
        << ", 0x" << (int)stage.alphaScale << ", 0x" << (int)stage.alphaClamp
        << ", 0x" << (int)stage.alphaRegId << ")\n";
    //*/

    std::string alphaIns[4];
    alphaIns[0] = getAlphaIn(stage.alphaIn[0], currMat.constAlphaSel[i], order);
    alphaIns[1] = getAlphaIn(stage.alphaIn[1], currMat.constAlphaSel[i], order);
    alphaIns[2] = getAlphaIn(stage.alphaIn[2], currMat.constAlphaSel[i], order);
    alphaIns[3] = getAlphaIn(stage.alphaIn[3], currMat.constAlphaSel[i], order);

    out << getOp(stage.alphaOp, stage.alphaBias, stage.alphaScale,
      stage.alphaClamp, stage.alphaRegId, alphaIns, 1) << "\n";

    out << std::dec;
  }
  out << "\n\n";

  /*
  if(mat.texCounts[currMat.texCountIndex] == 0)
    out << "  gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n";
  else
    out << "  gl_FragColor = texture2D(texture0, gl_TexCoord[0].st);\n";
  //*/


  //do alpha testing in shader, because normal opengl alpha testing
  //can't handle logical ops
  const AlphaCompare& ac = mat.alphaCompares[currMat.alphaCompIndex];

  out << "  bool a = ";
  out << getAlphaCompare(ac.comp0, ac.ref0);
  out << ";\n";
  out << "  bool b = ";
  out << getAlphaCompare(ac.comp1, ac.ref1);
  out << ";\n";
  //*
  out << "  if(!(";
  switch(ac.alphaOp)
  {
    case 0: //GX_AOP_AND
      out << "a && b";
      break;
    case 1: //GX_AOP_OR
      out << "a || b";
      break;
    case 2: //GX_AOP_XOR
      out << "a != b";
      break;
    case 3: //GX_AOP_XNOR
      out << "a == b";
      break;
  }
  
  out << "))\n";
  out << "    discard;\n";

  out << "}\n";
  return out.str();
}


static GLhandleARB
createShader(const std::string& sourceString, GLenum target)
{
  GLhandleARB ret = glCreateShaderObjectARB(target);

  GLint l = sourceString.length(), hasCompiled;
  const char* s = sourceString.c_str();
  glShaderSourceARB(ret, 1, &s, &l);
  glCompileShaderARB(ret);
  glGetObjectParameterivARB(ret, GL_OBJECT_COMPILE_STATUS_ARB, &hasCompiled);
  if(!hasCompiled)
  {
    warn("Failed to compile %s\nError: %s", sourceString.c_str(), getLog(ret).c_str());
    glDeleteObjectARB(ret);
    return 0;
  }

  return ret;
}

static GLhandleARB
createProgram(GLhandleARB& vertex, GLhandleARB& fragment)
{
  if(vertex == 0 || fragment == 0)
  {
    if(vertex != 0)
      glDeleteObjectARB(vertex);
    if(fragment != 0)
      glDeleteObjectARB(fragment);
    vertex = fragment = 0;
    return 0;
  }

  GLhandleARB program = glCreateProgramObjectARB();
  glAttachObjectARB(program, vertex);
  glAttachObjectARB(program, fragment);
  
  glLinkProgramARB(program);
  GLint hasLinked;
  glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &hasLinked);
  if(!hasLinked)
  {
    warn("Failed to link, error: %s", getLog(program).c_str());
    //warn("Failed to link.");

    glDeleteObjectARB(program);
    glDeleteObjectARB(vertex);
    glDeleteObjectARB(fragment);
    vertex = fragment = 0;
    return 0;
  }

  return program;
}

static void
freeOglBlockShaders(OglBlock& oglBlock)
{
  //TODO: check if program is currently bound, unbind it...(?)
  for(size_t i = 0; i < oglBlock.materials.size(); ++i)
  {
    if(oglBlock.materials[i].vertexShader != 0)
      glDeleteObjectARB(oglBlock.materials[i].vertexShader);
    if(oglBlock.materials[i].fragmentShader != 0)
      glDeleteObjectARB(oglBlock.materials[i].fragmentShader);
    if(oglBlock.materials[i].glslProgram != 0)
      glDeleteObjectARB(oglBlock.materials[i].glslProgram);

    oglBlock.materials[i].vertexShader = 0;
    oglBlock.materials[i].fragmentShader = 0;
    oglBlock.materials[i].glslProgram = 0;
  }
}


static void
compileShaderStrings(OglBlock& block)
{
  freeOglBlockShaders(block); //free old shaders

  for(size_t i = 0; i < block.materials.size(); ++i)
  {
    OglMaterial& oglMat = block.materials[i];
    oglMat.vertexShader = createShader(oglMat.vertexShaderString, GL_VERTEX_SHADER_ARB);
    oglMat.fragmentShader = createShader(oglMat.fragmentShaderString, GL_FRAGMENT_SHADER_ARB);
    oglMat.glslProgram = createProgram(oglMat.vertexShader, oglMat.fragmentShader);
  }
}

static void
generateShaderStrings(OglBlock& block, const Mat3& mat)
{
  block.materials.resize(mat.materials.size());
  for(unsigned int i = 0; i < mat.materials.size(); ++i)
  {
    OglMaterial& oglMat = block.materials[i];

    oglMat.vertexShader = 0;
    oglMat.fragmentShader = 0;
    oglMat.glslProgram = 0;

    oglMat.vertexShaderString = createVertexShaderString(i, mat);
    oglMat.fragmentShaderString = createFragmentShaderString(i, mat);
  }

  compileShaderStrings(block);
}

OglBlock* createOglBlock(const Mat3& mat, const std::string& baseName)
{
  warn("Mat supports still lacks texmatrix support, ind tex blocks, color chans and color swap support");

  OglBlock* block = new OglBlock;

  generateShaderStrings(*block, mat);

  return block;
}

void freeOglBlock(OglBlock*& oglBlock)
{
  if(oglBlock == NULL)
    return;

  freeOglBlockShaders(*oglBlock);

  delete oglBlock;
  oglBlock = NULL;
}

void setMaterial(int index, const OglBlock& block, const BModel& bmd)
{
  int m2 = bmd.mat3.indexToMatIndex[index];
  const Material& currMat = bmd.mat3.materials[m2];

  if(block.materials[m2].glslProgram == 0)
    return;

  //bind program (has to be bound _before_ uniforms can be set)
  glUseProgramObjectARB(block.materials[m2].glslProgram);

  //bind textures
  for(int i = 0; i < 8; ++i)
  {
    u16 stage = currMat.texStages[i];

    if(stage == 0xffff)
      continue;

    u16 t2 = bmd.mat3.texStageIndexToTextureIndex[stage];
    const ImageHeader& currTex = bmd.tex1.imageHeaders[t2];

    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, currTex.data->texId);
    setTexWrapMode(currTex.wrapS, currTex.wrapT);

    //set minification and magnification filters
    setFilters(currTex.magFilter, currTex.minFilter, currTex.data->mipmaps.size());


    //TODO: the following should be done during shader initialization
    std::ostringstream out;
    out << "texture" << i;
    GLint texLocation = glGetUniformLocationARB(block.materials[m2].glslProgram, out.str().c_str());
    if(texLocation != -1)
      glUniform1iARB(texLocation, i);
  }
  glActiveTexture(GL_TEXTURE0);
}
