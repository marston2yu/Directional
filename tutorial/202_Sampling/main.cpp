#include <directional/readOBJ.h>
#include <igl/euler_characteristic.h>
#include <igl/readDMAT.h>
#include <igl/writeDMAT.h>
#include <directional/principal_matching.h>
#include <directional/index_prescription.h>
#include <directional/power_field.h>
#include <directional/directional_viewer.h>
#include <directional/TriMesh.h>
#include <directional/FaceField.h>
#include <directional/power_to_raw.h>


directional::TriMesh mesh;
directional::FaceField rawField;
Eigen::VectorXi singVertices,singIndices;
Eigen::VectorXi b;
Eigen::MatrixXd bc;

int N=2;
double globalRotation=0.0;

typedef enum {TRIVIAL_ONE_SING, TRIVIAL_PRINCIPAL_MATCHING, IMPLICIT_FIELD} ViewingModes;
ViewingModes viewingMode=TRIVIAL_ONE_SING;

directional::DirectionalViewer viewer;


void update_directional_field()
{
  
  using namespace Eigen;
  using namespace std;
  VectorXd rotationAngles;
  Eigen::VectorXi presSingIndices;
  presSingIndices=VectorXi::Zero(rawField.dualCycles.rows());
  for (int i=0;i<singVertices.size();i++)
    presSingIndices(singVertices[i])=singIndices[i];
  
  double IPError;
  Eigen::VectorXi currIndices;
  directional::index_prescription(presSingIndices,N,globalRotation, rawField, rotationAngles, IPError);

  if (viewingMode==TRIVIAL_PRINCIPAL_MATCHING)
    directional::principal_matching(rawField);
  
  if (viewingMode==IMPLICIT_FIELD){
    bc.conservativeResize(b.rows(),3);
    for (int i=0;i<b.size();i++)
      bc.row(i)<<rawField.extField.block(b(i),0,1,3).normalized();
    
    Eigen::VectorXd effort;
    directional::FaceField powerField;
    powerField.init_field(*(rawField.mesh), POWER_FIELD, N);
    directional::power_field(powerField, b, bc, Eigen::VectorXd::Constant(b.size(),-1), N);
    directional::power_to_raw(powerField, N, rawField,true);
    directional::principal_matching(rawField);
  }
  
  viewer.set_field(rawField);
  
  if (viewingMode==TRIVIAL_ONE_SING)
    viewer.set_singularities(singVertices, singIndices);
   
  if ((viewingMode==TRIVIAL_PRINCIPAL_MATCHING)||(viewingMode==IMPLICIT_FIELD))
    viewer.set_singularities(rawField.singCycles, rawField.singIndices);
  
}



bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifiers)
{
  using namespace std;
  switch(key)
  {
    case '1': viewingMode=TRIVIAL_ONE_SING; cout<<"Showing prescribed singularity "<<std::endl;
      break;
    case '2': viewingMode=TRIVIAL_PRINCIPAL_MATCHING; cout<<"Principal-matching singularities "<<std::endl;
      break;
    case '3': viewingMode=IMPLICIT_FIELD; cout<<"Field interpolated from constraints with principal singularities "<<std::endl;
      break;
      
    case '4':{
      singIndices[0]--;
      singIndices[1]++;
      cout<<"Prescribed singularity index: "<<singIndices[0]<<"/"<<N<<std::endl;
      break;
    }
    case '5':{
      singIndices[0]++;
      singIndices[1]--;
      cout<<"Prescribed singularity index: "<<singIndices[0]<<"/"<<N<<std::endl;
      break;
    }
      
    case '6':{
      globalRotation+=igl::PI/16;
      std::cout<<"globalRotation: " <<globalRotation<<std::endl;
      break;
    }
    default: break;
  }
  update_directional_field();
  return true;
}

int main()
{
  std::cout <<
  "1-3      Toggle between singularity modes" << std::endl <<
  "4-5      Decrease/increase prescribed singularity index" << std::endl <<
  "6        Change global rotation" << std::endl;
  using namespace Eigen;
  using namespace std;
  directional::readOBJ(TUTORIAL_SHARED_PATH "/spherers.obj",mesh);

  igl::readDMAT(TUTORIAL_SHARED_PATH "/spheres_constFaces.dmat",b);
  
  singVertices.resize(2);
  singIndices.resize(2);
  singVertices(0)=35;
  singVertices(1)=36;
  singIndices(0)=N;
  singIndices(1)=N;
  
  rawField.init_field(mesh, RAW_FIELD, N);
  
  //viewing mesh
  viewer.set_mesh(mesh);
  viewer.set_field(rawField);
  viewer.set_selected_faces(b);
  update_directional_field();
  
  viewer.callback_key_down = &key_down;
  viewer.launch();
}
