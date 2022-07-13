// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_READ_RAW_FIELD_H
#define DIRECTIONAL_READ_RAW_FIELD_H
#include <cmath>
#include <Eigen/Core>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <directional/IntrinsicFaceTangentBundle.h>
#include <directional/definitions.h>

namespace directional
{
  
  // Reads a raw field from a file
  // Inputs:
  //   fileName: The to be loaded file.
  //    mesh: the underlying mesh to the read field
  // Outputs:
  //   N: The degree of the field
  //   rawField: the read RAW_FIELD field.
  // Return:
  //   Whether or not the file was read successfully
  bool IGL_INLINE read_raw_field(const std::string &fileName,
                                 const directional::IntrinsicFaceTangentBundle& ftb,
                                 int& N,
                                 directional::FaceField& field)
  {
    try
    {
      std::ifstream f(fileName);
      if (!f.is_open()) {
          return false;
      }
      int numF;
      f>>N;
      f>>numF;
      Eigen::MatrixXd extField;
      extField.conservativeResize(numF, 3*N);
      
      //Can we do better than element-wise reading?
      for (int i=0;i<extField.rows();i++)
        for (int j=0;j<extField.cols();j++)
          f>>extField(i,j);
      
      f.close();
      field.init(ftb, RAW_FIELD, N);
      field.set_extrinsic_field(extField);
      return f.good();
    }
    catch (std::exception e)
    {
      return false;
    }
  }
}

#endif
